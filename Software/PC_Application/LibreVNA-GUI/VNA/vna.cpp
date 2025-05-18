#include "vna.h"

#include "unit.h"
#include "CustomWidgets/toggleswitch.h"
#include "Traces/tracemodel.h"
#include "tracewidgetvna.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracexyplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include "CustomWidgets/siunitedit.h"
#include "Traces/Marker/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "Tools/mixedmodeconversion.h"
#include "ui_main.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"
#include "CustomWidgets/informationbox.h"
#include "Deembedding/manualdeembeddingdialog.h"
#include "Calibration/manualcalibrationdialog.h"
#include "Calibration/LibreCAL/librecaldialog.h"
#include "Util/util.h"
#include "Tools/parameters.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <math.h>
#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QActionGroup>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include <algorithm>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <iostream>
#include <fstream>
#include <QDateTime>
#include <QDockWidget>
#include <queue>
#include <QApplication>
#include <QActionGroup>
#include <QErrorMessage>
#include <QDebug>
#include <QStyle>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QDateTime>

VNA::VNA(AppWindow *window, QString name)
    : Mode(window, name, "VNA"),
      deembedding(traceModel),
      deembedding_active(false),
      tiles(new TileWidget(traceModel)),
    central(new QScrollArea)
{
    central->setWidget(tiles);
    central->setWidgetResizable(true);
    averages = 1;
    singleSweep = false;
    calMeasuring = false;
    calWaitFirst = false;
    calDialog = nullptr;

    changingSettings = false;
    settings.sweepType = SweepType::Frequency;
    settings.zerospan = false;

    traceModel.setSource(TraceModel::DataSource::VNA);

    configurationTimer.setSingleShot(true);
    connect(&configurationTimer, &QTimer::timeout, this, [=](){
        ConfigureDevice(configurationTimerResetTraces);
    });

    // Create default traces
    createDefaultTracesAndGraphs(2);

    connect(&traceModel, &TraceModel::requiredExcitation, this, &VNA::ExcitationRequired);

    // Create menu entries and connections
    auto calMenu = new QMenu("Calibration", window);
    window->menuBar()->insertMenu(window->getUi()->menuWindow->menuAction(), calMenu);
    actions.insert(calMenu->menuAction());
    auto calLoad = calMenu->addAction("Load");
    saveCal = calMenu->addAction("Save");
    calMenu->addSeparator();

    connect(calLoad, &QAction::triggered, [=](){
        LoadCalibration();
        if(cal.getCaltype().type != Calibration::Type::None) {
            if(InformationBox::AskQuestion("Adjust span?", "Do you want to adjust the span to match the loaded calibration file?", false)) {
                SpanMatchCal();
            }
        }
    });

    connect(saveCal, &QAction::triggered, [=](){
        SaveCalibration();
    });

    connect(&cal, &Calibration::startMeasurements, this, &VNA::StartCalibrationMeasurements);

    auto calData = calMenu->addAction("Calibration Measurements");
    connect(calData, &QAction::triggered, [=](){
        cal.edit();
    });

    auto calEditKit = calMenu->addAction("Edit Calibration Kit");
    connect(calEditKit, &QAction::triggered, [=](){
        cal.getKit().edit([=](){
            if(cal.getCaltype().type != Calibration::Type::None) {
                cal.compute(cal.getCaltype());
            }
        });
    });

    auto calElectronic = calMenu->addAction("Electronic Calibration");
    connect(calElectronic, &QAction::triggered, [=](){
        auto d = new LibreCALDialog(&cal);
        d->show();
    });

    calMenu->addSeparator();

    auto calImportTerms = calMenu->addAction("Import error terms as traces");
    calImportTerms->setEnabled(false);
    connect(calImportTerms, &QAction::triggered, [=](){
        auto import = new TraceImportDialog(traceModel, cal.getErrorTermTraces());
        if(AppWindow::showGUI()) {
            import->show();
        }
    });
    auto calImportMeas = calMenu->addAction("Import measurements as traces");
    calImportMeas->setEnabled(false);
    connect(calImportMeas, &QAction::triggered, [=](){
        auto import = new TraceImportDialog(traceModel, cal.getMeasurementTraces());
        if(AppWindow::showGUI()) {
            import->show();
        }
    });

    calMenu->addSeparator();
    auto calApplyToTraces = calMenu->addAction("Apply to traces...");
    calApplyToTraces->setEnabled(false);
    connect(calApplyToTraces, &QAction::triggered, [=]() {
        auto manualCalibration = new ManualCalibrationDialog(traceModel, &cal);
        if(AppWindow::showGUI()) {
            manualCalibration->show();
        }
    });

    cal.getKit().setIdealDefault();

    calDialog = new QProgressDialog();
    calDialog->setCancelButtonText("Abort");
    calDialog->setWindowTitle("Taking calibration measurement...");
    calDialog->setValue(0);
    calDialog->setWindowModality(Qt::ApplicationModal);
    calDialog->reset();
    calDialog->setMinimumDuration(0);

    // A modal QProgressDialog calls processEvents() in setValue(). Needs to use a queued connection to update the progress
    // value from within the NewDatapoint slot to prevent possible re-entrancy.
    connect(this, &VNA::calibrationMeasurementPercentage, calDialog, [=](int percent) {
        if(calMeasuring || percent == 100) {
            calDialog->setValue(percent);
        }
    }, Qt::QueuedConnection);

    connect(calDialog, &QProgressDialog::canceled, this, [=]() {
        // the user aborted the calibration measurement
        calMeasuring = false;
        cal.clearMeasurements(calMeasurements);
        cal.measurementsAbort();
        // delete calDialog;
    });

//    portExtension.setCalkit(&cal.getCalibrationKit());

    // De-embedding menu
    auto menuDeembed = new QMenu("De-embedding", window);
    window->menuBar()->insertMenu(window->getUi()->menuWindow->menuAction(), menuDeembed);
    actions.insert(menuDeembed->menuAction());
    auto confDeembed = new QAction("Setup...", menuDeembed);
    confDeembed->setMenuRole(QAction::NoRole);
    menuDeembed->addAction(confDeembed);
    connect(confDeembed, &QAction::triggered, &deembedding, &Deembedding::configure);

    enableDeembeddingAction = menuDeembed->addAction("De-embed VNA samples");
    enableDeembeddingAction->setCheckable(true);
    enableDeembeddingAction->setEnabled(false);
    connect(enableDeembeddingAction, &QAction::toggled, this, &VNA::EnableDeembedding);

    auto manualDeembed = menuDeembed->addAction("De-embed traces...");
    manualDeembed->setEnabled(false);
    connect(manualDeembed, &QAction::triggered, [=]() {
        auto manualDeembedding = new ManualDeembeddingDialog(traceModel, &deembedding);
        if(AppWindow::showGUI()) {
            manualDeembedding->show();
        }
    });

    connect(&deembedding, &Deembedding::optionAdded, [=](){
        EnableDeembedding(true);
        enableDeembeddingAction->setEnabled(true);
        manualDeembed->setEnabled(true);
    });
    connect(&deembedding, &Deembedding::allOptionsCleared, [=](){
        EnableDeembedding(false);
        enableDeembeddingAction->setEnabled(false);
        manualDeembed->setEnabled(false);
    });
    connect(&deembedding, &Deembedding::triggerMeasurement, [=]() {
        // de-embedding measurement requested
        wasRunningBeforeDeembeddingMeasurement = running;
        Run();
    });
    connect(&deembedding, &Deembedding::finishedMeasurement, [=](){
        if(wasRunningBeforeDeembeddingMeasurement) {
            Run();
        } else {
            Stop();
        }
    });

    // Tools menu
    auto toolsMenu = new QMenu("Tools", window);
    window->menuBar()->insertMenu(window->getUi()->menuWindow->menuAction(), toolsMenu);
    actions.insert(toolsMenu->menuAction());
    auto impedanceMatching = toolsMenu->addAction("Impedance Matching");
    connect(impedanceMatching, &QAction::triggered, this, &VNA::StartImpedanceMatching);
    auto mixedMode = toolsMenu->addAction("Mixed Mode Conversion");
    connect(mixedMode, &QAction::triggered, this, &VNA::StartMixedModeConversion);

    defaultCalMenu = new QMenu("Default Calibration", window);
    assignDefaultCal = defaultCalMenu->addAction("Assign...");
    removeDefaultCal = defaultCalMenu->addAction("Remove");
    removeDefaultCal->setEnabled(false);
    defaultCalMenu->setEnabled(false);

    actions.insert(window->getUi()->menuDevice->addSeparator());
    window->getUi()->menuDevice->addMenu(defaultCalMenu);
    actions.insert(defaultCalMenu->menuAction());

    connect(assignDefaultCal, &QAction::triggered, [=](){
       if(window->getDevice()) {
           auto key = "DefaultCalibration"+window->getDevice()->getSerial();
           QSettings settings;
           auto filename = QFileDialog::getOpenFileName(nullptr, "Load calibration data", settings.value(key).toString(), "Calibration files (*.cal)", nullptr, Preferences::QFileDialogOptions());
           if(!filename.isEmpty()) {
               settings.setValue(key, filename);
               removeDefaultCal->setEnabled(true);
               LoadCalibration(filename);
           }
       }
    });
    connect(removeDefaultCal, &QAction::triggered, [=](){
        QSettings settings;
        settings.remove("DefaultCalibration"+window->getDevice()->getSerial());
        removeDefaultCal->setEnabled(false);
    });


    // Sweep toolbar
    auto tb_sweep = new QToolBar("Sweep");

    std::vector<QAction*> frequencySweepActions;
    std::vector<QAction*> powerSweepActions;

    auto bRun = new QPushButton("Run/Stop");
    bRun->setToolTip("Pause/continue sweep");
    bRun->setCheckable(true);
    running = true;
    connect(bRun, &QPushButton::toggled, [=](){
        if(bRun->isChecked()) {
            Run();
        } else {
            Stop();
        }
    });
    connect(this, &VNA::sweepStopped, [=](){
        bRun->blockSignals(true);
        bRun->setChecked(false);
        bRun->setIcon(bRun->style()->standardIcon(QStyle::SP_MediaPause));
        bRun->blockSignals(false);
    });
    connect(this, &VNA::sweepStarted, [=](){
        bRun->blockSignals(true);
        bRun->setChecked(true);
        bRun->setIcon(bRun->style()->standardIcon(QStyle::SP_MediaPlay));
        bRun->blockSignals(false);
    });
    tb_sweep->addWidget(bRun);

    auto bSingle = new QPushButton("Single");
    bSingle->setToolTip("Single sweep");
    bSingle->setCheckable(true);
    connect(bSingle, &QPushButton::toggled, this, &VNA::SetSingleSweep);
    connect(this, &VNA::singleSweepChanged, bSingle, &QPushButton::setChecked);
    tb_sweep->addWidget(bSingle);

    tb_sweep->addWidget(new QLabel("Sweep type:"));
    cbSweepType = new QComboBox();
    cbSweepType->addItem("Frequency");
    cbSweepType->addItem("Power");
    tb_sweep->addWidget(cbSweepType);

    auto eStart = new SIUnitEdit("Hz", " kMG", 6);
    // calculate width required with expected string length
    auto width = QFontMetrics(eStart->font()).horizontalAdvance("3.00000GHz") + 15;
    eStart->setFixedWidth(width);
    eStart->setToolTip("Start frequency");
    connect(eStart, &SIUnitEdit::valueChanged, this, &VNA::SetStartFreq);
    connect(this, &VNA::startFreqChanged, eStart, &SIUnitEdit::setValueQuiet);
    frequencySweepActions.push_back(tb_sweep->addWidget(new QLabel("Start:")));
    frequencySweepActions.push_back(tb_sweep->addWidget(eStart));

    auto eCenter = new SIUnitEdit("Hz", " kMG", 6);
    eCenter->setFixedWidth(width);
    eCenter->setToolTip("Center frequency");
    connect(eCenter, &SIUnitEdit::valueChanged, this, &VNA::SetCenterFreq);
    connect(this, &VNA::centerFreqChanged, eCenter, &SIUnitEdit::setValueQuiet);
    frequencySweepActions.push_back(tb_sweep->addWidget(new QLabel("Center:")));
    frequencySweepActions.push_back(tb_sweep->addWidget(eCenter));

    auto eStop = new SIUnitEdit("Hz", " kMG", 6);
    eStop->setFixedWidth(width);
    eStop->setToolTip("Stop frequency");
    connect(eStop, &SIUnitEdit::valueChanged, this, &VNA::SetStopFreq);
    connect(this, &VNA::stopFreqChanged, eStop, &SIUnitEdit::setValueQuiet);
    frequencySweepActions.push_back(tb_sweep->addWidget(new QLabel("Stop:")));
    frequencySweepActions.push_back(tb_sweep->addWidget(eStop));

    auto eSpan = new SIUnitEdit("Hz", " kMG", 6);
    eSpan->setFixedWidth(width);
    eSpan->setToolTip("Span");
    connect(eSpan, &SIUnitEdit::valueChanged, this, &VNA::SetSpan);
    connect(this, &VNA::spanChanged, eSpan, &SIUnitEdit::setValueQuiet);
    frequencySweepActions.push_back(tb_sweep->addWidget(new QLabel("Span:")));
    frequencySweepActions.push_back(tb_sweep->addWidget(eSpan));

    auto bFull = new QPushButton(QIcon::fromTheme("zoom-fit-best", QIcon(":/icons/zoom-fit.png")), "");
    bFull->setToolTip("Full span");
    connect(bFull, &QPushButton::clicked, this, &VNA::SetFullSpan);
    frequencySweepActions.push_back(tb_sweep->addWidget(bFull));

    auto bZoomIn = new QPushButton(QIcon::fromTheme("zoom-in", QIcon(":/icons/zoom-in.png")), "");
    bZoomIn->setToolTip("Zoom in");
    connect(bZoomIn, &QPushButton::clicked, this, &VNA::SpanZoomIn);
    frequencySweepActions.push_back(tb_sweep->addWidget(bZoomIn));

    auto bZoomOut = new QPushButton(QIcon::fromTheme("zoom-out", QIcon(":/icons/zoom-out.png")), "");
    bZoomOut->setToolTip("Zoom out");
    connect(bZoomOut, &QPushButton::clicked, this, &VNA::SpanZoomOut);
    frequencySweepActions.push_back(tb_sweep->addWidget(bZoomOut));

    bZero = new QPushButton("0");
    bZero->setToolTip("Zero span");
    bZero->setMaximumWidth(28);
    bZero->setMaximumHeight(24);
    connect(bZero, &QPushButton::clicked, this, &VNA::SetZeroSpan);
    frequencySweepActions.push_back(tb_sweep->addWidget(bZero));

    bMatchCal = new QPushButton("Cal");
    bMatchCal->setToolTip("Match span of calibration");
    bMatchCal->setMaximumWidth(28);
    bMatchCal->setMaximumHeight(24);
    bMatchCal->setEnabled(false);
    connect(bMatchCal, &QPushButton::clicked, this, &VNA::SpanMatchCal);
    frequencySweepActions.push_back(tb_sweep->addWidget(bMatchCal));

    cbLogSweep = new  QCheckBox("Log");
    cbLogSweep->setToolTip("Logarithmic sweep");
    connect(cbLogSweep, &QCheckBox::toggled, this, &VNA::SetLogSweep);
    connect(this, &VNA::logSweepChanged, cbLogSweep, &QCheckBox::setChecked);
    frequencySweepActions.push_back(tb_sweep->addWidget(cbLogSweep));

    // power sweep widgets
    auto sbPowerLow = new QDoubleSpinBox();
    width = QFontMetrics(sbPowerLow->font()).horizontalAdvance("-30.00dBm") + 20;
    sbPowerLow->setFixedWidth(width);
    sbPowerLow->setRange(-100.0, 100.0);
    sbPowerLow->setSingleStep(0.25);
    sbPowerLow->setSuffix("dbm");
    sbPowerLow->setToolTip("Stimulus level");
    sbPowerLow->setKeyboardTracking(false);
    connect(sbPowerLow, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &VNA::SetStartPower);
    connect(this, &VNA::startPowerChanged, sbPowerLow, &QDoubleSpinBox::setValue);
    powerSweepActions.push_back(tb_sweep->addWidget(new QLabel("From:")));
    powerSweepActions.push_back(tb_sweep->addWidget(sbPowerLow));

    auto sbPowerHigh = new QDoubleSpinBox();
    width = QFontMetrics(sbPowerHigh->font()).horizontalAdvance("-30.00dBm") + 20;
    sbPowerHigh->setFixedWidth(width);
    sbPowerHigh->setRange(-100.0, 100.0);
    sbPowerHigh->setSingleStep(0.25);
    sbPowerHigh->setSuffix("dbm");
    sbPowerHigh->setToolTip("Stimulus level");
    sbPowerHigh->setKeyboardTracking(false);
    connect(sbPowerHigh, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &VNA::SetStopPower);
    connect(this, &VNA::stopPowerChanged, sbPowerHigh, &QDoubleSpinBox::setValue);
    powerSweepActions.push_back(tb_sweep->addWidget(new QLabel("To:")));
    powerSweepActions.push_back(tb_sweep->addWidget(sbPowerHigh));

    auto ePowerFreq = new SIUnitEdit("Hz", " kMG", 6);
    width = QFontMetrics(ePowerFreq->font()).horizontalAdvance("3.00000GHz") + 15;
    ePowerFreq->setFixedWidth(width);
    ePowerFreq->setToolTip("Start frequency");
    connect(ePowerFreq, &SIUnitEdit::valueChanged, this, &VNA::SetPowerSweepFrequency);
    connect(this, &VNA::powerSweepFrequencyChanged, ePowerFreq, &SIUnitEdit::setValueQuiet);
    powerSweepActions.push_back(tb_sweep->addWidget(new QLabel("at:")));
    powerSweepActions.push_back(tb_sweep->addWidget(ePowerFreq));

    window->addToolBar(tb_sweep);
    toolbars.insert(tb_sweep);

    // Acquisition toolbar
    auto tb_acq = new QToolBar("Acquisition");
    auto dbm = new QDoubleSpinBox();
    width = QFontMetrics(dbm->font()).horizontalAdvance("-30.00dBm") + 20;
    dbm->setFixedWidth(width);
    dbm->setRange(-100.0, 100.0);
    dbm->setSingleStep(0.25);
    dbm->setSuffix("dbm");
    dbm->setToolTip("Stimulus level");
    dbm->setKeyboardTracking(false);
    connect(dbm, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &VNA::SetSourceLevel);
    connect(this, &VNA::sourceLevelChanged, dbm, &QDoubleSpinBox::setValue);
    frequencySweepActions.push_back(tb_acq->addWidget(new QLabel("Level:")));
    frequencySweepActions.push_back(tb_acq->addWidget(dbm));

    auto points = new QSpinBox();
    points->setFixedWidth(65);
    points->setRange(1, UINT16_MAX);
    points->setSingleStep(100);
    points->setToolTip("Points/sweep");
    points->setKeyboardTracking(false);
    connect(points, qOverload<int>(&QSpinBox::valueChanged), this, &VNA::SetPoints);
    connect(this, &VNA::pointsChanged, [=](int p) {
        points->blockSignals(true);
        points->setValue(p);
        points->blockSignals(false);
    });
    tb_acq->addWidget(new QLabel("Points:"));
    tb_acq->addWidget(points);

    auto eBandwidth = new SIUnitEdit("Hz", " kM", 3);
    eBandwidth->setFixedWidth(70);
    eBandwidth->setToolTip("IF bandwidth");
    connect(eBandwidth, &SIUnitEdit::valueChanged, this, &VNA::SetIFBandwidth);
    connect(this, &VNA::IFBandwidthChanged, eBandwidth, &SIUnitEdit::setValueQuiet);
    tb_acq->addWidget(new QLabel("IF BW:"));
    tb_acq->addWidget(eBandwidth);

    tb_acq->addWidget(new QLabel("Dwell time:"));
    acquisitionDwellTime = new SIUnitEdit("s", "um ", 3);
    width = QFontMetrics(dbm->font()).horizontalAdvance("100ms") + 20;
    acquisitionDwellTime->setFixedWidth(width);
    connect(acquisitionDwellTime, &SIUnitEdit::valueChanged, this, &VNA::SetDwellTime);
    connect(this, &VNA::dwellTimeChanged, acquisitionDwellTime, &SIUnitEdit::setValueQuiet);
    tb_acq->addWidget(acquisitionDwellTime);

    tb_acq->addWidget(new QLabel("Averaging:"));
    lAverages = new QLabel("0/");
    tb_acq->addWidget(lAverages);
    auto sbAverages = new QSpinBox;
    sbAverages->setRange(1, 99);
    sbAverages->setFixedWidth(40);
    connect(sbAverages, qOverload<int>(&QSpinBox::valueChanged), this, &VNA::SetAveraging);
    connect(this, &VNA::averagingChanged, sbAverages, &QSpinBox::setValue);
    tb_acq->addWidget(sbAverages);
    auto bResetAvg = new QPushButton("Reset");
    connect(bResetAvg, &QPushButton::clicked, this, [=](){
        average.reset(settings.npoints);
        UpdateAverageCount();
    });
    tb_acq->addWidget(bResetAvg);

    window->addToolBar(tb_acq);
    toolbars.insert(tb_acq);

    // Calibration toolbar (and populate calibration menu)
    auto tb_cal = new QToolBar("Calibration");
    calLabel = new QLabel("Calibration:");
    UpdateCalWidget();
    tb_cal->addWidget(calLabel);
    auto cbEnableCal = new QCheckBox;
    tb_cal->addWidget(cbEnableCal);
    auto cbType = new QComboBox();

    auto updateCalComboBox = [=](){
        auto cals = cal.getAvailableCalibrations();
        cbType->blockSignals(true);
        cbType->clear();
        for(auto c : cals) {
            if(c.type == Calibration::Type::None) {
                continue;
            }
            cbType->addItem(c.getShortString());
        }
        cbType->setCurrentText(cal.getCaltype().getShortString());
        cbType->blockSignals(false);
    };

    connect(this, &VNA::deviceInitialized, updateCalComboBox);

    updateCalComboBox();

    auto calToolbarLambda = [=]() {
        if(cbEnableCal->isChecked()) {
            // Get requested calibration type from combobox
            ApplyCalibration(Calibration::CalType::fromShortString(cbType->currentText()));
        } else {
            DisableCalibration();
        }
    };

    // Calibration connections
    connect(&cal, &Calibration::activated, this, &VNA::UpdateStatusbar);
    connect(&cal, &Calibration::deactivated, this, &VNA::UpdateStatusbar);
    connect(cbEnableCal, &QCheckBox::stateChanged, calToolbarLambda);
    connect(cbType, qOverload<int>(&QComboBox::currentIndexChanged), calToolbarLambda);
    connect(&cal, &Calibration::deactivated, [=](){
        cbType->blockSignals(true);
        cbEnableCal->blockSignals(true);
        cbEnableCal->setCheckState(Qt::CheckState::Unchecked);
        // visually indicate loss of calibration
        // cal. file unknown at this moment
        UpdateCalWidget();
        cbType->blockSignals(false);
        cbEnableCal->blockSignals(false);
        calImportTerms->setEnabled(false);
        calImportMeas->setEnabled(false);
        calApplyToTraces->setEnabled(false);
        bMatchCal->setEnabled(false);
//        saveCal->setEnabled(false);
    });
    connect(&cal, &Calibration::activated, [=](Calibration::CalType applied){
        cbType->blockSignals(true);
        cbEnableCal->blockSignals(true);
        cbType->setCurrentText(applied.getShortString());
        cbEnableCal->setCheckState(Qt::CheckState::Checked);
        // restore default look of widget
        // on hover, show name of active cal. file
        UpdateCalWidget();
        cbType->blockSignals(false);
        cbEnableCal->blockSignals(false);
        calImportTerms->setEnabled(true);
        calImportMeas->setEnabled(true);
        calApplyToTraces->setEnabled(true);
        bMatchCal->setEnabled(true);
        saveCal->setEnabled(true);
        if(window->getDevice() && !cal.validForDevice(window->getDevice()->getSerial())) {
            InformationBox::ShowMessage("Invalid calibration", "The selected calibration was created for a different device. You can still load it but the resulting "
                                                               "data likely isn't useful.");
        }
    });

    tb_cal->addWidget(cbType);

    window->addToolBar(tb_cal);

    auto configureToolbarForFrequencySweep = [=](){
        for(auto a : frequencySweepActions) {
            a->setVisible(true);
        }
        for(auto a : powerSweepActions) {
            a->setVisible(false);
        }
        // enable calibration menu entries
        calData->setEnabled(true);
    };
    auto configureToolbarForPowerSweep = [=](){
        for(auto a : frequencySweepActions) {
            a->setVisible(false);
        }
        for(auto a : powerSweepActions) {
            a->setVisible(true);
        }
        // disable calibration menu entries
        calData->setEnabled(false);
    };

    connect(cbSweepType, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
        SetSweepType((SweepType) index);
    });
    connect(this, &VNA::sweepTypeChanged, [=](SweepType sw) {
        if(sw == SweepType::Frequency) {
            configureToolbarForFrequencySweep();
        } else if(sw == SweepType::Power) {
            configureToolbarForPowerSweep();
        }
        cbSweepType->setCurrentIndex((int) sw);
    });
    configureToolbarForFrequencySweep();
    // initial setup is frequency sweep
    configureToolbarForFrequencySweep();
    SetSweepType(SweepType::Frequency);

    toolbars.insert(tb_cal);

    markerModel = new MarkerModel(traceModel, this);

    auto tracesDock = new QDockWidget("Traces");
    traceWidget = new TraceWidgetVNA(traceModel, &cal, &deembedding);
    tracesDock->setWidget(traceWidget);
    window->addDockWidget(Qt::LeftDockWidgetArea, tracesDock);
    docks.insert(tracesDock);

    auto importAction = new QAction("Touchstone/CSV");
    connect(importAction, &QAction::triggered, traceWidget, &TraceWidgetVNA::importDialog);
    importActions.push_back(importAction);

    auto exportTouchstone = new QAction("Touchstone");
    connect(exportTouchstone, &QAction::triggered, traceWidget, &TraceWidgetVNA::exportTouchstone);
    exportActions.push_back(exportTouchstone);

    auto exportCSV = new QAction("CSV");
    connect(exportCSV, &QAction::triggered, traceWidget, &TraceWidgetVNA::exportCSV);
    exportActions.push_back(exportCSV);


    auto markerWidget = new MarkerWidget(*markerModel);

    auto markerDock = new QDockWidget("Marker");
    markerDock->setWidget(markerWidget);
    window->addDockWidget(Qt::BottomDockWidgetArea, markerDock);
    docks.insert(markerDock);

    SetupSCPI();

    // Set initial sweep settings
    auto& pref = Preferences::getInstance();

    if(pref.Acquisition.useMedianAveraging) {
        average.setMode(Averaging::Mode::Median);
    } else {
        average.setMode(Averaging::Mode::Mean);
    }

    if(pref.Startup.RememberSweepSettings) {
        LoadSweepSettings();
    } else {
        settings.Freq.start = pref.Startup.DefaultSweep.f_start;
        settings.Freq.stop = pref.Startup.DefaultSweep.f_stop;
        SetLogSweep(pref.Startup.DefaultSweep.logSweep);
        SetSourceLevel(pref.Startup.DefaultSweep.f_excitation);
        ConstrainAndUpdateFrequencies();
        SetStartPower(pref.Startup.DefaultSweep.dbm_start);
        SetStopPower(pref.Startup.DefaultSweep.dbm_stop);
        SetPowerSweepFrequency(pref.Startup.DefaultSweep.dbm_freq);
        SetIFBandwidth(pref.Startup.DefaultSweep.bandwidth);
        SetAveraging(pref.Startup.DefaultSweep.averaging);
        SetDwellTime(pref.Startup.DefaultSweep.dwellTime);
        SetPoints(pref.Startup.DefaultSweep.points);
        if(pref.Startup.DefaultSweep.type == "Power Sweep") {
            SetSweepType(SweepType::Power);
        } else {
            SetSweepType(SweepType::Frequency);
        }
    }

    // Set ObjectName for toolbars and docks
    for(auto d : findChildren<QDockWidget*>()) {
        d->setObjectName(d->windowTitle());
    }
    for(auto t : findChildren<QToolBar*>()) {
        t->setObjectName(t->windowTitle());
    }

    finalize(central);
}

Calibration::InterpolationType VNA::getCalInterpolation()
{
    double f_min = 0, f_max = 0;
    switch(settings.sweepType) {
    case SweepType::Last:
        // should never get here, use frequency values just in case
    case SweepType::Frequency:
        f_min = settings.Freq.start;
        f_max = settings.Freq.stop;
        break;
    case SweepType::Power:
        f_min = settings.Power.frequency;
        f_max = settings.Power.frequency;
        break;
    }
    return cal.getInterpolation(f_min, f_max, settings.npoints);
}

QString VNA::getCalStyle()
{
    auto interpol = getCalInterpolation();
    QString style = "";
    switch (interpol)
    {
    case Calibration::InterpolationType::Unchanged:
    case Calibration::InterpolationType::Exact:
    case Calibration::InterpolationType::Interpolate:
        style = "";
        break;

    case Calibration::InterpolationType::Extrapolate:
        style = "background-color: yellow";
        break;
    case Calibration::InterpolationType::NoCalibration:
        style = "background-color: red";
        break;
    }
    return style;
}

QString VNA::getCalToolTip()
{
    auto interpol = getCalInterpolation();
    QString txt = "";
    switch (interpol)
    {
    case Calibration::InterpolationType::Unchanged:
    case Calibration::InterpolationType::Exact:
    case Calibration::InterpolationType::Interpolate:
    case Calibration::InterpolationType::Extrapolate:
    {
        QString lo = Unit::ToString(cal.getMinFreq(), "", " kMG", 5);
        QString hi = Unit::ToString(cal.getMaxFreq(), "", " kMG", 5);
        if (settings.Freq.start < cal.getMinFreq() ) { lo = "<font color=\"red\">" + lo + "</font>";}
        if (settings.Freq.stop > cal.getMaxFreq() ) { hi = "<font color=\"red\">" + hi + "</font>";}
        txt =
                "limits: " + lo + " - " + hi
                + "<br>"
                + "points: " + QString::number(cal.getNumPoints())
                + "<br>"
                "file: " + cal.getCurrentCalibrationFile();
        break;
    }
    case Calibration::InterpolationType::NoCalibration:
        txt = "none";
        break;
    }
    return txt;
}

void VNA::deactivate()
{
    StoreSweepSettings();
    configurationTimer.stop();
    Mode::deactivate();
    setOperationPending(false);
}

static void SetComboBoxItemEnabled(QComboBox * comboBox, int index, bool enabled)
{
    auto * model = qobject_cast<QStandardItemModel*>(comboBox->model());
    assert(model);
    if(!model) return;

    auto * item = model->item(index);
    assert(item);
    if(!item) return;
    item->setEnabled(enabled);
}

void VNA::initializeDevice()
{
    if(!window->getDevice()->supports(DeviceDriver::Feature::VNA)) {
        InformationBox::ShowError("Unsupported", "The connected device does not support VNA mode");
        return;
    }
    cbLogSweep->setEnabled(window->getDevice()->supports(DeviceDriver::Feature::VNALogSweep));
    if(!window->getDevice()->supports(DeviceDriver::Feature::VNALogSweep)) {
        SetLogSweep(false);
    }

    bZero->setEnabled(window->getDevice()->supports(DeviceDriver::Feature::VNAZeroSpan));
    SetComboBoxItemEnabled(cbSweepType, 0, window->getDevice()->supports(DeviceDriver::Feature::VNAFrequencySweep));
    SetComboBoxItemEnabled(cbSweepType, 1, window->getDevice()->supports(DeviceDriver::Feature::VNAPowerSweep));

    defaultCalMenu->setEnabled(true);
    connect(window->getDevice(), &DeviceDriver::VNAmeasurementReceived, this, &VNA::NewDatapoint, Qt::UniqueConnection);
    // Check if default calibration exists and attempt to load it
    QSettings s;
    auto key = "DefaultCalibration"+window->getDevice()->getSerial();
    if (s.contains(key)) {
        auto filename = s.value(key).toString();
        qDebug() << "Attempting to load default calibration file " << filename;
        if(QFile::exists(filename)) {
            if(cal.fromFile(filename)) {
                qDebug() << "Calibration successful from " << filename;
            } else {
                qDebug() << "Calibration not successfull from: " << filename;
            }
        } else {
            qDebug() << "Calibration file not found: " << filename;
        }
        removeDefaultCal->setEnabled(true);
    } else {
        qDebug() << "No default calibration file set for this device";
        removeDefaultCal->setEnabled(false);
    }
    // Configure initial state of device
    ConstrainAndUpdateFrequencies();
    SettingsChanged(true);
    emit deviceInitialized();
    if(window->getDevice() && !cal.validForDevice(window->getDevice()->getSerial())) {
        InformationBox::ShowMessage("Invalid calibration", "The current calibration was created for a different device. You can still use it but the resulting "
                                    "data likely isn't useful.");
    }
}

void VNA::deviceDisconnected()
{
    defaultCalMenu->setEnabled(false);
    emit sweepStopped();
}

void VNA::shutdown()
{
    if(cal.hasUnsavedChanges() && cal.getCaltype().type != Calibration::Type::None) {
        auto save = InformationBox::AskQuestion("Save calibration?", "The calibration contains data that has not been saved yet. Do you want to save it before exiting?", false);
        if(save) {
            SaveCalibration();
        }
    }
}

void VNA::resetSettings()
{
    settings.Freq.start = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq;
    settings.Freq.stop = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq;
    SetLogSweep(false);
    SetSourceLevel(DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxdBm);
    ConstrainAndUpdateFrequencies();
    SetStartPower(DeviceDriver::getInfo(window->getDevice()).Limits.VNA.mindBm);
    SetStopPower(DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxdBm);
    SetPowerSweepFrequency(DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq);
    SetIFBandwidth(1000);
    SetDwellTime(0);
    SetAveraging(1);
    SetPoints(501);
    SetSweepType(SweepType::Frequency);
    Stop();
}

nlohmann::json VNA::toJSON()
{
    nlohmann::json j;
    // save current sweep/acquisition settings
    nlohmann::json sweep;
    sweep["type"] = SweepTypeToString(settings.sweepType).toStdString();
    nlohmann::json freq;
    freq["start"] = settings.Freq.start;
    freq["stop"] = settings.Freq.stop;
    freq["power"] = settings.Freq.excitation_power;
    freq["log"] = settings.Freq.logSweep;
    sweep["frequency"] = freq;
    sweep["single"] = singleSweep;
    nlohmann::json power;
    power["start"] = settings.Power.start;
    power["stop"] = settings.Power.stop;
    power["frequency"] = settings.Power.frequency;
    sweep["power"] = power;
    sweep["points"] = settings.npoints;
    sweep["IFBW"] = settings.bandwidth;
    j["sweep"] = sweep;

    j["traces"] = traceModel.toJSON();
    j["tiles"] = tiles->toJSON();
    j["markers"] = markerModel->toJSON();
    j["de-embedding"] = deembedding.toJSON();
    j["de-embedding_enabled"] = deembedding_active;
    return j;
}

void VNA::fromJSON(nlohmann::json j)
{
    if(j.is_null()) {
        return;
    }
    if(j.contains("traces")) {
        traceModel.fromJSON(j["traces"]);
    }
    if(j.contains("tiles")) {
        tiles->fromJSON(j["tiles"]);
    }
    if(j.contains("markers")) {
        markerModel->fromJSON(j["markers"]);
    }
    if(j.contains("de-embedding")) {
        deembedding.fromJSON(j["de-embedding"]);
        EnableDeembedding(j.value("de-embedding_enabled", true));
    } else {
        EnableDeembedding(false);
    }

    // sweep configuration has to go last so graphs can catch events from changed sweep
    if(j.contains("sweep")) {
        auto sweep = j["sweep"];
        // restore sweep settings, keep current value as default in case of missing entry
        SetPoints(sweep.value("points", settings.npoints));
        SetIFBandwidth(sweep.value("IFBW", settings.bandwidth));
        if(sweep.contains("frequency")) {
            auto freq = sweep["frequency"];
            SetStartFreq(freq.value("start", settings.Freq.start));
            SetStopFreq(freq.value("stop", settings.Freq.stop));
            SetSourceLevel(freq.value("power", settings.Freq.excitation_power));
            SetLogSweep(freq.value("log", settings.Freq.logSweep));
        }
        if(sweep.contains("power")) {
            auto power = sweep["power"];
            SetStartPower(power.value("start", settings.Power.start));
            SetStopPower(power.value("stop", settings.Power.stop));
            SetPowerSweepFrequency(power.value("frequency", settings.Power.frequency));
        }
        auto type = SweepTypeFromString(QString::fromStdString(sweep["type"]));
        if(type == SweepType::Last) {
            // default to frequency sweep
            type = SweepType::Frequency;
        }
        SetSweepType(type);
        SetSingleSweep(sweep.value("single", singleSweep));
    }
}

using namespace std;

void VNA::NewDatapoint(DeviceDriver::VNAMeasurement m)
{
    if(isActive != true) {
        // ignore
        return;
    }

    if(changingSettings) {
        // already setting new sweep settings, ignore incoming points from old settings
        return;
    }

    // Calculate sweep time
    if(m.pointNum == 0 && lastPoint > 0) {
        // new sweep started
        auto now = QDateTime::currentDateTimeUtc();
        auto sweepTime = lastStart.msecsTo(now);
        lastStart = now;
        qDebug() << "Sweep took"<<sweepTime<<"milliseconds";
    }

    emit newRawDatapoint(m);

    if(singleSweep && average.getLevel() == averages) {
        Stop();
        return;
    }

    auto m_avg = m;

    bool needsSegmentUpdate = false;
    if (settings.segments > 1) {
        // using multiple segments, adjust pointNum
        auto pointsPerSegment = ceil((double) settings.npoints / settings.segments);
        if (m_avg.pointNum == pointsPerSegment - 1) {
            needsSegmentUpdate = true;
        }
        m_avg.pointNum += pointsPerSegment * settings.activeSegment;
        if(m_avg.pointNum == settings.npoints - 1) {
            needsSegmentUpdate = true;
        }
    }

    if(m_avg.pointNum >= settings.npoints) {
        qWarning() << "Ignoring point with too large point number (" << m.pointNum << ")";
        return;
    }

    m_avg = average.process(m_avg);

    TraceMath::DataType type = TraceMath::DataType::Frequency;
    if(settings.zerospan) {
        type = TraceMath::DataType::TimeZeroSpan;

        // keep track of first point time
        if(m_avg.pointNum == 0) {
            settings.firstPointTime = m_avg.us;
            m_avg.us = 0;
        } else {
            m_avg.us -= settings.firstPointTime;
        }
    } else {
        switch(settings.sweepType) {
        case SweepType::Last:
        case SweepType::Frequency:
            type = TraceMath::DataType::Frequency;
            break;
        case SweepType::Power:
            type = TraceMath::DataType::Power;
            break;
        }
    }

    window->addStreamingData(m_avg, AppWindow::VNADataType::Raw, settings.zerospan);

    if(average.settled()) {
        setOperationPending(false);
    }

    if(calMeasuring) {
        if(average.currentSweep() == averages) {
            // this is the last averaging sweep, use values for calibration
            if(!calWaitFirst || m_avg.pointNum == 0) {
                calWaitFirst = false;
                cal.addMeasurements(calMeasurements, m_avg);
                if(m_avg.pointNum == settings.npoints - 1) {
                    calMeasuring = false;
                    cal.measurementsComplete();
                }
            }
        }
        int percentage = (((average.currentSweep() - 1) * 100) + (m_avg.pointNum + 1) * 100 / settings.npoints) / averages;
        emit calibrationMeasurementPercentage(percentage);
    }

    cal.correctMeasurement(m_avg);

    if(cal.getCaltype().type != Calibration::Type::None) {
        window->addStreamingData(m_avg, AppWindow::VNADataType::Calibrated, settings.zerospan);
    }

    traceModel.addVNAData(m_avg, type, false);
    if(deembedding_active) {
        deembedding.Deembed(m_avg);
        window->addStreamingData(m_avg, AppWindow::VNADataType::Deembedded, settings.zerospan);
        traceModel.addVNAData(m_avg, type, true);
    }


    emit dataChanged();
    if(m_avg.pointNum == settings.npoints - 1) {
        UpdateAverageCount();
        markerModel->updateMarkers();
    }

    if(m_avg.pointNum > 0 && m_avg.pointNum != (unsigned int) (lastPoint + 1)) {
        qWarning() << "Got point" << m_avg.pointNum << "but last received point was" << lastPoint << "("<<(m_avg.pointNum-lastPoint-1)<<"missed points)";
    }
    lastPoint = m_avg.pointNum;

    if (needsSegmentUpdate) {
        if( settings.activeSegment < settings.segments - 1) {
            settings.activeSegment++;
        } else {
            settings.activeSegment = 0;
        }
        SettingsChanged(false, 0);
    }
}

void VNA::UpdateAverageCount()
{
    lAverages->setText(QString::number(average.getLevel()) + "/");
}

void VNA::SettingsChanged(bool resetTraces, int delay)
{
    if(!running) {
        // not running, no need for any communication
        return;
    }
    if(isActive && window->getDevice()) {
        setOperationPending(true);
    }
    configurationTimer.start(delay);
    changingSettings = true;
    configurationTimerResetTraces = resetTraces;
    if(resetTraces) {
        ResetLiveTraces();
    }
}

void VNA::StartImpedanceMatching()
{
    auto dialog = new ImpedanceMatchDialog(*markerModel);
    if(AppWindow::showGUI()) {
        dialog->show();
    }
}

void VNA::StartMixedModeConversion()
{
    auto dialog = new MixedModeConversion(traceModel);
    connect(dialog, &MixedModeConversion::tracesCreated, [=](std::vector<Trace*> traces) {
        auto d = new TraceImportDialog(traceModel, traces);
        if(AppWindow::showGUI()) {
            d->show();
        }
    });
    if(AppWindow::showGUI()) {
        dialog->show();
    }
}

void VNA::SetSweepType(SweepType sw)
{
    if(settings.sweepType != sw) {
        settings.sweepType = sw;
        emit sweepTypeChanged(sw);
        ConstrainAndUpdateFrequencies();
        SettingsChanged();
    }
}

void VNA::SetStartFreq(double freq)
{
    settings.Freq.start = freq;
    if(settings.Freq.stop < freq) {
        settings.Freq.stop = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetStopFreq(double freq)
{
    settings.Freq.stop = freq;
    if(settings.Freq.start > freq) {
        settings.Freq.start = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetCenterFreq(double freq)
{
    auto old_span = settings.Freq.stop - settings.Freq.start;
    if (freq - old_span / 2 <= DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq) {
        // would shift start frequency below minimum
        settings.Freq.start = 0;
        settings.Freq.stop = 2 * freq;
    } else if(freq + old_span / 2 >= DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq) {
        // would shift stop frequency above maximum
        settings.Freq.start = 2 * freq - DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq;
        settings.Freq.stop = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq;
    } else {
        settings.Freq.start = freq - old_span / 2;
        settings.Freq.stop = freq + old_span / 2;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetSpan(double span)
{
    auto maxFreq = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq;
    auto old_center = (settings.Freq.start + settings.Freq.stop) / 2;
    if(old_center < DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq + span / 2) {
        // would shift start frequency below minimum
        settings.Freq.start = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq;
        settings.Freq.stop = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq + span;
    } else if(old_center > maxFreq - span / 2) {
        // would shift stop frequency above maximum
        settings.Freq.start = maxFreq - span;
        settings.Freq.stop = maxFreq;
    } else {
        settings.Freq.start = old_center - span / 2;
         settings.Freq.stop = settings.Freq.start + span;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetFullSpan()
{
    auto &pref = Preferences::getInstance();
    if(pref.Acquisition.fullSpanCalibratedRange && cal.getNumPoints() > 0) {
        // calibration is active, use it as the full span range
        settings.Freq.start = cal.getMinFreq();
        settings.Freq.stop = cal.getMaxFreq();
    } else {
        if(pref.Acquisition.fullSpanManual) {
            settings.Freq.start = pref.Acquisition.fullSpanStart;
            settings.Freq.stop = pref.Acquisition.fullSpanStop;
        } else {
            settings.Freq.start = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq;
            settings.Freq.stop = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq;
        }
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetZeroSpan()
{
    auto center = (settings.Freq.start + settings.Freq.stop) / 2;
    SetStartFreq(center);
    SetStopFreq(center);
}

void VNA::SpanZoomIn()
{
    auto center = (settings.Freq.start + settings.Freq.stop) / 2;
    auto old_span = settings.Freq.stop - settings.Freq.start;
    settings.Freq.start = center - old_span / 4;
    settings.Freq.stop = center + old_span / 4;
    ConstrainAndUpdateFrequencies();
}

void VNA::SpanZoomOut()
{
    auto center = (settings.Freq.start + settings.Freq.stop) / 2;
    auto old_span = settings.Freq.stop - settings.Freq.start;
    if(center > old_span) {
        settings.Freq.start = center - old_span;
    } else {
        settings.Freq.start = 0;
    }
    settings.Freq.stop = center + old_span;
    ConstrainAndUpdateFrequencies();
}

bool VNA::SpanMatchCal()
{
    if(cal.getCaltype().type == Calibration::Type::None) {
        // no cal, nothing to adjust
        return false;
    }
    SetStartFreq(cal.getMinFreq());
    SetStopFreq(cal.getMaxFreq());
    SetPoints(cal.getNumPoints());
    UpdateCalWidget();
    return true;
}

void VNA::SetLogSweep(bool log)
{
    if(settings.Freq.logSweep != log) {
        settings.Freq.logSweep = log;
        emit logSweepChanged(log);
        SettingsChanged();
    }
}


void VNA::SetSourceLevel(double level)
{
    if(level > DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxdBm) {
        level = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxdBm;
    } else if(level < DeviceDriver::getInfo(window->getDevice()).Limits.VNA.mindBm) {
        level = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.mindBm;
    }
    emit sourceLevelChanged(level);
    settings.Freq.excitation_power = level;
    SettingsChanged();
}

void VNA::SetDwellTime(double time) {
    if(time > DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxDwellTime) {
        time = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxDwellTime;
    }
    emit dwellTimeChanged(time);
    settings.dwellTime = time;
    SettingsChanged();
}

void VNA::SetStartPower(double level)
{
    settings.Power.start = level;
    emit startPowerChanged(level);
    ConstrainAndUpdateFrequencies();
}

void VNA::SetStopPower(double level)
{
    settings.Power.stop = level;
    emit stopPowerChanged(level);
    ConstrainAndUpdateFrequencies();
}

void VNA::SetPowerSweepFrequency(double freq)
{
    settings.Power.frequency = freq;
    emit powerSweepFrequencyChanged(freq);
    ConstrainAndUpdateFrequencies();
    SettingsChanged();
}

void VNA::SetPoints(unsigned int points)
{
    unsigned int maxPoints = Preferences::getInstance().Acquisition.allowSegmentedSweep ? UINT16_MAX : DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxPoints;
    if(points > maxPoints) {
        points = maxPoints;
    } else if (points < 2) {
        points = 2;
    }
    if (points > DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxPoints) {
        // needs segmented sweep
        settings.segments = ceil((double) points / DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxPoints);
        settings.activeSegment = 0;
    } else {
        // can fit all points into one segment
        settings.segments = 1;
        settings.activeSegment = 0;
    }
    emit pointsChanged(points);
    deembedding.setPointsInSweepForMeasurement(points);
    settings.npoints = points;
    SettingsChanged();
}

void VNA::SetIFBandwidth(double bandwidth)
{
    if(bandwidth > DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxIFBW) {
        bandwidth = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxIFBW;
    } else if(bandwidth < DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minIFBW) {
        bandwidth = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minIFBW;
    }
    settings.bandwidth = bandwidth;
    emit IFBandwidthChanged(settings.bandwidth);
    SettingsChanged();
}

void VNA::SetAveraging(unsigned int averages)
{
    this->averages = averages;
    average.setAverages(averages);
    emit averagingChanged(averages);
    UpdateAverageCount();
    if(isActive && window->getDevice()) {
        setOperationPending(!average.settled());
    }
}

void VNA::ExcitationRequired()
{
    if(!Preferences::getInstance().Acquisition.alwaysExciteAllPorts) {
        for(unsigned int i=1;i<=DeviceDriver::getInfo(window->getDevice()).Limits.VNA.ports;i++) {
            auto required = traceModel.PortExcitationRequired(i);
            auto set = find(settings.excitedPorts.begin(), settings.excitedPorts.end(), i) != settings.excitedPorts.end();
            if(required != set) {
                // Required port excitation changed
                SettingsChanged();
                break;
            }
        }
    }
}

void VNA::DisableCalibration()
{
    cal.deactivate();
}

void VNA::ApplyCalibration(Calibration::CalType type)
{
    if(cal.canCompute(type)) {
        try {
            cal.compute(type);
        } catch (runtime_error &e) {
            InformationBox::ShowError("Calibration failure", e.what());
            DisableCalibration();
        }
    } else {
        if(settings.sweepType == SweepType::Frequency) {
            // Not all required traces available
            InformationBox::ShowMessageBlocking("Missing calibration measurements", "Not all calibration measurements for this type of calibration have been taken. The calibration can be enabled after the missing measurements have been acquired.");
            DisableCalibration();
            cal.edit();
        } else {
            // Not all required traces available
            InformationBox::ShowMessageBlocking("Missing calibration measurements", "Not all calibration measurements for this type of calibration have been taken. Please switch to frequency sweep to take these measurements.");
            DisableCalibration();
        }
    }
}

void VNA::StartCalibrationMeasurements(std::set<CalibrationMeasurement::Base*> m)
{
    if(!window->getDevice()) {
        return;
    }
    // Stop sweep
    running = false;
    ConfigureDevice();
    calMeasurements = m;
    // Delete any already captured data of this measurement
    cal.clearMeasurements(m);
    calWaitFirst = true;
    // show messagebox
    QString text = "Measuring ";
    if(m.size() == 1) {
        text.append("\"");
        text.append(CalibrationMeasurement::Base::TypeToString((*m.begin())->getType()));
        text.append("\" parameters.");
    } else {
        text.append("multiple calibration standards.");
    }
    calDialog->setLabelText(text);

    // Trigger sweep to start from beginning
    running = true;
    ConfigureDevice(true, [=](bool){
        // enable calibration measurement only in transmission callback (prevents accidental sampling of data which was still being processed)
        calMeasuring = true;
    });
}

void VNA::SetupSCPI()
{
    SCPINode::add(new SCPICommand("SWEEP", [=](QStringList params) -> QString {
        if(params.size() >= 1) {
            if(params[0] == "FREQUENCY") {
                SetSweepType(SweepType::Frequency);
                return SCPI::getResultName(SCPI::Result::Empty);
            } else if(params[0] == "POWER") {
                SetSweepType(SweepType::Power);
                return SCPI::getResultName(SCPI::Result::Empty);
            }
        }
        // either no parameter or invalid
        return SCPI::getResultName(SCPI::Result::Error);
    }, [=](QStringList) -> QString {
        return settings.sweepType == SweepType::Frequency ? "FREQUENCY" : "POWER";
    }));
    auto scpi_freq = new SCPINode("FREQuency");
    SCPINode::add(scpi_freq);
    scpi_freq->add(new SCPICommand("SPAN", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetSpan(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.stop - settings.Freq.start, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("START", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetStartFreq(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.start, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("CENTer", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetCenterFreq(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number((settings.Freq.start + settings.Freq.stop)/2, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("STOP", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetStopFreq(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.stop, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("FULL", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        SetFullSpan();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    scpi_freq->add(new SCPICommand("ZERO", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        SetZeroSpan();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    auto scpi_power = new SCPINode("POWer");
    SCPINode::add(scpi_power);
    scpi_power->add(new SCPICommand("START", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetStartPower(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Power.start);
    }));
    scpi_power->add(new SCPICommand("STOP", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetStopPower(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Power.stop);
    }));
    SCPINode::add(new SCPICommand("SWEEPTYPE", [=](QStringList params) -> QString {
        if(params.size() >= 1) {
            if(params[0] == "LIN") {
                SetLogSweep(false);
                return SCPI::getResultName(SCPI::Result::Empty);
            } else if(params[0] == "LOG") {
                SetLogSweep(true);
                return SCPI::getResultName(SCPI::Result::Empty);
            }
        }
        // either no parameter or invalid
        return SCPI::getResultName(SCPI::Result::Error);
    }, [=](QStringList) -> QString {
        return settings.Freq.logSweep ? "LOG" : "LIN";
    }));
    auto scpi_acq = new SCPINode("ACQuisition");
    SCPINode::add(scpi_acq);
    scpi_acq->add(new SCPICommand("RUN", [=](QStringList) -> QString {
        Run();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        return running ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    scpi_acq->add(new SCPICommand("STOP", [=](QStringList) -> QString {
        Stop();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    scpi_acq->add(new SCPICommand("IFBW", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetIFBandwidth(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.bandwidth);
    }));
    scpi_acq->add(new SCPICommand("POINTS", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetPoints(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.npoints);
    }));
    scpi_acq->add(new SCPICommand("AVG", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetAveraging(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(averages);
    }));
    scpi_acq->add(new SCPICommand("AVGLEVel", nullptr, [=](QStringList) -> QString {
        return QString::number(average.getLevel());
    }));
    scpi_acq->add(new SCPICommand("FINished", nullptr, [=](QStringList) -> QString {
        return average.settled() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    scpi_acq->add(new SCPICommand("LIMit", nullptr, [=](QStringList) -> QString {
        return tiles->allLimitsPassing() ? "PASS" : "FAIL";
    }));
    scpi_acq->add(new SCPICommand("SINGLE", [=](QStringList params) -> QString {
        bool single;
        if(!SCPI::paramToBool(params, 0, single)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetSingleSweep(single);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return singleSweep ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    auto scpi_stim = new SCPINode("STIMulus");
    SCPINode::add(scpi_stim);
    scpi_stim->add(new SCPICommand("LVL", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetSourceLevel(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.excitation_power);
    }));
    scpi_stim->add(new SCPICommand("FREQuency", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetPowerSweepFrequency(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Power.frequency, 'f', 0);
    }));
    SCPINode::add(traceWidget);
    SCPINode::add(&cal);
    cal.add(new SCPICommand("BUSy", nullptr, [=](QStringList) -> QString {
        return CalibrationMeasurementActive() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));

    SCPINode::add(&deembedding);
}

void VNA::ConstrainAndUpdateFrequencies()
{
    if(settings.Freq.stop > DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq) {
        settings.Freq.stop = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq;
    }
    if(settings.Freq.start > settings.Freq.stop) {
        settings.Freq.start = settings.Freq.stop;
    }
    if(settings.Freq.start < DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq) {
        settings.Freq.start = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq;
    }
    settings.zerospan = (settings.sweepType == SweepType::Frequency && settings.Freq.start == settings.Freq.stop)
            || (settings.sweepType == SweepType::Power && settings.Power.start == settings.Power.stop);
    emit startFreqChanged(settings.Freq.start);
    emit stopFreqChanged(settings.Freq.stop);
    emit spanChanged(settings.Freq.stop - settings.Freq.start);
    emit centerFreqChanged((settings.Freq.stop + settings.Freq.start)/2);
    SettingsChanged();
}

void VNA::LoadSweepSettings()
{
    auto& pref = Preferences::getInstance();
    QSettings s;
    // frequency sweep settings
    settings.Freq.start = s.value("SweepFreqStart", pref.Startup.DefaultSweep.f_start).toULongLong();
    settings.Freq.stop = s.value("SweepFreqStop", pref.Startup.DefaultSweep.f_stop).toULongLong();
    SetSourceLevel(s.value("SweepFreqLevel", pref.Startup.DefaultSweep.f_excitation).toDouble());
    SetLogSweep(s.value("SweepFreqLog", pref.Startup.DefaultSweep.logSweep).toBool());
    // power sweep settings
    SetStartPower(s.value("SweepPowerStart", pref.Startup.DefaultSweep.dbm_start).toDouble());
    SetStopPower(s.value("SweepPowerStop", pref.Startup.DefaultSweep.dbm_stop).toDouble());
    SetPowerSweepFrequency(s.value("SweepPowerFreq", pref.Startup.DefaultSweep.dbm_freq).toULongLong());
    SetPoints(s.value("SweepPoints", pref.Startup.DefaultSweep.points).toInt());
    SetIFBandwidth(s.value("SweepBandwidth", pref.Startup.DefaultSweep.bandwidth).toUInt());
    SetAveraging(s.value("SweepAveraging", pref.Startup.DefaultSweep.averaging).toInt());
    SetDwellTime(s.value("SweepDwellTime", pref.Startup.DefaultSweep.dwellTime).toDouble());
    ConstrainAndUpdateFrequencies();
    auto typeString = s.value("SweepType", pref.Startup.DefaultSweep.type).toString();
    if(typeString == "Power") {
        SetSweepType(SweepType::Power);
    } else {
        SetSweepType(SweepType::Frequency);
    }
}

void VNA::StoreSweepSettings()
{
    QSettings s;
    s.setValue("SweepType", settings.sweepType == SweepType::Frequency ? "Frequency" : "Power");
    s.setValue("SweepFreqStart", static_cast<unsigned long long>(settings.Freq.start));
    s.setValue("SweepFreqStop", static_cast<unsigned long long>(settings.Freq.stop));
    s.setValue("SweepFreqLevel", settings.Freq.excitation_power);
    s.setValue("SweepFreqLog", settings.Freq.logSweep);
    s.setValue("SweepPowerStart", settings.Power.start);
    s.setValue("SweepPowerStop", settings.Power.stop);
    s.setValue("SweepPowerFreq", static_cast<unsigned long long>(settings.Power.frequency));
    s.setValue("SweepBandwidth", settings.bandwidth);
    s.setValue("SweepPoints", settings.npoints);
    s.setValue("SweepAveraging", averages);
    s.setValue("SweepDwellTime", settings.dwellTime);
}

void VNA::UpdateCalWidget()
{
    calLabel->setStyleSheet(getCalStyle());
    calLabel->setToolTip(getCalToolTip());
}

void VNA::ConstrainAllSettings()
{
    auto maxFreq = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxFreq;
    auto minFreq = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minFreq;
    auto maxPower = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxdBm;
    auto minPower = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.mindBm;
    auto maxIFBW = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxIFBW;
    auto minIFBW = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.minIFBW;
    auto maxDwell = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxDwellTime;
    auto maxPoints = DeviceDriver::getInfo(window->getDevice()).Limits.VNA.maxPoints;
    Util::constrain(settings.Freq.start, minFreq, maxFreq);
    Util::constrain(settings.Freq.stop, minFreq, maxFreq);
    Util::constrain(settings.Freq.excitation_power, minPower, maxPower);
    Util::constrain(settings.bandwidth, minIFBW, maxIFBW);
    Util::constrain(settings.dwellTime, 0.0, maxDwell);
    Util::constrain(settings.npoints, (unsigned int) 0, maxPoints);
    Util::constrain(settings.Power.frequency, minFreq, maxFreq);
    Util::constrain(settings.Power.start, minPower, maxPower);
    Util::constrain(settings.Power.stop, minPower, maxPower);
    emit startFreqChanged(settings.Freq.start);
    emit stopFreqChanged(settings.Freq.stop);
    emit sourceLevelChanged(settings.Freq.excitation_power);
    emit IFBandwidthChanged(settings.bandwidth);
    emit dwellTimeChanged(settings.dwellTime);
    emit pointsChanged(settings.npoints);
    emit powerSweepFrequencyChanged(settings.Power.frequency);
    emit startPowerChanged(settings.Power.start);
    emit stopPowerChanged(settings.Power.stop);
}

void VNA::createDefaultTracesAndGraphs(int ports)
{
    auto getDefaultColor = [](int ports, int i, int j)->QColor {
        // Default colors for up to four ports, ensures that e.g. S21 always has the same color
        const array<vector<QColor>, 4> defaultColors = {{
            {Qt::yellow},
            {Qt::yellow, Qt::blue, Qt::green, Qt::red},
            {Qt::yellow, Qt::blue, Qt::cyan, Qt::green, Qt::red, Qt::darkGreen, Qt::darkBlue, Qt::darkYellow, Qt::magenta},
            {Qt::yellow, Qt::blue, Qt::cyan, Qt::darkCyan, Qt::green, Qt::red, Qt::darkGreen, Qt::gray, Qt::darkBlue, Qt::darkYellow, Qt::magenta, Qt::darkMagenta, Qt::cyan, Qt::darkGray, Qt::lightGray, Qt::darkRed},
        }};

        if(ports >= 1 && ports <= 4) {
            return defaultColors[ports-1][i*ports+j];
        } else {
            // not enough predefined colors available for all ports, just cycle through list
            const array<QColor, 16> list = {{
                Qt::yellow, Qt::blue, Qt::green, Qt::red, Qt::cyan, Qt::magenta, Qt::yellow, Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::gray, Qt::darkCyan, Qt::darkMagenta, Qt::darkYellow, Qt::darkGray, Qt::lightGray
            }};
            auto index = (i*ports+j) % list.size();
            return list[index];
        }
    };

    if(ports > 1) {
        vector<vector<TracePlot*>> plots;
        for(int i=0;i<ports;i++) {
            plots.push_back(vector<TracePlot*>());
            for(int j=0;j<ports;j++) {
                QString param = "S"+QString::number(i+1)+QString::number(j+1);
                auto trace = new Trace(param, getDefaultColor(ports, i, j), param);
                traceModel.addTrace(trace);
                TracePlot *plot = TracePlot::createDefaultPlotForTrace(traceModel, trace);
                plot->updateSpan(settings.Freq.start, settings.Freq.stop);
                plot->enableTrace(trace, true);
                plots[i].push_back(plot);
            }
        }
        // Add created graphs to tiles
        tiles->clear();
        TileWidget *tile = tiles;
        for(int i=0;i<ports;i++) {
            TileWidget *row;
            if(i != ports - 1) {
                // this is not the last row, split tile
                tile->splitVertically();
                row = tile->Child1();
                tile = tile->Child2();
            } else {
                row = tile;
            }
            for(int j=0;j<ports;j++) {
                TileWidget *graphTile;
                if(j != ports - 1) {
                    row->splitHorizontally();
                    graphTile = row->Child1();
                    row = row->Child2();
                } else {
                    graphTile = row;
                }
                graphTile->setPlot(plots[i][j]);
            }
        }
        if(ports >= 3) {
            // default split at the middle does not result in all plots being the same size, adjust
            tile = tiles;
            for(int i=0;i<ports;i++) {
                TileWidget *rowTile;
                if(i < ports - 1) {
                    tile->setSplitPercentage(100 / (ports - i));
                    rowTile = tile->Child1();
                } else {
                    rowTile = tile;
                }
                for(int j=0;j<ports-1;j++) {
                    rowTile->setSplitPercentage(100 / (ports - j));
                    rowTile = rowTile->Child2();
                }
                tile = tile->Child2();
            }
        }
    } else if(ports == 1) {
        tiles->clear();
        tiles->splitHorizontally();
        auto trace = new Trace("S11", getDefaultColor(ports, 0, 0), "S11");
        traceModel.addTrace(trace);
        auto smithchart = new TraceSmithChart(traceModel);
        auto xyplot = new TraceXYPlot(traceModel);
        smithchart->updateSpan(settings.Freq.start, settings.Freq.stop);
        smithchart->enableTrace(trace, true);
        xyplot->updateSpan(settings.Freq.start, settings.Freq.stop);
        xyplot->enableTrace(trace, true);
        tiles->Child1()->setPlot(smithchart);
        tiles->Child2()->setPlot(xyplot);
    }
}

void VNA::EnableDeembedding(bool enable)
{
    deembedding_active = enable;
    enableDeembeddingAction->blockSignals(true);
    enableDeembeddingAction->setChecked(enable);
    enableDeembeddingAction->blockSignals(false);
    for(auto t : traceModel.getLiveTraces()) {
        if(enable) {
            t->setDeembeddingActive(true);
        } else {
            t->clearDeembedding();
        }
    }

}

void VNA::setAveragingMode(Averaging::Mode mode)
{
    average.setMode(mode);
}

void VNA::preset()
{
    for(auto t : traceModel.getTraces()) {
        if(Trace::isVNAParameter(t->name())) {
            traceModel.removeTrace(t);
        }
    }
    // Create default traces
    createDefaultTracesAndGraphs(DeviceDriver::getInfo(window->getDevice()).Limits.VNA.ports);
}

void VNA::deviceInfoUpdated()
{
    if(window->getDevice()->supports(DeviceDriver::Feature::VNADwellTime)) {
        acquisitionDwellTime->setEnabled(true);
    } else {
        acquisitionDwellTime->setEnabled(false);
    }
    ConstrainAllSettings();
}

QString VNA::SweepTypeToString(VNA::SweepType sw)
{
    switch(sw) {
    case SweepType::Frequency: return "Frequency";
    case SweepType::Power: return "Power";
    default: return "Unknown";
    }
}

VNA::SweepType VNA::SweepTypeFromString(QString s)
{
    for(int i=0;i<(int)SweepType::Last;i++) {
        if(SweepTypeToString((SweepType) i) == s) {
            return (SweepType) i;
        }
    }
    // not found
    return SweepType::Last;
}


void VNA::UpdateStatusbar()
{
    if(cal.getCaltype().type != Calibration::Type::None) {
        QFileInfo fi(cal.getCurrentCalibrationFile());
        auto filename = fi.fileName();
        if(filename.isEmpty()) {
            filename = "Unsaved";
        }
        setStatusbarMessage("Calibration: "+filename);
    } else {
        setStatusbarMessage("Calibration: -");
    }
}

void VNA::SetSingleSweep(bool single)
{
    if(singleSweep != single) {
        singleSweep = single;
        if(single) {
            Run();
        }
        emit singleSweepChanged(single);
    } else {
        // if already set to single, a second single command triggers a new sweep
        if(single && !running) {
            Run();
        }
    }
}

void VNA::Run()
{
    running = true;
    ConfigureDevice();
}

void VNA::Stop()
{
    running = false;
    ConfigureDevice(false);
    setOperationPending(false);
}

void VNA::ConfigureDevice(bool resetTraces, std::function<void(bool)> cb)
{
    configurationTimer.stop();
    if(running) {
        if (resetTraces) {
            ResetLiveTraces();
        }
        changingSettings = true;
        // assemble VNA protocol settings
        DeviceDriver::VNASettings s = {};
        s.IFBW = settings.bandwidth;
        if(Preferences::getInstance().Acquisition.alwaysExciteAllPorts) {
            for(unsigned int i=1;i<=DeviceDriver::getInfo(window->getDevice()).Limits.VNA.ports;i++) {
                s.excitedPorts.push_back(i);
            }
        } else {
            if(deembedding.isMeasuring()) {
                // use the required ports for the de-embedding measurement
                for(auto p : deembedding.getAffectedPorts()) {
                    s.excitedPorts.push_back(p);
                }
            } else {
                // use the required ports from the trace model
                for(unsigned int i=1;i<=DeviceDriver::getInfo(window->getDevice()).Limits.VNA.ports;i++) {
                    if(traceModel.PortExcitationRequired(i)) {
                        s.excitedPorts.push_back(i);
                    }
                }
            }
        }
        settings.excitedPorts = s.excitedPorts;

        double start = settings.sweepType == SweepType::Frequency ? settings.Freq.start : settings.Power.start;
        double stop = settings.sweepType == SweepType::Frequency ? settings.Freq.stop : settings.Power.stop;
        int npoints = settings.npoints;
        emit traceModel.SpanChanged(start, stop);
        if (settings.segments > 1) {
            // more than one segment, adjust start/stop
            npoints = ceil((double) settings.npoints / settings.segments);
            unsigned int segmentStartPoint = npoints * settings.activeSegment;
            unsigned int segmentStopPoint = segmentStartPoint + npoints - 1;
            if(segmentStopPoint >= settings.npoints) {
                segmentStopPoint = settings.npoints - 1;
                npoints = settings.npoints - segmentStartPoint;
            }
            auto seg_start = Util::Scale<double>(segmentStartPoint, 0, settings.npoints - 1, start, stop);
            auto seg_stop = Util::Scale<double>(segmentStopPoint, 0, settings.npoints - 1, start, stop);
            start = seg_start;
            stop = seg_stop;
        }

        if(settings.sweepType == SweepType::Frequency) {
            s.freqStart = start;
            s.freqStop = stop;
            s.points = npoints;
            s.dBmStart = settings.Freq.excitation_power;
            s.dBmStop = settings.Freq.excitation_power;
            s.logSweep = settings.Freq.logSweep;
        } else if(settings.sweepType == SweepType::Power) {
            s.freqStart = settings.Power.frequency;
            s.freqStop = settings.Power.frequency;
            s.points = npoints;
            s.dBmStart = start;
            s.dBmStop = stop;
            s.logSweep = false;
        }
        s.dwellTime = settings.dwellTime;
        if(window->getDevice() && isActive) {
            window->getDevice()->setVNA(s, [=](bool res){
                // device received command, reset traces now
                if (resetTraces) {
                    ResetLiveTraces();
                }
                if(cb) {
                    cb(res);
                }
                changingSettings = false;
                lastStart = QDateTime::currentDateTimeUtc();
                lastPoint = -1;
            });
            emit sweepStarted();
        } else {
            // no device, unable to start sweep
            emit sweepStopped();
            changingSettings = false;
        }
    } else {
        if(window->getDevice()) {
            changingSettings = true;
            window->getDevice()->setIdle([=](bool){
                changingSettings = false;
            });
        } else {
            emit sweepStopped();
            changingSettings = false;
        }
        emit sweepStopped();
    }
}

void VNA::ResetLiveTraces()
{
    settings.activeSegment = 0;
    average.reset(settings.npoints);
    traceModel.clearLiveData();
    UpdateAverageCount();
    UpdateCalWidget();
    if(isActive && window->getDevice()) {
        setOperationPending(true);
    }
}

bool VNA::LoadCalibration(QString filename)
{
    return cal.fromFile(filename);
}

bool VNA::SaveCalibration(QString filename)
{
    return cal.toFile(filename);
}
