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

using namespace std;

constexpr Protocol::SweepSettings VNA::defaultSweep;

VNA::VNA(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("VNA");
    QCoreApplication::setOrganizationName("Application");

    settings = defaultSweep;
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
            auto fw_update = new FirmwareUpdateDialog(*device);
            fw_update->exec();
        }
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

    auto tw = new TraceWidget(traceModel);
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

    auto menuLayout = new QStackedLayout;
    auto mMain = new Menu(*menuLayout);

    auto mFrequency = new Menu(*menuLayout, "Frequency");
    auto mCenter = new MenuValue("Center Frequency", (settings.f_start + settings.f_stop)/2, "Hz", " kMG", 6);
    mFrequency->addItem(mCenter);
    auto mStart = new MenuValue("Start Frequency", settings.f_start, "Hz", " kMG", 6);
    mFrequency->addItem(mStart);
    auto mStop = new MenuValue("Stop Frequency", settings.f_stop, "Hz", " kMG", 6);
    mFrequency->addItem(mStop);
    mFrequency->finalize();
    mMain->addMenu(mFrequency);

    auto mSpan = new Menu(*menuLayout, "Span");
    auto mSpanWidth = new MenuValue("Span", settings.f_stop - settings.f_start, "Hz", " kMG", 6);
    mSpan->addItem(mSpanWidth);
    auto mSpanZoomIn = new MenuAction("Zoom in");
    mSpan->addItem(mSpanZoomIn);
    auto mSpanZoomOut = new MenuAction("Zoom out");
    mSpan->addItem(mSpanZoomOut);
    auto mSpanFull = new MenuAction("Full span");
    mSpan->addItem(mSpanFull);
    mSpan->finalize();
    mMain->addMenu(mSpan);

    auto mAcquisition = new Menu(*menuLayout, "Acquisition");
    auto mdbm = new MenuValue("Source Level", -10, "dbm", " ");
    mAcquisition->addItem(mdbm);
    auto mPoints = new MenuValue("Points", settings.points, "", " ");
    mAcquisition->addItem(mPoints);
    auto mBandwidth = new MenuValue("IF Bandwidth", settings.if_bandwidth, "Hz", " k", 3);
    mAcquisition->addItem(mBandwidth);
    auto mAverages = new MenuValue("Averages", averages);
    mAcquisition->addItem(mAverages);
    mAcquisition->finalize();
    mMain->addMenu(mAcquisition);

    auto mCalibration = new Menu(*menuLayout, "Calibration");
    auto mCalPort1 = new Menu(*menuLayout, "Port 1");
    auto mCalPort1Open = new MenuAction("Port 1 Open");
    auto mCalPort1Short = new MenuAction("Port 1 Short");
    auto mCalPort1Load = new MenuAction("Port 1 Load");
    mCalPort1->addItem(mCalPort1Short);
    mCalPort1->addItem(mCalPort1Open);
    mCalPort1->addItem(mCalPort1Load);
    mCalPort1->finalize();
    mCalibration->addMenu(mCalPort1);
    auto mCalPort2 = new Menu(*menuLayout, "Port 2");
    auto mCalPort2Open = new MenuAction("Port 2 Open");
    auto mCalPort2Short = new MenuAction("Port 2 Short");
    auto mCalPort2Load = new MenuAction("Port 2 Load");
    mCalPort2->addItem(mCalPort2Short);
    mCalPort2->addItem(mCalPort2Open);
    mCalPort2->addItem(mCalPort2Load);
    mCalPort2->finalize();
    mCalibration->addMenu(mCalPort2);
    auto mCalThrough = new MenuAction("Through");
    auto mCalIsolation = new MenuAction("Isolation");
    mCalibration->addItem(mCalThrough);
    mCalibration->addItem(mCalIsolation);

    mCalSOL1 = new MenuAction("Apply Port 1 SOL");
    mCalibration->addItem(mCalSOL1);
    mCalSOL1->setDisabled(true);

    mCalSOL2 = new MenuAction("Apply Port 2 SOL");
    mCalibration->addItem(mCalSOL2);
    mCalSOL2->setDisabled(true);

    mCalFullSOLT = new MenuAction("Apply full SOLT");
    mCalibration->addItem(mCalFullSOLT);
    mCalFullSOLT->setDisabled(true);

    auto mCalSave = new MenuAction("Save to file");
    mCalibration->addItem(mCalSave);

    auto mCalLoad = new MenuAction("Load from file");
    mCalibration->addItem(mCalLoad);

    auto mEditKit = new MenuAction("Edit CalKit");
    mCalibration->addItem(mEditKit);

    mCalibration->finalize();
    mMain->addMenu(mCalibration);

    auto mSystem = new Menu(*menuLayout, "System");
    auto aManual = new MenuAction("Manual Control");
    auto aMatchDialog = new MenuAction("Impedance Matching");
    mSystem->addItem(aManual);
    mSystem->addItem(aMatchDialog);
    mSystem->finalize();
    mMain->addMenu(mSystem);

    mMain->finalize();

    // Frequency and span connections
    // setting values
    connect(mCenter, &MenuValue::valueChanged, this, &VNA::SetCenterFreq);
    connect(mStart, &MenuValue::valueChanged, this, &VNA::SetStartFreq);
    connect(mStop, &MenuValue::valueChanged, this, &VNA::SetStopFreq);
    connect(mSpanWidth, &MenuValue::valueChanged, this, &VNA::SetSpan);
    connect(mSpanZoomIn, &MenuAction::triggered, this, &VNA::SpanZoomIn);
    connect(mSpanZoomOut, &MenuAction::triggered, this, &VNA::SpanZoomOut);
    connect(mSpanFull, &MenuAction::triggered, this, &VNA::SetFullSpan);
    // readback and update line edits
    connect(this, &VNA::startFreqChanged, mStart, &MenuValue::setValueQuiet);
    connect(this, &VNA::stopFreqChanged, mStop, &MenuValue::setValueQuiet);
    connect(this, &VNA::centerFreqChanged, mCenter, &MenuValue::setValueQuiet);
    connect(this, &VNA::spanChanged, mSpanWidth, &MenuValue::setValueQuiet);

    // Acquisition connections
    // setting values
    connect(mPoints, &MenuValue::valueChanged, [=](double newval){
        SetPoints(newval);
    });
    connect(mdbm, &MenuValue::valueChanged, this, &VNA::SetSourceLevel);
    connect(mBandwidth, &MenuValue::valueChanged, this, &VNA::SetIFBandwidth);
    connect(mAverages, &MenuValue::valueChanged, [=](double newval){
       SetAveraging(newval);
    });
    // readback and update line edits
    connect(this, &VNA::sourceLevelChanged, mdbm, &MenuValue::setValueQuiet);
    connect(this, &VNA::pointsChanged, [=](int newval) {
        mPoints->setValueQuiet(newval);
    });
    connect(this, &VNA::IFBandwidthChanged, mBandwidth, &MenuValue::setValueQuiet);
    connect(this, &VNA::averagingChanged, [=](int newval) {
        mAverages->setValueQuiet(newval);
    });

    connect(mCalPort1Open, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Port1Open);
    });
    connect(mCalPort1Short, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Port1Short);
    });
    connect(mCalPort1Load, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Port1Load);
    });
    connect(mCalPort2Open, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Port2Open);
    });
    connect(mCalPort2Short, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Port2Short);
    });
    connect(mCalPort2Load, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Port2Load);
    });
    connect(mCalThrough, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Through);
    });
    connect(mCalIsolation, &MenuAction::triggered, [=](){
       StartCalibrationMeasurement(Calibration::Measurement::Isolation);
    });
    connect(mCalSOL1, &MenuAction::triggered, [=](){
        ApplyCalibration(Calibration::Type::Port1SOL);
    });
    connect(mCalSOL2, &MenuAction::triggered, [=](){
        ApplyCalibration(Calibration::Type::Port2SOL);
    });
    connect(mCalFullSOLT, &MenuAction::triggered, [=](){
        ApplyCalibration(Calibration::Type::FullSOLT);
    });

    connect(mCalSave, &MenuAction::triggered, [=](){
        cal.saveToFile();
    });

    connect(mCalLoad, &MenuAction::triggered, [=](){
        if(cal.openFromFile()) {
            // Check if applying calibration is available
            if(cal.calculationPossible(Calibration::Type::Port1SOL)) {
                mCalSOL1->setEnabled(true);
            }
            if(cal.calculationPossible(Calibration::Type::Port2SOL)) {
                mCalSOL2->setEnabled(true);
            }
            if(cal.calculationPossible(Calibration::Type::FullSOLT)) {
                mCalFullSOLT->setEnabled(true);
            }
        }
    });

    connect(mEditKit, &MenuAction::triggered, [=](){
        cal.getCalibrationKit().edit();
    });

    // Manual control trigger
    connect(aManual, &MenuAction::triggered, this, &VNA::StartManualControl);
    connect(aMatchDialog, &MenuAction::triggered, this, &VNA::StartImpedanceMatching);

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    auto mainWidget = new QWidget;
    auto mainLayout = new QHBoxLayout;
    mainWidget->setLayout(mainLayout);
    auto statusWidget = new QWidget;
    statusWidget->setLayout(statusLayout);
//    statusWidget->setFixedWidth(150);
    auto statusDock = new QDockWidget("Status");
    statusDock->setWidget(statusWidget);
//    statusDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, statusDock);

    auto tracesDock = new QDockWidget("Traces");
    tracesDock->setWidget(tw);
//    tracesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, tracesDock);

//    mainLayout->addWidget(statusWidget);
    mainLayout->addWidget(tiles);
    auto menuWidget = new QWidget;
    menuWidget->setLayout(menuLayout);
//    menuWidget->setFixedWidth(180);
    auto menuDock = new QDockWidget("Menu");
    menuDock->setWidget(menuWidget);
//    menuDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, menuDock);
//    mainLayout->addWidget(menuWidget);

    auto markerWidget = new MarkerWidget(*markerModel);

    auto markerDock = new QDockWidget("Marker");
    markerDock->setWidget(markerWidget);
//    markerDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, markerDock);

    auto logDock = new QDockWidget("Device Log");
    logDock->setWidget(&deviceLog);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);

    setCentralWidget(mainWidget);

    // status and menu dock hidden by default
    menuDock->close();
    statusDock->close();

    // fill dock/toolbar hide/show menu and set initial state if available
    QSettings settings;
    ui->menuDocks->clear();
    for(auto d : findChildren<QDockWidget*>()) {
        ui->menuDocks->addAction(d->toggleViewAction());
        bool hidden = settings.value("dock_"+d->windowTitle(), d->isHidden()).toBool();
        if(hidden) {
            d->close();
        } else {
            d->show();
        }
    }
    ui->menuToolbars->clear();
    for(auto t : findChildren<QToolBar*>()) {
        ui->menuToolbars->addAction(t->toggleViewAction());
        bool hidden = settings.value("toolbar_"+t->windowTitle(), t->isHidden()).toBool();
        if(hidden) {
            t->close();
        } else {
            t->show();
        }
    }

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    qRegisterMetaType<Protocol::Datapoint>("Datapoint");

    ConstrainAndUpdateFrequencies();

    // List available devices
    if(UpdateDeviceList()) {
        // at least one device available
        ConnectToDevice();
    }
}

void VNA::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    // save dock/toolbar visibility
    for(auto d : findChildren<QDockWidget*>()) {
        settings.setValue("dock_"+d->windowTitle(), d->isHidden());
    }
    ui->menuToolbars->clear();
    for(auto t : findChildren<QToolBar*>()) {
        settings.setValue("toolbar_"+t->windowTitle(), t->isHidden());
    }
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
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
                // Check if applying calibration is available
                if(cal.calculationPossible(Calibration::Type::Port1SOL)) {
                    mCalSOL1->setEnabled(true);
                }
                if(cal.calculationPossible(Calibration::Type::Port2SOL)) {
                    mCalSOL2->setEnabled(true);
                }
                if(cal.calculationPossible(Calibration::Type::FullSOLT)) {
                    mCalFullSOLT->setEnabled(true);
                }
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
        device->Configure(settings);
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

    // Reference toolbar
    auto tb_reference = new QToolBar("Reference", this);
    tb_reference->addWidget(new QLabel("Ref:"));
    toolbars.referenceType = new QComboBox();
    toolbars.referenceType->addItem("Int");
    toolbars.referenceType->addItem("Ext");
    auto refInAuto = new QCheckBox("Auto");
    refInAuto->setChecked(true);
    toolbars.referenceType->setEnabled(false);
    connect(refInAuto, &QCheckBox::clicked, [this](bool checked) {
        // TODO change device settings
        toolbars.referenceType->setEnabled(!checked);
    });
    tb_reference->addWidget(toolbars.referenceType);
    tb_reference->addWidget(refInAuto);
    tb_reference->addSeparator();
    tb_reference->addWidget(new QLabel("Ref out:"));
    auto refOutEnabled = new QCheckBox();
    auto refOutFreq = new QComboBox();
    refOutFreq->addItem("10 MHz");
    refOutFreq->addItem("100 MHz");
    tb_reference->addWidget(refOutEnabled);
    tb_reference->addWidget(refOutFreq);

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
