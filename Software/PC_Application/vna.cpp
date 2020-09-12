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
#include "valueinput.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include <algorithm>
#include "Menu/menu.h"
#include "Menu/menuaction.h"
#include "Menu/menuvalue.h"
#include "Menu/menubool.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <iostream>
#include <fstream>
#include <QDateTime>
#include "unit.h"
#include "CustomWidgets/toggleswitch.h"
#include "Device/manualcontroldialog.h"
#include "Traces/tracemodel.h"
#include "Traces/tracewidget.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracebodeplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include <QDockWidget>
#include "Traces/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "Calibration/calibrationtracedialog.h"
#include "ui_main.h"
#include "Device/firmwareupdatedialog.h"
#include "preferences.h"
#include "signalgenerator.h"
#include <QDesktopWidget>
#include <QApplication>

using namespace std;

constexpr Protocol::SweepSettings VNA::defaultSweep;

VNA::VNA(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("VNA");
    QCoreApplication::setApplicationName("Application");

    pref.load();

    averages = 1;
    calValid = false;
    calMeasuring = false;
    device = nullptr;
    calDialog.reset();

    ui->setupUi(this);
//    ui->statusbar->insertPermanentWidget(0, &lDeviceStatus);
//    ui->statusbar->insertPermanentWidget(1, new QPushButton("Test"));
    ui->statusbar->addWidget(&lConnectionStatus);
    auto div1 = new QFrame;
    div1->setFrameShape(QFrame::VLine);
    ui->statusbar->addWidget(div1);
    ui->statusbar->addWidget(&lDeviceInfo);
    ui->statusbar->addWidget(new QLabel, 1);
    //ui->statusbar->setStyleSheet("QStatusBar::item { border: 1px solid black; };");

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

    auto tracebode1 = new TraceBodePlot(traceModel);
    tracebode1->enableTrace(tS12, true);
    auto tracebode2 = new TraceBodePlot(traceModel);
    tracebode2->enableTrace(tS21, true);

    auto tiles = new TileWidget(traceModel);
    tiles->splitVertically();
    tiles->Child1()->splitHorizontally();
    tiles->Child2()->splitHorizontally();
    tiles->Child1()->Child1()->setPlot(tracesmith1);
    tiles->Child1()->Child2()->setPlot(tracebode1);
    tiles->Child2()->Child1()->setPlot(tracebode2);
    tiles->Child2()->Child2()->setPlot(tracesmith2);

    // Create GUI modes
    central = new QStackedWidget;
    setCentralWidget(central);
    modeVNA = new GUIMode(this, "Vector Network Analyzer", tiles);
    auto signalGenWidget = new Signalgenerator;
    modeSGen = new GUIMode(this, "Signal Generator", signalGenWidget);
    modeSGen->addHiddenElement(ui->menuTools->menuAction());
    modeSGen->addHiddenElement(ui->menuCalibration->menuAction());

    CreateToolbars();
    // UI connections
    connect(ui->actionUpdate_Device_List, &QAction::triggered, this, &VNA::UpdateDeviceList);
    connect(ui->actionDisconnect, &QAction::triggered, this, &VNA::DisconnectDevice);
    connect(ui->actionQuit, &QAction::triggered, this, &VNA::close);
    connect(ui->actionManual_Control, &QAction::triggered, this, &VNA::StartManualControl);
    connect(ui->actionImpedance_Matching, &QAction::triggered, this, &VNA::StartImpedanceMatching);
    connect(ui->actionEdit_Calibration_Kit, &QAction::triggered, [=](){
        cal.getCalibrationKit().edit();
    });
    connect(ui->actionImport_error_terms_as_traces, &QAction::triggered, [=](){
        auto import = new TraceImportDialog(traceModel, cal.getErrorTermTraces());
        import->show();
    });
    connect(ui->actionTracedata, &QAction::triggered, [=](){
       auto dialog = new CalibrationTraceDialog(&cal);
       connect(dialog, &CalibrationTraceDialog::triggerMeasurement, this, &VNA::StartCalibrationMeasurement);
       connect(dialog, &CalibrationTraceDialog::applyCalibration, this, &VNA::ApplyCalibration);
       connect(this, &VNA::CalibrationMeasurementComplete, dialog, &CalibrationTraceDialog::measurementComplete);
       dialog->show();
    });
    connect(ui->actionAssignDefaultCal, &QAction::triggered, [=](){
       if(device) {
           auto key = "DefaultCalibration"+device->serial();
           QSettings settings;
           auto filename = QFileDialog::getOpenFileName(nullptr, "Load calibration data", settings.value(key).toString(), "Calibration files (*.cal)", nullptr, QFileDialog::DontUseNativeDialog);
           if(!filename.isEmpty()) {
               settings.setValue(key, filename);
               ui->actionRemoveDefaultCal->setEnabled(true);
           }
       }
    });
    connect(ui->actionRemoveDefaultCal, &QAction::triggered, [=](){
        QSettings settings;
        settings.remove("DefaultCalibration"+device->serial());
        ui->actionRemoveDefaultCal->setEnabled(false);
    });
    connect(ui->actionFirmware_Update, &QAction::triggered, [=](){
        if(device) {
            auto fw_update = new FirmwareUpdateDialog(device);
            fw_update->exec();
        }
    });
    connect(ui->actionPreferences, &QAction::triggered, [=](){
       pref.edit();
    });


    setWindowTitle("VNA");

    markerModel = new TraceMarkerModel(traceModel);

    // Create status panel
    auto statusLayout = new QVBoxLayout();
    statusLayout->setSpacing(0);
    QFont statusFont( "Arial", 8);
    {
        auto l = new QLabel("Start Frequency:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lStart.setAlignment(Qt::AlignRight);
        lStart.setFont(statusFont);
        statusLayout->addWidget(&lStart);

        l = new QLabel("Center Frequency:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lCenter.setAlignment(Qt::AlignRight);
        lCenter.setFont(statusFont);
        statusLayout->addWidget(&lCenter);

        l = new QLabel("Stop Frequency:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lStop.setAlignment(Qt::AlignRight);
        lStop.setFont(statusFont);
        statusLayout->addWidget(&lStop);

        l = new QLabel("Span:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lSpan.setAlignment(Qt::AlignRight);
        lSpan.setFont(statusFont);
        statusLayout->addWidget(&lSpan);

        statusLayout->addStretch();

        l = new QLabel("Points:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lPoints.setAlignment(Qt::AlignRight);
        lPoints.setFont(statusFont);
        statusLayout->addWidget(&lPoints);

        l = new QLabel("IF Bandwidth:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lBandwidth.setAlignment(Qt::AlignRight);
        lBandwidth.setFont(statusFont);
        statusLayout->addWidget(&lBandwidth);

        l = new QLabel("Averages:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lAverages.setAlignment(Qt::AlignRight);
        lAverages.setFont(statusFont);
        statusLayout->addWidget(&lAverages);

        statusLayout->addStretch();

        l = new QLabel("Calibration:");
        l->setAlignment(Qt::AlignLeft);
        l->setFont(statusFont);
        statusLayout->addWidget(l);
        lCalibration.setAlignment(Qt::AlignRight);
        lCalibration.setFont(statusFont);
        statusLayout->addWidget(&lCalibration);
    }
    statusLayout->addStretch();

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    auto statusWidget = new QWidget;
    statusWidget->setLayout(statusLayout);
//    statusWidget->setFixedWidth(150);
    auto statusDock = new QDockWidget("Status");
    statusDock->setWidget(statusWidget);
    addDockWidget(Qt::LeftDockWidgetArea, statusDock);

    auto tracesDock = new QDockWidget("Traces");
    tracesDock->setWidget(new TraceWidget(traceModel));
    addDockWidget(Qt::LeftDockWidgetArea, tracesDock);


    auto markerWidget = new MarkerWidget(*markerModel);

    auto markerDock = new QDockWidget("Marker");
    markerDock->setWidget(markerWidget);
    addDockWidget(Qt::BottomDockWidgetArea, markerDock);

    auto logDock = new QDockWidget("Device Log");
    logDock->setWidget(&deviceLog);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);

    modeSGen->addHiddenElement(markerDock);
    modeSGen->addHiddenElement(tracesDock);
    modeSGen->addHiddenElement(statusDock);

    // status and menu dock hidden by default
    statusDock->close();

    {
        QSettings settings;
        restoreGeometry(settings.value("geometry").toByteArray());
    }

    // Set ObjectName for toolbars and docks
    for(auto d : findChildren<QDockWidget*>()) {
        d->setObjectName(d->windowTitle());
    }
    for(auto t : findChildren<QToolBar*>()) {
        t->setObjectName(t->windowTitle());
    }

    // Set default mode
    modeVNA->activate();

    qRegisterMetaType<Protocol::Datapoint>("Datapoint");


    // Set initial sweep settings
    if(pref.Startup.RememberSweepSettings) {
        LoadSweepSettings();
    } else {
        settings.f_start = pref.Startup.DefaultSweep.start;
        settings.f_stop = pref.Startup.DefaultSweep.stop;
        ConstrainAndUpdateFrequencies();
        SetSourceLevel(pref.Startup.DefaultSweep.excitation);
        SetIFBandwidth(pref.Startup.DefaultSweep.bandwidth);
        SetPoints(pref.Startup.DefaultSweep.points);
    }

    // List available devices
    if(UpdateDeviceList() && pref.Startup.ConnectToFirstDevice) {
        // at least one device available
        ConnectToDevice();
    }
}

void VNA::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    // deactivate currently used mode (stores mode state in settings)
    GUIMode::getActiveMode()->deactivate();
    StoreSweepSettings();
    pref.store();
    QMainWindow::closeEvent(event);
}

void VNA::NewDatapoint(Protocol::Datapoint d)
{
    if(calMeasuring) {
        if(!calWaitFirst || d.pointNum == 0) {
            calWaitFirst = false;
            cal.addMeasurement(calMeasurement, d);
            if(d.pointNum == settings.points - 1) {
                calMeasuring = false;
                emit CalibrationMeasurementComplete(calMeasurement);
            }
            calDialog.setValue(d.pointNum + 1);
        }
    }
    if(calValid) {
        cal.correctMeasurement(d);
    }
    d = average.process(d);
    traceModel.addVNAData(d);
    emit dataChanged();
    if(d.pointNum == settings.points - 1) {
        UpdateStatusPanel();
    }
}

void VNA::UpdateStatusPanel()
{
    lStart.setText(Unit::ToString(settings.f_start, "Hz", " kMG", 4));
    lCenter.setText(Unit::ToString((settings.f_start + settings.f_stop)/2, "Hz", " kMG", 4));
    lStop.setText(Unit::ToString(settings.f_stop, "Hz", " kMG", 4));
    lSpan.setText(Unit::ToString(settings.f_stop - settings.f_start, "Hz", " kMG", 4));
    lPoints.setText(QString::number(settings.points));
    lBandwidth.setText(Unit::ToString(settings.if_bandwidth, "Hz", " k", 2));
    lAverages.setText(QString::number(average.getLevel()) + "/" + QString::number(averages));
    if(calValid) {
        switch(cal.getInterpolation(settings)) {
        case Calibration::InterpolationType::Extrapolate:
            lCalibration.setText("Enabled/Extrapolating");
            break;
        case Calibration::InterpolationType::Interpolate:
            lCalibration.setText("Enabled/Interpolating");
            break;
        case Calibration::InterpolationType::Exact:
        case Calibration::InterpolationType::Unchanged:
            lCalibration.setText("Enabled");
            break;
        default:
            lCalibration.setText("Unknown");
            break;
        }
    } else {
        lCalibration.setText("Off");
    }
}

void VNA::SettingsChanged()
{
    if(device) {
        device->Configure(settings);
    }
    average.reset();
    traceModel.clearVNAData();
    UpdateStatusPanel();
    TracePlot::UpdateSpan(settings.f_start, settings.f_stop);
}

void VNA::ConnectToDevice(QString serial)
{
    if(device) {
        DisconnectDevice();
    }
    try {
        qDebug() << "Attempting to connect to device...";
        device = new Device(serial);
        lConnectionStatus.setText("Connected to " + device->serial());
        qInfo() << "Connected to " << device->serial();
        lDeviceInfo.setText(device->getLastDeviceInfoString());
        connect(device, &Device::DatapointReceived, this, &VNA::NewDatapoint);
        connect(device, &Device::LogLineReceived, &deviceLog, &DeviceLog::addLine);
        connect(device, &Device::ConnectionLost, this, &VNA::DeviceConnectionLost);
        connect(device, &Device::DeviceInfoUpdated, [this]() {
           lDeviceInfo.setText(device->getLastDeviceInfoString());
        });
        ui->actionDisconnect->setEnabled(true);
        ui->actionManual_Control->setEnabled(true);
        ui->menuDefault_Calibration->setEnabled(true);
        ui->actionFirmware_Update->setEnabled(true);
        {
            // Check if default calibration exists and attempt to load it
            QSettings settings;
            auto key = "DefaultCalibration"+device->serial();
            if (settings.contains(key)) {
                auto filename = settings.value(key).toString();
                qDebug() << "Attempting to load default calibration file \"" << filename << "\"";
                if(QFile::exists(filename)) {
                    cal.openFromFile(filename);
                    ApplyCalibration(cal.getType());
                }
                ui->actionRemoveDefaultCal->setEnabled(true);
            } else {
                qDebug() << "No default calibration file set for this device";
                ui->actionRemoveDefaultCal->setEnabled(false);
            }
        }
        // Configure initial state of device
        device->Configure(settings);
        UpdateReference();
    } catch (const runtime_error e) {
        DisconnectDevice();
        UpdateDeviceList();
    }
}

void VNA::DisconnectDevice()
{
    if(device) {
        delete device;
        device = nullptr;
    }
    ui->actionDisconnect->setEnabled(false);
    ui->actionManual_Control->setEnabled(false);
    ui->menuDefault_Calibration->setEnabled(false);
    ui->actionFirmware_Update->setEnabled(false);
    if(deviceActionGroup->checkedAction()) {
        deviceActionGroup->checkedAction()->setChecked(false);
    }
    lConnectionStatus.setText("No device connected");
    lDeviceInfo.setText("No device information available yet");
}

void VNA::DeviceConnectionLost()
{
    DisconnectDevice();
    QMessageBox::warning(this, "Disconnected", "The USB connection to the device has been lost");
    UpdateDeviceList();
}

void VNA::CreateToolbars()
{
    // Sweep toolbar
    auto tb_sweep = new QToolBar("Sweep", this);
    auto eStart = new SIUnitEdit("Hz", " kMG", 6);
    eStart->setFixedWidth(100);
    eStart->setToolTip("Start frequency");
    connect(eStart, &SIUnitEdit::valueChanged, this, &VNA::SetStartFreq);
    connect(this, &VNA::startFreqChanged, eStart, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Start:"));
    tb_sweep->addWidget(eStart);

    auto eCenter = new SIUnitEdit("Hz", " kMG", 6);
    eCenter->setFixedWidth(100);
    eCenter->setToolTip("Center frequency");
    connect(eCenter, &SIUnitEdit::valueChanged, this, &VNA::SetCenterFreq);
    connect(this, &VNA::centerFreqChanged, eCenter, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Center:"));
    tb_sweep->addWidget(eCenter);

    auto eStop = new SIUnitEdit("Hz", " kMG", 6);
    eStop->setFixedWidth(100);
    eStop->setToolTip("Stop frequency");
    connect(eStop, &SIUnitEdit::valueChanged, this, &VNA::SetStopFreq);
    connect(this, &VNA::stopFreqChanged, eStop, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Stop:"));
    tb_sweep->addWidget(eStop);

    auto eSpan = new SIUnitEdit("Hz", " kMG", 6);
    eSpan->setFixedWidth(100);
    eSpan->setToolTip("Span");
    connect(eSpan, &SIUnitEdit::valueChanged, this, &VNA::SetSpan);
    connect(this, &VNA::spanChanged, eSpan, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Span:"));
    tb_sweep->addWidget(eSpan);

    auto bFull = new QPushButton(QIcon::fromTheme("zoom-fit-best"), "");
    bFull->setToolTip("Full span");
    connect(bFull, &QPushButton::clicked, this, &VNA::SetFullSpan);
    tb_sweep->addWidget(bFull);

    auto bZoomIn = new QPushButton(QIcon::fromTheme("zoom-in"), "");
    bZoomIn->setToolTip("Zoom in");
    connect(bZoomIn, &QPushButton::clicked, this, &VNA::SpanZoomIn);
    tb_sweep->addWidget(bZoomIn);

    auto bZoomOut = new QPushButton(QIcon::fromTheme("zoom-out"), "");
    bZoomOut->setToolTip("Zoom out");
    connect(bZoomOut, &QPushButton::clicked, this, &VNA::SpanZoomOut);
    tb_sweep->addWidget(bZoomOut);

    addToolBar(tb_sweep);
    modeSGen->addHiddenElement(tb_sweep);

    // Acquisition toolbar
    auto tb_acq = new QToolBar("Acquisition", this);
    auto dbm = new QDoubleSpinBox();
    dbm->setValue(settings.cdbm_excitation * 100);
    dbm->setFixedWidth(95);
    dbm->setRange(-42.0, -10.0);
    dbm->setSingleStep(0.25);
    dbm->setSuffix("dbm");
    dbm->setToolTip("Stimulus level");
    connect(dbm, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &VNA::SetSourceLevel);
    connect(this, &VNA::sourceLevelChanged, dbm, &QDoubleSpinBox::setValue);
    tb_acq->addWidget(new QLabel("Level:"));
    tb_acq->addWidget(dbm);

    auto points = new QSpinBox();
    points->setFixedWidth(55);
    points->setRange(1, 4501);
    points->setValue(settings.points);
    points->setSingleStep(100);
    points->setToolTip("Points/sweep");
    connect(points, qOverload<int>(&QSpinBox::valueChanged), this, &VNA::SetPoints);
    connect(this, &VNA::pointsChanged, points, &QSpinBox::setValue);
    tb_acq->addWidget(new QLabel("Points:"));
    tb_acq->addWidget(points);

    auto eBandwidth = new SIUnitEdit("Hz", " k", 3);
    eBandwidth->setValueQuiet(settings.if_bandwidth);
    eBandwidth->setFixedWidth(70);
    eBandwidth->setToolTip("IF bandwidth");
    connect(eBandwidth, &SIUnitEdit::valueChanged, this, &VNA::SetIFBandwidth);
    connect(this, &VNA::IFBandwidthChanged, eBandwidth, &SIUnitEdit::setValueQuiet);
    tb_acq->addWidget(new QLabel("IF BW:"));
    tb_acq->addWidget(eBandwidth);

    addToolBar(tb_acq);
    modeSGen->addHiddenElement(tb_acq);

    // Reference toolbar
    auto tb_reference = new QToolBar("Reference", this);
    tb_reference->addWidget(new QLabel("Ref:"));
    toolbars.reference.type = new QComboBox();
    toolbars.reference.type->addItem("Int");
    toolbars.reference.type->addItem("Ext");
    toolbars.reference.automatic = new QCheckBox("Auto");
    connect(toolbars.reference.automatic, &QCheckBox::clicked, [this](bool checked) {
        toolbars.reference.type->setEnabled(!checked);
        UpdateReference();
    });
    //    toolbars.reference.automatic->setChecked(true);
    tb_reference->addWidget(toolbars.reference.type);
    tb_reference->addWidget(toolbars.reference.automatic);
    tb_reference->addSeparator();
    tb_reference->addWidget(new QLabel("Ref out:"));
    toolbars.reference.outputEnabled = new QCheckBox();
    toolbars.reference.outFreq = new QComboBox();
    toolbars.reference.outFreq->addItem("10 MHz");
    toolbars.reference.outFreq->addItem("100 MHz");
    tb_reference->addWidget(toolbars.reference.outputEnabled);
    tb_reference->addWidget(toolbars.reference.outFreq);
    connect(toolbars.reference.type, qOverload<int>(&QComboBox::currentIndexChanged), this, &VNA::UpdateReference);
    connect(toolbars.reference.outFreq, qOverload<int>(&QComboBox::currentIndexChanged), this, &VNA::UpdateReference);
    connect(toolbars.reference.outputEnabled, &QCheckBox::clicked, this, &VNA::UpdateReference);

    addToolBar(tb_reference);

    // Calibration toolbar (and populate calibration menu)
    auto tb_cal = new QToolBar("Calibration");
    tb_cal->addWidget(new QLabel("Calibration:"));
    auto cbEnableCal = new QCheckBox;
    tb_cal->addWidget(cbEnableCal);
    auto cbType = new QComboBox();
    auto calMenuGroup = new QActionGroup(this);
    calMenuGroup->addAction(ui->actionCalDisabled);
    for(auto type : Calibration::Types()) {
        cbType->addItem(Calibration::TypeToString(type), (int) type);
        auto menuAction = new QAction(Calibration::TypeToString(type));
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
        ui->menuCalibration->insertAction(ui->actionCalDisabled, menuAction);
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
        ui->actionCalDisabled->setChecked(true);
        cbEnableCal->setCheckState(Qt::CheckState::Unchecked);
        cbType->blockSignals(false);
        cbEnableCal->blockSignals(false);
    });
    connect(ui->actionCalDisabled, &QAction::triggered, this, &VNA::DisableCalibration);
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
        cbType->blockSignals(false);
        cbEnableCal->blockSignals(false);
    });

    tb_cal->addWidget(cbType);

    addToolBar(tb_cal);
    modeSGen->addHiddenElement(tb_cal);
}

int VNA::UpdateDeviceList()
{
    ui->menuConnect_to->clear();
    auto devices = Device::GetDevices();
    if(devices.size()) {
        for(auto d : devices) {
            auto connectAction = ui->menuConnect_to->addAction(d);
            deviceActionGroup->addAction(connectAction);
            connectAction->setCheckable(true);
            if(device && d == device->serial()) {
                connectAction->setChecked(true);
            }
            connect(connectAction, &QAction::triggered, [this, connectAction, d]() {
               ConnectToDevice(d);
               if(device) {
                   // connectAction might have been unchecked if it was a reconnect to the already connected device
                   connectAction->setChecked(true);
               }
            });
        }
        ui->menuConnect_to->setEnabled(true);
    } else {
        // no devices available, disable connection option
        ui->menuConnect_to->setEnabled(false);
    }
    return devices.size();
}

void VNA::StartManualControl()
{
    auto control = new ManualControlDialog(*device, this);
    connect(control, &QDialog::finished, [this](){
        SettingsChanged();
    });
    control->show();
}

void VNA::StartImpedanceMatching()
{
    auto dialog = new ImpedanceMatchDialog(*markerModel);
    dialog->show();
}

void VNA::SetStartFreq(double freq)
{
    settings.f_start = freq;
    if(settings.f_stop < freq) {
        settings.f_stop = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetStopFreq(double freq)
{
    settings.f_stop = freq;
    if(settings.f_start > freq) {
        settings.f_start = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetCenterFreq(double freq)
{
    auto old_span = settings.f_stop - settings.f_start;
    if (freq > old_span / 2) {
        settings.f_start = freq - old_span / 2;
        settings.f_stop = freq + old_span / 2;
    } else {
        settings.f_start = 0;
        settings.f_stop = 2 * freq;
    }
    ConstrainAndUpdateFrequencies();
}

void VNA::SetSpan(double span)
{
    auto old_center = (settings.f_start + settings.f_stop) / 2;
    if(old_center > span / 2) {
        settings.f_start = old_center - span / 2;
    } else {
        settings.f_start = 0;
    }
    settings.f_stop = old_center + span / 2;
    ConstrainAndUpdateFrequencies();
}

void VNA::SetFullSpan()
{
    settings.f_start = 0;
    settings.f_stop = maxFreq;
    ConstrainAndUpdateFrequencies();
}

void VNA::SpanZoomIn()
{
    auto center = (settings.f_start + settings.f_stop) / 2;
    auto old_span = settings.f_stop - settings.f_start;
    settings.f_start = center - old_span / 4;
    settings.f_stop = center + old_span / 4;
    ConstrainAndUpdateFrequencies();
}

void VNA::SpanZoomOut()
{
    auto center = (settings.f_start + settings.f_stop) / 2;
    auto old_span = settings.f_stop - settings.f_start;
    if(center > old_span) {
        settings.f_start = center - old_span;
    } else {
        settings.f_start = 0;
    }
    settings.f_stop = center + old_span;
    ConstrainAndUpdateFrequencies();
}

void VNA::SetSourceLevel(double level)
{
    // TODO remove hardcoded limits
    if(level > -10.0) {
        level = -10.0;
    } else if(level < -42.0) {
        level = -42.0;
    }
    emit sourceLevelChanged(level);
    settings.cdbm_excitation = level * 100;
    SettingsChanged();
}

void VNA::SetPoints(unsigned int points)
{
    // TODO remove hardcoded limits
    if (points < 1) {
        points = 1;
    } else if(points > 4501) {
        points = 4501;
    }
    emit pointsChanged(points);
    settings.points = points;
    SettingsChanged();
}

void VNA::SetIFBandwidth(double bandwidth)
{
    settings.if_bandwidth = bandwidth;
    emit IFBandwidthChanged(bandwidth);
    SettingsChanged();
}

void VNA::SetAveraging(unsigned int averages)
{
    this->averages = averages;
    average.setAverages(averages);
    emit averagingChanged(averages);
    SettingsChanged();
}

void VNA::DisableCalibration(bool force)
{
    if(calValid || force) {
        calValid = false;
        ui->actionImport_error_terms_as_traces->setEnabled(false);
        emit CalibrationDisabled();
        average.reset();
    }
}

void VNA::ApplyCalibration(Calibration::Type type)
{
    if(cal.calculationPossible(type)) {
        try {
            cal.constructErrorTerms(type);
            calValid = true;
            average.reset();
            ui->actionImport_error_terms_as_traces->setEnabled(true);
            emit CalibrationApplied(type);
        } catch (runtime_error e) {
            QMessageBox::critical(this, "Calibration failure", e.what());
            DisableCalibration(true);
        }
    } else {
        // Not all required traces available
        // TODO start tracedata dialog with required traces
        QMessageBox::information(this, "Missing calibration traces", "Not all calibration traces for this type of calibration have been measured. The calibration can be enabled after the missing traces have been acquired.");
        DisableCalibration(true);
        auto traceDialog = new CalibrationTraceDialog(&cal, type);
        connect(traceDialog, &CalibrationTraceDialog::triggerMeasurement, this, &VNA::StartCalibrationMeasurement);
        connect(traceDialog, &CalibrationTraceDialog::applyCalibration, this, &VNA::ApplyCalibration);
        connect(this, &VNA::CalibrationMeasurementComplete, traceDialog, &CalibrationTraceDialog::measurementComplete);
        traceDialog->show();
    }
}

void VNA::StartCalibrationMeasurement(Calibration::Measurement m)
{
    // Trigger sweep to start from beginning
    SettingsChanged();
    calMeasurement = m;
    // Delete any already captured data of this measurement
    cal.clearMeasurement(m);
    calWaitFirst = true;
    calMeasuring = true;
    QString text = "Measuring \"";
    text.append(Calibration::MeasurementToString(m));
    text.append("\" parameters.");
    calDialog.setRange(0, settings.points);
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
}

void VNA::UpdateReference()
{
    if(!device) {
        // can't update without a device connected
        return;
    }
    Protocol::ReferenceSettings s = {};
    if(toolbars.reference.automatic->isChecked()) {
        s.AutomaticSwitch = 1;
    }
    if(toolbars.reference.type->currentText()=="Ext") {
        s.UseExternalRef = 1;
    }
    if(toolbars.reference.outputEnabled->isChecked()) {
        switch(toolbars.reference.outFreq->currentIndex()) {
        case 0:
            s.ExtRefOuputFreq = 10000000;
            break;
        case 1:
            s.ExtRefOuputFreq = 100000000;
            break;
        }
    }
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::Reference;
    p.reference = s;
    device->SendPacket(p);
}

void VNA::ConstrainAndUpdateFrequencies()
{
    if(settings.f_stop > maxFreq) {
        settings.f_stop = maxFreq;
    }
    if(settings.f_start > settings.f_stop) {
        settings.f_start = settings.f_stop;
    }
    emit startFreqChanged(settings.f_start);
    emit stopFreqChanged(settings.f_stop);
    emit spanChanged(settings.f_stop - settings.f_start);
    emit centerFreqChanged((settings.f_stop + settings.f_start)/2);
    SettingsChanged();
}

void VNA::LoadSweepSettings()
{
    QSettings s;
    settings.f_start = s.value("SweepStart", pref.Startup.DefaultSweep.start).toULongLong();
    settings.f_stop = s.value("SweepStop", pref.Startup.DefaultSweep.stop).toULongLong();
    ConstrainAndUpdateFrequencies();
    SetIFBandwidth(s.value("SweepBandwidth", pref.Startup.DefaultSweep.bandwidth).toUInt());
    SetPoints(s.value("SweepPoints", pref.Startup.DefaultSweep.points).toInt());
    SetSourceLevel(s.value("SweepLevel", pref.Startup.DefaultSweep.excitation).toDouble());
}

void VNA::StoreSweepSettings()
{
    QSettings s;
    s.setValue("SweepStart", static_cast<unsigned long long>(settings.f_start));
    s.setValue("SweepStop", static_cast<unsigned long long>(settings.f_stop));
    s.setValue("SweepBandwidth", settings.if_bandwidth);
    s.setValue("SweepPoints", settings.points);
    s.setValue("SweepLevel", (double) settings.cdbm_excitation / 100.0);
}

VNA::GUIMode* VNA::GUIMode::activeMode = nullptr;
QWidget* VNA::GUIMode::cornerWidget = nullptr;
QButtonGroup* VNA::GUIMode::modeButtonGroup = nullptr;

VNA::GUIMode::GUIMode(VNA *vna, QString name, QWidget *centralWidget)
    : vna(vna),
      name(name),
      central(centralWidget)
{
    vna->central->addWidget(central);
    // Create mode switch button
    auto modeSwitch = new QPushButton(name);
    modeSwitch->setCheckable(true);
    modeSwitch->setMaximumHeight(vna->ui->menubar->height());
    if(!cornerWidget) {
        // this is the first created mode, initialize corner widget and set this mode as active
        modeSwitch->setChecked(true);
        cornerWidget = new QWidget;
        cornerWidget->setLayout(new QHBoxLayout);
        cornerWidget->layout()->setSpacing(0);
        cornerWidget->layout()->setMargin(0);
        cornerWidget->layout()->setContentsMargins(0,0,0,0);
        vna->menuBar()->setCornerWidget(cornerWidget);
        modeButtonGroup = new QButtonGroup;
        vna->ui->menubar->setMaximumHeight(vna->ui->menubar->height());
    }
    cornerWidget->layout()->addWidget(modeSwitch);
    modeButtonGroup->addButton(modeSwitch);

    connect(modeSwitch, &QPushButton::clicked, [=](){
        activate();
    });
}

void VNA::GUIMode::activate()
{
    if(activeMode == this) {
        // already active;
        return;
    } else if(activeMode) {
        activeMode->deactivate();
    }
    QSettings settings;
    // hide menu actions that are not applicable to this mode
    for(auto a : hiddenActions) {
        a->setVisible(false);
    }

    vna->central->setCurrentWidget(central);

    // restore dock and toolbar positions
//    vna->restoreGeometry(settings.value("geometry_"+name).toByteArray());
    vna->restoreState(settings.value("windowState_"+name).toByteArray());

    // restore visibility of toolbars and docks
    vna->ui->menuDocks->clear();
    for(auto d : vna->findChildren<QDockWidget*>()) {
        if(hiddenDocks.count(d)) {
            // this dock is not applicable for the current state, hide and don't add menu entry
            d->hide();
            continue;
        }
        vna->ui->menuDocks->addAction(d->toggleViewAction());
        bool hidden = settings.value("dock_"+name+"_"+d->windowTitle(), d->isHidden()).toBool();
        if(hidden) {
            d->hide();
        } else {
            d->show();
        }
    }
    vna->ui->menuToolbars->clear();
    for(auto t : vna->findChildren<QToolBar*>()) {
        if(hiddenToolbars.count(t)) {
            // this toolbar is not applicable for the current state, hide and don't add menu entry
            t->hide();
            continue;
        }
        vna->ui->menuToolbars->addAction(t->toggleViewAction());
        bool hidden = settings.value("toolbar_"+name+"_"+t->windowTitle(), t->isHidden()).toBool();
        if(hidden) {
            t->hide();
        } else {
            t->show();
        }
    }

    activeMode = this;
}

void VNA::GUIMode::deactivate()
{
    QSettings settings;
    // save dock/toolbar visibility
    for(auto d : vna->findChildren<QDockWidget*>()) {
        settings.setValue("dock_"+name+"_"+d->windowTitle(), d->isHidden());
    }
    for(auto t : vna->findChildren<QToolBar*>()) {
        settings.setValue("toolbar_"+name+"_"+t->windowTitle(), t->isHidden());
    }
//    settings.setValue("geometry_"+name, vna->saveGeometry());
    settings.setValue("windowState_"+name, vna->saveState());

    // restore hidden items
    for(auto a : hiddenActions) {
        a->setVisible(true);
    }

    activeMode = nullptr;
}

VNA::GUIMode *VNA::GUIMode::getActiveMode()
{
    return activeMode;
}

QString VNA::GUIMode::getName() const
{
    return name;
}
