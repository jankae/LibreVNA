#include "vna.h"
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
#include "unit.h"
#include <queue>
#include "CustomWidgets/toggleswitch.h"
#include "Device/manualcontroldialog.h"
#include "Traces/tracemodel.h"
#include "tracewidgetvna.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracexyplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include "CustomWidgets/siunitedit.h"
#include <QDockWidget>
#include "Traces/Marker/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "Calibration/calibrationtracedialog.h"
#include "ui_main.h"
#include "Device/firmwareupdatedialog.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QActionGroup>
#include <QErrorMessage>
#include "CustomWidgets/informationbox.h"
#include <QDebug>
#include "Deembedding/manualdeembeddingdialog.h"
#include "Calibration/manualcalibrationdialog.h"

VNA::VNA(AppWindow *window)
    : Mode(window, "Vector Network Analyzer"),
      SCPINode("VNA"),
      deembedding(traceModel),
      central(new TileWidget(traceModel))
{
    averages = 1;
    calValid = false;
    calMeasuring = false;
    calDialog.reset();
    calEdited = false;

    // Create default traces
    auto tS11 = new Trace("S11", Qt::yellow);
    tS11->fromLivedata(Trace::LivedataType::Overwrite, Trace::LiveParameter::S11);
    traceModel.addTrace(tS11);
    auto tS12 = new Trace("S12", Qt::blue);
    tS12->fromLivedata(Trace::LivedataType::Overwrite, Trace::LiveParameter::S12);
    traceModel.addTrace(tS12);
    auto tS21 = new Trace("S21", Qt::green);
    tS21->fromLivedata(Trace::LivedataType::Overwrite, Trace::LiveParameter::S21);
    traceModel.addTrace(tS21);
    auto tS22 = new Trace("S22", Qt::red);
    tS22->fromLivedata(Trace::LivedataType::Overwrite, Trace::LiveParameter::S22);
    traceModel.addTrace(tS22);

    auto tracesmith1 = new TraceSmithChart(traceModel);
    tracesmith1->enableTrace(tS11, true);

    auto tracesmith2 = new TraceSmithChart(traceModel);
    tracesmith2->enableTrace(tS22, true);

    auto traceXY1 = new TraceXYPlot(traceModel);
    traceXY1->enableTrace(tS12, true);
    auto traceXY2 = new TraceXYPlot(traceModel);
    traceXY2->enableTrace(tS21, true);

    connect(this, &VNA::graphColorsChanged, [=](){
        for (auto p : TracePlot::getPlots()) {
            p->updateGraphColors();
        }
    });

    connect(&traceModel, &TraceModel::requiredExcitation, this, &VNA::ExcitationRequired);

    central->splitVertically();
    central->Child1()->splitHorizontally();
    central->Child2()->splitHorizontally();
    central->Child1()->Child1()->setPlot(tracesmith1);
    central->Child1()->Child2()->setPlot(traceXY1);
    central->Child2()->Child1()->setPlot(traceXY2);
    central->Child2()->Child2()->setPlot(tracesmith2);

    // Create menu entries and connections
    auto calMenu = new QMenu("Calibration", window);
    window->menuBar()->insertMenu(window->getUi()->menuWindow->menuAction(), calMenu);
    actions.insert(calMenu->menuAction());
    auto calLoad = calMenu->addAction("Load");
    saveCal = calMenu->addAction("Save");
    calMenu->addSeparator();
    saveCal->setEnabled(false);

    connect(calLoad, &QAction::triggered, [=](){
        cal.openFromFile();
        if(cal.getType() == Calibration::Type::None) {
            DisableCalibration();
        } else {
            ApplyCalibration(cal.getType());
        }
        calEdited = false;
    });

    connect(saveCal, &QAction::triggered, [=](){
        if(cal.saveToFile()) {
            calEdited = false;
        }
    });

    auto calDisable = calMenu->addAction("Disabled");
    calDisable->setCheckable(true);
    calDisable->setChecked(true);
    calMenu->addSeparator();
    auto calData = calMenu->addAction("Calibration Measurements");
    connect(calData, &QAction::triggered, [=](){
       StartCalibrationDialog();
    });

    auto calEditKit = calMenu->addAction("Edit Calibration Kit");
    connect(calEditKit, &QAction::triggered, [=](){
        cal.getCalibrationKit().edit([=](){
            if(calValid) {
                ApplyCalibration(cal.getType());
            }
        });
    });

    calMenu->addSeparator();

    auto calImportTerms = calMenu->addAction("Import error terms as traces");
    calImportTerms->setEnabled(false);
    connect(calImportTerms, &QAction::triggered, [=](){
        auto import = new TraceImportDialog(traceModel, cal.getErrorTermTraces());
        import->show();
    });
    auto calImportMeas = calMenu->addAction("Import measurements as traces");
    calImportMeas->setEnabled(false);
    connect(calImportMeas, &QAction::triggered, [=](){
        auto import = new TraceImportDialog(traceModel, cal.getMeasurementTraces());
        import->show();
    });

    calMenu->addSeparator();
    auto calApplyToTraces = calMenu->addAction("Apply to traces...");
    calApplyToTraces->setEnabled(false);
    connect(calApplyToTraces, &QAction::triggered, [=]() {
        auto manualCalibration = new ManualCalibrationDialog(traceModel, &cal);
        manualCalibration->show();
    });

//    portExtension.setCalkit(&cal.getCalibrationKit());

    // De-embedding menu
    auto menuDeembed = new QMenu("De-embedding", window);
    window->menuBar()->insertMenu(window->getUi()->menuWindow->menuAction(), menuDeembed);
    actions.insert(menuDeembed->menuAction());
    auto confDeembed = menuDeembed->addAction("Setup...");
    connect(confDeembed, &QAction::triggered, &deembedding, &Deembedding::configure);

    enableDeembeddingAction = menuDeembed->addAction("De-embed VNA samples");
    enableDeembeddingAction->setCheckable(true);
    enableDeembeddingAction->setEnabled(false);
    connect(enableDeembeddingAction, &QAction::toggled, this, &VNA::EnableDeembedding);

    auto manualDeembed = menuDeembed->addAction("De-embed traces...");
    manualDeembed->setEnabled(false);
    connect(manualDeembed, &QAction::triggered, [=]() {
        auto manualDeembedding = new ManualDeembeddingDialog(traceModel, &deembedding);
        manualDeembedding->show();
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

    // Tools menu
    auto toolsMenu = new QMenu("Tools", window);
    window->menuBar()->insertMenu(window->getUi()->menuWindow->menuAction(), toolsMenu);
    actions.insert(toolsMenu->menuAction());
    auto impedanceMatching = toolsMenu->addAction("Impedance Matching");
    connect(impedanceMatching, &QAction::triggered, this, &VNA::StartImpedanceMatching);

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
           auto key = "DefaultCalibration"+window->getDevice()->serial();
           QSettings settings;
           auto filename = QFileDialog::getOpenFileName(nullptr, "Load calibration data", settings.value(key).toString(), "Calibration files (*.cal)", nullptr, QFileDialog::DontUseNativeDialog);
           if(!filename.isEmpty()) {
               settings.setValue(key, filename);
               removeDefaultCal->setEnabled(true);
           }
       }
    });
    connect(removeDefaultCal, &QAction::triggered, [=](){
        QSettings settings;
        settings.remove("DefaultCalibration"+window->getDevice()->serial());
        removeDefaultCal->setEnabled(false);
    });


    // Sweep toolbar
    auto tb_sweep = new QToolBar("Sweep");

    std::vector<QAction*> frequencySweepActions;
    std::vector<QAction*> powerSweepActions;

    tb_sweep->addWidget(new QLabel("Sweep type:"));
    auto cbSweepType = new QComboBox();
    cbSweepType->addItem("Frequency");
    cbSweepType->addItem("Power");
    tb_sweep->addWidget(cbSweepType);

    auto eStart = new SIUnitEdit("Hz", " kMG", 6);
    // calculate width required with expected string length
    auto width = QFontMetrics(eStart->font()).width("3.00000GHz") + 15;
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

    // power sweep widgets
    auto sbPowerLow = new QDoubleSpinBox();
    width = QFontMetrics(sbPowerLow->font()).width("-30.00dBm") + 20;
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
    width = QFontMetrics(sbPowerHigh->font()).width("-30.00dBm") + 20;
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
    width = QFontMetrics(ePowerFreq->font()).width("3.00000GHz") + 15;
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
    width = QFontMetrics(dbm->font()).width("-30.00dBm") + 20;
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
    points->setFixedWidth(55);
    points->setRange(1, 9999);
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

    auto eBandwidth = new SIUnitEdit("Hz", " k", 3);
    eBandwidth->setFixedWidth(70);
    eBandwidth->setToolTip("IF bandwidth");
    connect(eBandwidth, &SIUnitEdit::valueChanged, this, &VNA::SetIFBandwidth);
    connect(this, &VNA::IFBandwidthChanged, eBandwidth, &SIUnitEdit::setValueQuiet);
    tb_acq->addWidget(new QLabel("IF BW:"));
    tb_acq->addWidget(eBandwidth);

    tb_acq->addWidget(new QLabel("Averaging:"));
    lAverages = new QLabel("0/");
    tb_acq->addWidget(lAverages);
    auto sbAverages = new QSpinBox;
    sbAverages->setRange(1, 99);
    sbAverages->setFixedWidth(40);
    connect(sbAverages, qOverload<int>(&QSpinBox::valueChanged), this, &VNA::SetAveraging);
    connect(this, &VNA::averagingChanged, sbAverages, &QSpinBox::setValue);
    tb_acq->addWidget(sbAverages);

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
    auto calMenuGroup = new QActionGroup(this);
    calMenuGroup->addAction(calDisable);
    for(auto type : Calibration::Types()) {
        cbType->addItem(Calibration::TypeToString(type), (int) type);
        auto menuAction = new QAction(Calibration::TypeToString(type), calMenu);
        calMenuGroup->addAction(menuAction);
        connect(menuAction, &QAction::triggered, [=](){
            ApplyCalibration(type);
        });
        connect(this, &VNA::CalibrationApplied, [=](Calibration::Type applied){
             if(type == applied) {
                 menuAction->setChecked(true);
             }
        });
        menuAction->setCheckable(true);
        calMenu->insertAction(calDisable, menuAction);
    }

    auto calToolbarLambda = [=]() {
        if(cbEnableCal->isChecked()) {
            // Get requested calibration type from combobox
            ApplyCalibration((Calibration::Type) cbType->itemData(cbType->currentIndex()).toInt());
        } else {
            DisableCalibration();
        }
    };

    // Calibration connections
    connect(cbEnableCal, &QCheckBox::stateChanged, calToolbarLambda);
    connect(cbType, qOverload<int>(&QComboBox::currentIndexChanged), calToolbarLambda);
    connect(this, &VNA::CalibrationDisabled, [=](){
        cbType->blockSignals(true);
        cbEnableCal->blockSignals(true);
        calDisable->setChecked(true);
        cbEnableCal->setCheckState(Qt::CheckState::Unchecked);
        // visually indicate loss of calibration
        // cal. file unknown at this moment
        UpdateCalWidget();
        cbType->blockSignals(false);
        cbEnableCal->blockSignals(false);
        calImportTerms->setEnabled(false);
        calImportMeas->setEnabled(false);
        calApplyToTraces->setEnabled(false);
        saveCal->setEnabled(false);
    });
    connect(calDisable, &QAction::triggered, this, &VNA::DisableCalibration);
    connect(this, &VNA::CalibrationApplied, [=](Calibration::Type applied){
        cbType->blockSignals(true);
        cbEnableCal->blockSignals(true);
        for(int i=0;i<cbType->count();i++) {
            if(cbType->itemData(i).toInt() == (int) applied) {
                cbType->setCurrentIndex(i);
                break;
            }
        }
        cbEnableCal->setCheckState(Qt::CheckState::Checked);
        // restore default look of widget
        // on hover, show name of active cal. file
        UpdateCalWidget();
        cbType->blockSignals(false);
        cbEnableCal->blockSignals(false);
        calImportTerms->setEnabled(true);
        calImportMeas->setEnabled(true);
        calApplyToTraces->setEnabled(true);
        saveCal->setEnabled(true);
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
    // initial setup is frequency sweep
    configureToolbarForFrequencySweep();
    SetSweepType(SweepType::Frequency);

    toolbars.insert(tb_cal);

//    auto tb_portExtension = portExtension.createToolbar();
//    window->addToolBar(tb_portExtension);
//    toolbars.insert(tb_portExtension);


    markerModel = new MarkerModel(traceModel, this);

    auto tracesDock = new QDockWidget("Traces");
    traceWidget = new TraceWidgetVNA(traceModel, cal, deembedding);
    tracesDock->setWidget(traceWidget);
    window->addDockWidget(Qt::LeftDockWidgetArea, tracesDock);
    docks.insert(tracesDock);


    auto markerWidget = new MarkerWidget(*markerModel);

    auto markerDock = new QDockWidget("Marker");
    markerDock->setWidget(markerWidget);
    window->addDockWidget(Qt::BottomDockWidgetArea, markerDock);
    docks.insert(markerDock);

    SetupSCPI();

    // Set initial sweep settings
    auto pref = Preferences::getInstance();

    if(pref.Startup.RememberSweepSettings) {
        LoadSweepSettings();
    } else {
        settings.Freq.start = pref.Startup.DefaultSweep.f_start;
        settings.Freq.stop = pref.Startup.DefaultSweep.f_stop;
        SetSourceLevel(pref.Startup.DefaultSweep.f_excitation);
        ConstrainAndUpdateFrequencies();
        SetStartPower(pref.Startup.DefaultSweep.dbm_start);
        SetStopPower(pref.Startup.DefaultSweep.dbm_stop);
        SetPowerSweepFrequency(pref.Startup.DefaultSweep.dbm_freq);
        SetIFBandwidth(pref.Startup.DefaultSweep.bandwidth);
        SetAveraging(pref.Startup.DefaultSweep.averaging);
        SetPoints(pref.Startup.DefaultSweep.points);
        if(pref.Startup.DefaultSweep.type == "Power Sweep") {
            SetSweepType(SweepType::Power);
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
    double f_min, f_max;
    switch(settings.sweepType) {
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
    Calibration::InterpolationType interpol = getCalInterpolation();
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
    Calibration::InterpolationType interpol = getCalInterpolation();
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
    Mode::deactivate();
}

void VNA::initializeDevice()
{
    defaultCalMenu->setEnabled(true);
    connect(window->getDevice(), &Device::DatapointReceived, this, &VNA::NewDatapoint, Qt::UniqueConnection);
    // Check if default calibration exists and attempt to load it
    QSettings s;
    auto key = "DefaultCalibration"+window->getDevice()->serial();
    if (s.contains(key)) {
        auto filename = s.value(key).toString();
        qDebug() << "Attempting to load default calibration file " << filename;
        if(QFile::exists(filename)) {
            if(cal.openFromFile(filename)) {
                ApplyCalibration(cal.getType());
//                portExtension.setCalkit(&cal.getCalibrationKit());
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
    SettingsChanged();
}

void VNA::deviceDisconnected()
{
    defaultCalMenu->setEnabled(false);
}

void VNA::shutdown()
{
    if(calEdited && calValid) {
        auto save = InformationBox::AskQuestion("Save calibration?", "The calibration contains data that has not been saved yet. Do you want to save it before exiting?", false);
        if(save) {
            cal.saveToFile();
        }
    }
}

nlohmann::json VNA::toJSON()
{
    nlohmann::json j;
    j["traces"] = traceModel.toJSON();
    j["tiles"] = central->toJSON();
    j["markers"] = markerModel->toJSON();
    j["de-embedding"] = deembedding.toJSON();
    j["de-embedding_enabled"] = deembedding_active;
    return j;
}

void VNA::fromJSON(nlohmann::json j)
{
    if(j.contains("traces")) {
        traceModel.fromJSON(j["traces"]);
    }
    if(j.contains("tiles")) {
        central->fromJSON(j["tiles"]);
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
}

using namespace std;

void VNA::NewDatapoint(Protocol::Datapoint d)
{
    d = average.process(d);
    if(calMeasuring) {
        if(average.currentSweep() == averages) {
            // this is the last averaging sweep, use values for calibration
            if(!calWaitFirst || d.pointNum == 0) {
                calWaitFirst = false;
                cal.addMeasurement(calMeasurement, d);
                if(d.pointNum == settings.npoints - 1) {
                    calMeasuring = false;
                    qDebug() << "Calibration measurement" << cal.MeasurementToString(calMeasurement) << "complete";
                    emit CalibrationMeasurementComplete(calMeasurement);
                }
            }
        }
        int percentage = (((average.currentSweep() - 1) * 100) + (d.pointNum + 1) * 100 / settings.npoints) / averages;
        calDialog.setValue(percentage);
    }
    if(calValid) {
        cal.correctMeasurement(d);
    }

    if(deembedding_active) {
        deembedding.Deembed(d);
    }

    TraceMath::DataType type;
    switch(settings.sweepType) {
    case SweepType::Frequency:
        type = TraceMath::DataType::Frequency;
        break;
    case SweepType::Power:
        type = TraceMath::DataType::Power;
        break;
    }

    traceModel.addVNAData(d, type);
    emit dataChanged();
    if(d.pointNum == settings.npoints - 1) {
        UpdateAverageCount();
        markerModel->updateMarkers();
    }
    static unsigned int lastPoint = 0;
    if(d.pointNum > 0 && d.pointNum != lastPoint + 1) {
        qWarning() << "Got point" << d.pointNum << "but last received point was" << lastPoint << "("<<(d.pointNum-lastPoint-1)<<"missed points)";
    }
    lastPoint = d.pointNum;
}

void VNA::UpdateAverageCount()
{
    lAverages->setText(QString::number(average.getLevel()) + "/");
}

void VNA::SettingsChanged(std::function<void (Device::TransmissionResult)> cb)
{
    // assemble VNA protocol settings
    Protocol::SweepSettings s;
    s.suppressPeaks = Preferences::getInstance().Acquisition.suppressPeaks ? 1 : 0;
    if(Preferences::getInstance().Acquisition.alwaysExciteBothPorts) {
        s.excitePort1 = 1;
        s.excitePort2 = 1;
    } else {
        s.excitePort1 = traceModel.PortExcitationRequired(1);
        s.excitePort2 = traceModel.PortExcitationRequired(2);
    }
    settings.excitingPort1 = s.excitePort1;
    settings.excitingPort2 = s.excitePort2;
    if(settings.sweepType == SweepType::Frequency) {
        s.fixedPowerSetting = Preferences::getInstance().Acquisition.adjustPowerLevel ? 0 : 1;
        s.f_start = settings.Freq.start;
        s.f_stop = settings.Freq.stop;
        s.points = settings.npoints;
        s.if_bandwidth = settings.bandwidth;
        s.cdbm_excitation_start = settings.Freq.excitation_power * 100;
        s.cdbm_excitation_stop = settings.Freq.excitation_power * 100;
    } else if(settings.sweepType == SweepType::Power) {
        s.fixedPowerSetting = 0;
        s.f_start = settings.Power.frequency;
        s.f_stop = settings.Power.frequency;
        s.points = settings.npoints;
        s.if_bandwidth = settings.bandwidth;
        s.cdbm_excitation_start = settings.Power.start * 100;
        s.cdbm_excitation_stop = settings.Power.stop * 100;
    }
    if(window->getDevice() && Mode::getActiveMode() == this) {
        window->getDevice()->Configure(s, [=](Device::TransmissionResult res){
            // device received command, reset traces now
            average.reset(s.points);
            traceModel.clearLiveData();
            UpdateAverageCount();
            UpdateCalWidget();
            if(cb) {
                cb(res);
            }
        });
    }
    emit traceModel.SpanChanged(s.f_start, s.f_stop);
}

void VNA::StartImpedanceMatching()
{
    auto dialog = new ImpedanceMatchDialog(*markerModel);
    dialog->show();
}


void VNA::SetSweepType(SweepType sw)
{
    if(settings.sweepType != sw) {
        settings.sweepType = sw;
        emit sweepTypeChanged(sw);
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
    if (freq - old_span / 2 <= Device::Info().limits_minFreq) {
        // would shift start frequency below minimum
        settings.Freq.start = 0;
        settings.Freq.stop = 2 * freq;
    } else if(freq + old_span / 2 >= Device::Info().limits_maxFreq) {
        // would shift stop frequency above maximum
        settings.Freq.start = 2 * freq - Device::Info().limits_maxFreq;
        settings.Freq.stop = Device::Info().limits_maxFreq;
    } else {
        settings.Freq.start = freq - old_span / 2;
        settings.Freq.stop = freq + old_span / 2;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetSpan(double span)
{
    auto maxFreq = Preferences::getInstance().Acquisition.harmonicMixing ? Device::Info().limits_maxFreqHarmonic : Device::Info().limits_maxFreq;
    auto old_center = (settings.Freq.start + settings.Freq.stop) / 2;
    if(old_center < Device::Info().limits_minFreq + span / 2) {
        // would shift start frequency below minimum
        settings.Freq.start = Device::Info().limits_minFreq;
        settings.Freq.stop = Device::Info().limits_minFreq + span;
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
    settings.Freq.start = Device::Info().limits_minFreq;
    settings.Freq.stop = Device::Info().limits_maxFreq;
    ConstrainAndUpdateFrequencies();
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

void VNA::SetSourceLevel(double level)
{
    if(level > Device::Info().limits_cdbm_max / 100.0) {
        level = Device::Info().limits_cdbm_max / 100.0;
    } else if(level < Device::Info().limits_cdbm_min / 100.0) {
        level = Device::Info().limits_cdbm_min / 100.0;
    }
    emit sourceLevelChanged(level);
    settings.Freq.excitation_power = level;
    SettingsChanged();
}

void VNA::SetStartPower(double level)
{
    settings.Power.start = level;
    emit startPowerChanged(level);
    SettingsChanged();
}

void VNA::SetStopPower(double level)
{
    settings.Power.stop = level;
    emit stopPowerChanged(level);
    SettingsChanged();
}

void VNA::SetPowerSweepFrequency(double freq)
{
    settings.Power.frequency = freq;
    emit powerSweepFrequencyChanged(freq);
    SettingsChanged();
}

void VNA::SetPoints(unsigned int points)
{
    if(points > Device::Info().limits_maxPoints) {
        points = Device::Info().limits_maxPoints;
    } else if (points < 2) {
        points = 2;
    }
    emit pointsChanged(points);
    settings.npoints = points;
    SettingsChanged();
}

void VNA::SetIFBandwidth(double bandwidth)
{
    if(bandwidth > Device::Info().limits_maxIFBW) {
        bandwidth = Device::Info().limits_maxIFBW;
    } else if(bandwidth < Device::Info().limits_minIFBW) {
        bandwidth = Device::Info().limits_minIFBW;
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
    SettingsChanged();
}

void VNA::ExcitationRequired(bool port1, bool port2)
{
    if(Preferences::getInstance().Acquisition.alwaysExciteBothPorts) {
        port1 = true;
        port2 = true;
    }
    // check if settings actually changed
    if(settings.excitingPort1 != port1
        || settings.excitingPort2 != port2) {
        settings.excitingPort1 = port1;
        settings.excitingPort2 = port2;
        SettingsChanged();
    }
}

void VNA::DisableCalibration(bool force)
{
    if(calValid || force) {
        calValid = false;
        cal.resetErrorTerms();
        emit CalibrationDisabled();
    }
}

void VNA::ApplyCalibration(Calibration::Type type)
{
    if(cal.calculationPossible(type)) {
        try {
            if(cal.constructErrorTerms(type)) {
                calValid = true;
                emit CalibrationApplied(type);
            } else {
                DisableCalibration(true);
            }
        } catch (runtime_error e) {
            QMessageBox::critical(window, "Calibration failure", e.what());
            DisableCalibration(true);
        }
    } else {
        if(settings.sweepType == SweepType::Frequency) {
            // Not all required traces available
            InformationBox::ShowMessageBlocking("Missing calibration measurements", "Not all calibration measurements for this type of calibration have been taken. The calibration can be enabled after the missing measurements have been acquired.");
            DisableCalibration(true);
            StartCalibrationDialog(type);
        } else {
            // Not all required traces available
            InformationBox::ShowMessageBlocking("Missing calibration measurements", "Not all calibration measurements for this type of calibration have been taken. Please switch to frequency sweep to take these measurements.");
            DisableCalibration(true);
        }
    }
}

void VNA::StartCalibrationMeasurement(Calibration::Measurement m)
{
    auto device = window->getDevice();
    if(!device) {
        return;
    }
    // Stop sweep
    StopSweep();
    qDebug() << "Taking" << Calibration::MeasurementToString(m) << "measurement";
    calMeasurement = m;
    // Delete any already captured data of this measurement
    cal.clearMeasurement(m);
    calWaitFirst = true;
    QString text = "Measuring \"";
    text.append(Calibration::MeasurementToString(m));
    text.append("\" parameters.");
    calDialog.setLabelText(text);
    calDialog.setCancelButtonText("Abort");
    calDialog.setWindowTitle("Taking calibration measurement...");
    calDialog.setValue(0);
    calDialog.setWindowModality(Qt::ApplicationModal);
    // always show the dialog
    calDialog.setMinimumDuration(0);
    connect(&calDialog, &QProgressDialog::canceled, [=]() {
        // the user aborted the calibration measurement
        calMeasuring = false;
        cal.clearMeasurement(calMeasurement);
    });
    // Trigger sweep to start from beginning
    SettingsChanged([=](Device::TransmissionResult){
        // enable calibration measurement only in transmission callback (prevents accidental sampling of data which was still being processed)
        calMeasuring = true;
    });
    calEdited = true;
}

void VNA::SetupSCPI()
{
    SCPINode::add(new SCPICommand("SWEEP", [=](QStringList params) -> QString {
        if(params.size() >= 1) {
            if(params[0] == "FREQUENCY") {
                SetSweepType(SweepType::Frequency);
                return "";
            } else if(params[0] == "POWER") {
                SetSweepType(SweepType::Power);
                return "";
            }
        }
        // either no parameter or invalid
        return "ERROR";
    }, [=](QStringList) -> QString {
        return settings.sweepType == SweepType::Frequency ? "FREQUENCY" : "POWER";
    }));
    auto scpi_freq = new SCPINode("FREQuency");
    SCPINode::add(scpi_freq);
    scpi_freq->add(new SCPICommand("SPAN", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetSpan(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.stop - settings.Freq.start);
    }));
    scpi_freq->add(new SCPICommand("START", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetStartFreq(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.start);
    }));
    scpi_freq->add(new SCPICommand("CENTer", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetCenterFreq(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number((settings.Freq.start + settings.Freq.stop)/2);
    }));
    scpi_freq->add(new SCPICommand("STOP", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetStopFreq(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.stop);
    }));
    scpi_freq->add(new SCPICommand("FULL", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        SetFullSpan();
        return "";
    }, nullptr));
    auto scpi_power = new SCPINode("POWer");
    SCPINode::add(scpi_power);
    scpi_power->add(new SCPICommand("START", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {
            return "ERROR";
        } else {
            SetStartPower(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Power.start);
    }));
    scpi_power->add(new SCPICommand("STOP", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {
            return "ERROR";
        } else {
            SetStopPower(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Power.stop);
    }));
    auto scpi_acq = new SCPINode("ACQuisition");
    SCPINode::add(scpi_acq);
    scpi_acq->add(new SCPICommand("IFBW", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetIFBandwidth(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.bandwidth);
    }));
    scpi_acq->add(new SCPICommand("POINTS", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetPoints(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.npoints);
    }));
    scpi_acq->add(new SCPICommand("AVG", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetAveraging(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(averages);
    }));
    scpi_acq->add(new SCPICommand("AVGLEVel", nullptr, [=](QStringList) -> QString {
        return QString::number(average.getLevel());
    }));
    scpi_acq->add(new SCPICommand("FINished", nullptr, [=](QStringList) -> QString {
        return average.getLevel() == averages ? "TRUE" : "FALSE";
    }));
    auto scpi_stim = new SCPINode("STIMulus");
    SCPINode::add(scpi_stim);
    scpi_stim->add(new SCPICommand("LVL", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {
            return "ERROR";
        } else {
            SetSourceLevel(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Freq.excitation_power);
    }));
    scpi_stim->add(new SCPICommand("FREQuency", [=](QStringList params) -> QString {
        unsigned long newval;
        if(!SCPI::paramToULong(params, 0, newval)) {
            return "ERROR";
        } else {
            SetPowerSweepFrequency(newval);
            return "";
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.Power.frequency);
    }));
    SCPINode::add(traceWidget);
    auto scpi_cal = new SCPINode("CALibration");
    SCPINode::add(scpi_cal);
    scpi_cal->add(new SCPICommand("TYPE", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return "ERROR";
        } else {
            auto type = Calibration::TypeFromString(params[0].replace('_', ' '));
            if(type == Calibration::Type::Last) {
                // failed to parse string
                return "ERROR";
            } else if(type == Calibration::Type::None) {
                DisableCalibration();
            } else {
                // check if calibration can be activated
                if(cal.calculationPossible(type)) {
                    ApplyCalibration(type);
                } else {
                    return "ERROR";
                }
            }
        }
        return "";
    }, [=](QStringList) -> QString {
        auto ret = Calibration::TypeToString(cal.getType());
        ret.replace(' ', '_');
        return ret;
    }));
    scpi_cal->add(new SCPICommand("MEASure", [=](QStringList params) -> QString {
        if(params.size() != 1 || CalibrationMeasurementActive() || !window->getDevice() || Mode::getActiveMode() != this) {
            // no measurement specified, still busy or invalid mode
            return "ERROR";
        } else {
            auto meas = Calibration::MeasurementFromString(params[0].replace('_', ' '));
            if(meas == Calibration::Measurement::Last) {
                // failed to parse string
                return "ERROR";
            } else {
                StartCalibrationMeasurement(meas);
            }
        }
        return "";
    }, nullptr));
    scpi_cal->add(new SCPICommand("BUSy", nullptr, [=](QStringList) -> QString {
        return CalibrationMeasurementActive() ? "TRUE" : "FALSE";
    }));
}

void VNA::ConstrainAndUpdateFrequencies()
{
    auto pref = Preferences::getInstance();
    double maxFreq;
    if(pref.Acquisition.harmonicMixing) {
        maxFreq = Device::Info().limits_maxFreqHarmonic;
    } else {
        maxFreq = Device::Info().limits_maxFreq;
    }
    if(settings.Freq.stop > maxFreq) {
        settings.Freq.stop = maxFreq;
    }
    if(settings.Freq.start > settings.Freq.stop) {
        settings.Freq.start = settings.Freq.stop;
    }
    if(settings.Freq.start < Device::Info().limits_minFreq) {
        settings.Freq.start = Device::Info().limits_minFreq;
    }
    emit startFreqChanged(settings.Freq.start);
    emit stopFreqChanged(settings.Freq.stop);
    emit spanChanged(settings.Freq.stop - settings.Freq.start);
    emit centerFreqChanged((settings.Freq.stop + settings.Freq.start)/2);
    SettingsChanged();
}

void VNA::LoadSweepSettings()
{
    auto pref = Preferences::getInstance();
    QSettings s;
    // frequency sweep settings
    settings.Freq.start = s.value("SweepFreqStart", pref.Startup.DefaultSweep.f_start).toULongLong();
    settings.Freq.stop = s.value("SweepFreqStop", pref.Startup.DefaultSweep.f_stop).toULongLong();
    SetSourceLevel(s.value("SweepFreqLevel", pref.Startup.DefaultSweep.f_excitation).toDouble());
    // power sweep settings
    SetStartPower(s.value("SweepPowerStart", pref.Startup.DefaultSweep.dbm_start).toDouble());
    SetStopPower(s.value("SweepPowerStop", pref.Startup.DefaultSweep.dbm_stop).toDouble());
    SetPowerSweepFrequency(s.value("SweepPowerFreq", pref.Startup.DefaultSweep.dbm_freq).toULongLong());
    SetPoints(s.value("SweepPoints", pref.Startup.DefaultSweep.points).toInt());
    SetIFBandwidth(s.value("SweepBandwidth", pref.Startup.DefaultSweep.bandwidth).toUInt());
    SetAveraging(s.value("SweepAveraging", pref.Startup.DefaultSweep.averaging).toInt());
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
    s.setValue("SweepPowerStart", settings.Power.start);
    s.setValue("SweepPowerStop", settings.Power.stop);
    s.setValue("SweepPowerFreq", static_cast<unsigned long long>(settings.Power.frequency));
    s.setValue("SweepBandwidth", settings.bandwidth);
    s.setValue("SweepPoints", settings.npoints);
    s.setValue("SweepAveraging", averages);
}

void VNA::StopSweep()
{
    if(window->getDevice()) {
        window->getDevice()->SetIdle();
    }
}

void VNA::StartCalibrationDialog(Calibration::Type type)
{
    auto traceDialog = new CalibrationTraceDialog(&cal, settings.Freq.start, settings.Freq.stop, type);
    connect(traceDialog, &CalibrationTraceDialog::triggerMeasurement, this, &VNA::StartCalibrationMeasurement);
    connect(traceDialog, &CalibrationTraceDialog::applyCalibration, this, &VNA::ApplyCalibration);
    connect(this, &VNA::CalibrationMeasurementComplete, traceDialog, &CalibrationTraceDialog::measurementComplete);
    connect(traceDialog, &CalibrationTraceDialog::calibrationInvalidated, [=](){
       DisableCalibration(true);
       InformationBox::ShowMessageBlocking("Calibration disabled", "The currently active calibration is no longer supported by the available measurements and was disabled.");
    });
    traceDialog->show();
}

void VNA::UpdateCalWidget()
{
    calLabel->setStyleSheet(getCalStyle());
    calLabel->setToolTip(getCalToolTip());
}

void VNA::EnableDeembedding(bool enable)
{
    deembedding_active = enable;
    enableDeembeddingAction->blockSignals(true);
    enableDeembeddingAction->setChecked(enable);
    enableDeembeddingAction->blockSignals(false);
}

void VNA::updateGraphColors()
{
    emit graphColorsChanged();
}
