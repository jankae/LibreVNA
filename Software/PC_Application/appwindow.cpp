#include "appwindow.h"
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
#include "CustomWidgets/toggleswitch.h"
#include "Device/manualcontroldialog.h"
#include "Traces/tracemodel.h"
#include "Traces/tracewidget.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracexyplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include "CustomWidgets/siunitedit.h"
#include <QDockWidget>
#include "Traces/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "Calibration/calibrationtracedialog.h"
#include "ui_main.h"
#include "Device/firmwareupdatedialog.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QActionGroup>
#include <mode.h>
#include "VNA/vna.h"
#include "Generator/generator.h"
#include "SpectrumAnalyzer/spectrumanalyzer.h"
#include "Calibration/sourcecaldialog.h"
#include "Calibration/receivercaldialog.h"
#include "Calibration/frequencycaldialog.h"
#include <QDebug>
#include "CustomWidgets/jsonpickerdialog.h"
#include <QCommandLineParser>

using namespace std;

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , ui(new Ui::MainWindow)
    , server(nullptr)
{
    QCoreApplication::setOrganizationName("LibreVNA");
    QCoreApplication::setApplicationName("LibreVNA-GUI");
    auto commit = QString(GITHASH);
    commit.truncate(7);
    QCoreApplication::setApplicationVersion(QString::number(FW_MAJOR) + "." + QString::number(FW_MINOR)
                                            + "." + QString::number(FW_PATCH) + FW_SUFFIX + " ("+ commit+")");

    qSetMessagePattern("%{time process}: [%{type}] %{message}");

//    qDebug().setVerbosity(0);
    qDebug() << "Application start";

    parser.setApplicationDescription("LibreVNA-GUI");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"p","port"}, "Specify port to listen for SCPI commands", "port"));
    parser.addOption(QCommandLineOption({"d","device"}, "Only allow connections to the specified device", "device"));
    parser.addOption(QCommandLineOption("no-gui", "Disables the graphical interface"));

    parser.process(QCoreApplication::arguments());

    Preferences::getInstance().load();
    device = nullptr;

    if(parser.isSet("port")) {
        bool OK;
        auto port = parser.value("port").toUInt(&OK);
        if(!OK) {
            // set default port
            port = Preferences::getInstance().General.SCPI.port;
        }
        StartTCPServer(port);
        Preferences::getInstance().manualTCPport();
    } else if(Preferences::getInstance().General.SCPI.enabled) {
        StartTCPServer(Preferences::getInstance().General.SCPI.port);
    }

    ui->setupUi(this);
    ui->statusbar->addWidget(&lConnectionStatus);
    auto div1 = new QFrame;
    div1->setFrameShape(QFrame::VLine);
    ui->statusbar->addWidget(div1);
    ui->statusbar->addWidget(&lDeviceInfo);
    ui->statusbar->addWidget(new QLabel, 1);

    lADCOverload.setStyleSheet("color : red");
    lADCOverload.setText("ADC overload");
    lADCOverload.setVisible(false);
    ui->statusbar->addWidget(&lADCOverload);

    lUnlevel.setStyleSheet("color : red");
    lUnlevel.setText("Unlevel");
    lUnlevel.setVisible(false);
    ui->statusbar->addWidget(&lUnlevel);

    lUnlock.setStyleSheet("color : red");
    lUnlock.setText("Unlock");
    lUnlock.setVisible(false);
    ui->statusbar->addWidget(&lUnlock);
    //ui->statusbar->setStyleSheet("QStatusBar::item { border: 1px solid black; };");

    CreateToolbars();
    auto logDock = new QDockWidget("Device Log");
    logDock->setWidget(&deviceLog);
    logDock->setObjectName("Log Dock");
    addDockWidget(Qt::BottomDockWidgetArea, logDock);

    // fill toolbar/dock menu
    ui->menuDocks->clear();
    for(auto d : findChildren<QDockWidget*>()) {
        ui->menuDocks->addAction(d->toggleViewAction());
    }
    ui->menuToolbars->clear();
    for(auto t : findChildren<QToolBar*>()) {
        ui->menuToolbars->addAction(t->toggleViewAction());
    }

    // Create GUI modes
    central = new QStackedWidget;
    setCentralWidget(central);
    vna = new VNA(this);
    generator = new Generator(this);
    spectrumAnalyzer = new SpectrumAnalyzer(this);

    // UI connections
    connect(ui->actionUpdate_Device_List, &QAction::triggered, this, &AppWindow::UpdateDeviceList);
    connect(ui->actionDisconnect, &QAction::triggered, this, &AppWindow::DisconnectDevice);
    connect(ui->actionQuit, &QAction::triggered, this, &AppWindow::close);
    connect(ui->actionSave_setup, &QAction::triggered, [=](){
        auto filename = QFileDialog::getSaveFileName(nullptr, "Save setup data", "", "Setup files (*.setup)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(!filename.endsWith(".setup")) {
            filename.append(".setup");
        }
        ofstream file;
        file.open(filename.toStdString());
        file << setw(4) << SaveSetup() << endl;
        file.close();
    });
    connect(ui->actionLoad_setup, &QAction::triggered, [=](){
        auto filename = QFileDialog::getOpenFileName(nullptr, "Load setup data", "", "Setup files (*.setup)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        ifstream file;
        file.open(filename.toStdString());
        if(!file.is_open()) {
            qWarning() << "Unable to open file:" << filename;
            return;
        }
        nlohmann::json j;
        file >> j;
        file.close();
        LoadSetup(j);
    });
    connect(ui->actionSave_image, &QAction::triggered, [=](){
        Mode::getActiveMode()->saveSreenshot();
    });

    connect(ui->actionManual_Control, &QAction::triggered, this, &AppWindow::StartManualControl);
    connect(ui->actionFirmware_Update, &QAction::triggered, this, &AppWindow::StartFirmwareUpdateDialog);
    connect(ui->actionSource_Calibration, &QAction::triggered, this, &AppWindow::SourceCalibrationDialog);
    connect(ui->actionReceiver_Calibration, &QAction::triggered, this, &AppWindow::ReceiverCalibrationDialog);
    connect(ui->actionFrequency_Calibration, &QAction::triggered, this, &AppWindow::FrequencyCalibrationDialog);
    connect(ui->actionPreferences, &QAction::triggered, [=](){
        // save previous SCPI settings in case they change
        auto &p = Preferences::getInstance();
        auto SCPIenabled = p.General.SCPI.enabled;
        auto SCPIport = p.General.SCPI.port;
        p.edit();
        if(SCPIenabled != p.General.SCPI.enabled || SCPIport != p.General.SCPI.port) {
            StopTCPServer();
            if(p.General.SCPI.enabled) {
                StartTCPServer(p.General.SCPI.port);
            }
        }
        // settings might have changed, update necessary stuff
//        TraceXYPlot::updateGraphColors();
    });
    connect(ui->actionAbout, &QAction::triggered, [=](){
        QMessageBox::about(this, "About", "More information: github.com/jankae/LibreVNA\n"
                           "\nVersion: " + QCoreApplication::applicationVersion());
    });

    setWindowTitle("LibreVNA-GUI");

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    {
        QSettings settings;
        restoreGeometry(settings.value("geometry").toByteArray());
    }

    SetupSCPI();

    // Set default mode
    vna->activate();

    qRegisterMetaType<Protocol::Datapoint>("Datapoint");
    qRegisterMetaType<Protocol::ManualStatus>("Manual");
    qRegisterMetaType<Protocol::SpectrumAnalyzerResult>("SpectrumAnalyzerResult");
    qRegisterMetaType<Protocol::AmplitudeCorrectionPoint>("AmplitudeCorrection");

    // List available devices
    if(UpdateDeviceList() && Preferences::getInstance().Startup.ConnectToFirstDevice) {
        // at least one device available
        ConnectToDevice();
    }
    if(!parser.isSet("no-gui")) {
        resize(1280, 800);
        show();
    }
}

AppWindow::~AppWindow()
{
    StopTCPServer();
    delete ui;
}

void AppWindow::closeEvent(QCloseEvent *event)
{
    vna->shutdown();
    generator->shutdown();
    spectrumAnalyzer->shutdown();
    delete device;
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    // deactivate currently used mode (stores mode state in settings)
    if(Mode::getActiveMode()) {
        Mode::getActiveMode()->deactivate();
    }
    Preferences::getInstance().store();
    QMainWindow::closeEvent(event);
}

bool AppWindow::ConnectToDevice(QString serial)
{
    if(serial.isEmpty()) {
        qDebug() << "Trying to connect to any device";
    } else {
        qDebug() << "Trying to connect to" << serial;
    }
    if(device) {
        qDebug() << "Already connected to a device, disconnecting first...";
        DisconnectDevice();
    }
    try {
        qDebug() << "Attempting to connect to device...";
        device = new Device(serial);
        lConnectionStatus.setText("Connected to " + device->serial());
        qInfo() << "Connected to" << device->serial();
        lDeviceInfo.setText(device->getLastDeviceInfoString());
        connect(device, &Device::LogLineReceived, &deviceLog, &DeviceLog::addLine);
        connect(device, &Device::ConnectionLost, this, &AppWindow::DeviceConnectionLost);
        connect(device, &Device::DeviceInfoUpdated, [this]() {
           lDeviceInfo.setText(device->getLastDeviceInfoString());
           lADCOverload.setVisible(device->Info().ADC_overload);
           lUnlevel.setVisible(device->Info().unlevel);
           lUnlock.setVisible(!device->Info().LO1_locked || !device->Info().source_locked);
        });
        connect(device, &Device::NeedsFirmwareUpdate, this, &AppWindow::DeviceNeedsUpdate);
        ui->actionDisconnect->setEnabled(true);
        ui->actionManual_Control->setEnabled(true);
        ui->actionFirmware_Update->setEnabled(true);
        ui->actionSource_Calibration->setEnabled(true);
        ui->actionReceiver_Calibration->setEnabled(true);
        ui->actionFrequency_Calibration->setEnabled(true);

        Mode::getActiveMode()->initializeDevice();
        UpdateReference();

        for(auto d : deviceActionGroup->actions()) {
            if(d->text() == device->serial()) {
                d->blockSignals(true);
                d->setChecked(true);
                d->blockSignals(false);
                break;
            }
        }
        return true;
    } catch (const runtime_error &e) {
        qWarning() << "Failed to connect:" << e.what();
        DisconnectDevice();
        UpdateDeviceList();
        return false;
    }
}

void AppWindow::DisconnectDevice()
{
    delete device;
    device = nullptr;
    ui->actionDisconnect->setEnabled(false);
    ui->actionManual_Control->setEnabled(false);
    ui->actionFirmware_Update->setEnabled(false);
    ui->actionSource_Calibration->setEnabled(false);
    ui->actionReceiver_Calibration->setEnabled(false);
    ui->actionFrequency_Calibration->setEnabled(false);
    for(auto a : deviceActionGroup->actions()) {
        a->setChecked(false);
    }
    if(deviceActionGroup->checkedAction()) {
        deviceActionGroup->checkedAction()->setChecked(false);
    }
    lConnectionStatus.setText("No device connected");
    lDeviceInfo.setText("No device information available yet");
    Mode::getActiveMode()->deviceDisconnected();
    qDebug() << "Disconnected device";
}

void AppWindow::DeviceConnectionLost()
{
    DisconnectDevice();
    QMessageBox::warning(this, "Disconnected", "The USB connection to the device has been lost");
    UpdateDeviceList();
}

void AppWindow::CreateToolbars()
{
    // Reference toolbar
    auto tb_reference = new QToolBar("Reference", this);
    tb_reference->addWidget(new QLabel("Ref in:"));
    toolbars.reference.type = new QComboBox();
    toolbars.reference.type->addItem("Int");
    toolbars.reference.type->addItem("Ext");
    toolbars.reference.type->addItem("Auto");
    tb_reference->addWidget(toolbars.reference.type);
    tb_reference->addSeparator();
    tb_reference->addWidget(new QLabel("Ref out:"));
    toolbars.reference.outFreq = new QComboBox();
    toolbars.reference.outFreq->addItem("Off");
    toolbars.reference.outFreq->addItem("10 MHz");
    toolbars.reference.outFreq->addItem("100 MHz");
    tb_reference->addWidget(toolbars.reference.outFreq);
    connect(toolbars.reference.type, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppWindow::UpdateReference);
    connect(toolbars.reference.outFreq, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppWindow::UpdateReference);
    addToolBar(tb_reference);
    tb_reference->setObjectName("Reference Toolbar");
}

void AppWindow::SetupSCPI()
{
    scpi.add(new SCPICommand("*IDN", nullptr, [=](QStringList){
        return "LibreVNA-GUI";
    }));
    auto scpi_dev = new SCPINode("DEVice");
    scpi.add(scpi_dev);
    scpi_dev->add(new SCPICommand("DISConnect", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        DisconnectDevice();
        return "";
    }, nullptr));
    scpi_dev->add(new SCPICommand("CONNect", [=](QStringList params) -> QString {
        QString serial;
        if(params.size() > 0) {
            serial = params[0];
        }
        if(!ConnectToDevice(serial)) {
            return "Device not found";
        } else {
            return "";
        }
    }, [=](QStringList) -> QString {
        if(device) {
            return device->serial();
        } else {
            return "Not connected";
        }
    }));
    scpi_dev->add(new SCPICommand("LIST", nullptr, [=](QStringList) -> QString {
        QString ret;
        for(auto d : Device::GetDevices()) {
            ret += d + ",";
        }
        // remove last comma
        ret.chop(1);
        return ret;
    }));
    auto scpi_ref = new SCPINode("REFerence");
    scpi_dev->add(scpi_ref);
    scpi_ref->add(new SCPICommand("OUT", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return "ERROR";
        } else if(params[0] == "0" || params[0] == "OFF") {
            toolbars.reference.outFreq->setCurrentIndex(0);
        } else if(params[0] == "10") {
            toolbars.reference.outFreq->setCurrentIndex(1);
        } else if(params[0] == "100") {
            toolbars.reference.outFreq->setCurrentIndex(2);
        } else {
            return "ERROR";
        }
        return "";
    }, [=](QStringList) -> QString {
        switch(toolbars.reference.outFreq->currentIndex()) {
        case 0: return "OFF";
        case 1: return "10";
        case 2: return "100";
        default: return "ERROR";
        }
    }));
    scpi_ref->add(new SCPICommand("IN", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return "ERROR";
        } else if(params[0] == "INT") {
            toolbars.reference.type->setCurrentIndex(0);
        } else if(params[0] == "EXT") {
            toolbars.reference.type->setCurrentIndex(1);
        } else if(params[0] == "AUTO") {
            toolbars.reference.type->setCurrentIndex(2);
        } else {
            return "ERROR";
        }
        return "";
    }, [=](QStringList) -> QString {
        switch(Device::Info().extRefInUse) {
        case 0: return "INT";
        case 1: return "EXT";
        default: return "ERROR";
        }
    }));
    scpi_dev->add(new SCPICommand("MODE", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return "ERROR";
        }
        if (params[0] == "VNA") {
            vna->activate();
        } else if(params[0] == "GEN") {
            generator->activate();
        } else if(params[0] == "SA") {
            spectrumAnalyzer->activate();
        } else {
            return "INVALID MDOE";
        }
        return "";
    }, [=](QStringList) -> QString {
        auto active = Mode::getActiveMode();
        if(active == vna) {
            return "VNA";
        } else if(active == generator) {
            return "GEN";
        } else if(active == spectrumAnalyzer) {
            return "SA";
        } else {
            return "ERROR";
        }
    }));
    auto scpi_status = new SCPINode("STAtus");
    scpi_dev->add(scpi_status);
    scpi_status->add(new SCPICommand("UNLOcked", nullptr, [=](QStringList){
        bool locked = Device::Info().source_locked && Device::Info().LO1_locked;
        return locked ? "FALSE" : "TRUE";
    }));
    scpi_status->add(new SCPICommand("ADCOVERload", nullptr, [=](QStringList){
        return Device::Info().ADC_overload ? "TRUE" : "FALSE";
    }));
    scpi_status->add(new SCPICommand("UNLEVel", nullptr, [=](QStringList){
        return Device::Info().unlevel ? "TRUE" : "FALSE";
    }));
    auto scpi_info = new SCPINode("INFo");
    scpi_dev->add(scpi_info);
    scpi_info->add(new SCPICommand("FWREVision", nullptr, [=](QStringList){
        return QString::number(Device::Info().FW_major)+"."+QString::number(Device::Info().FW_minor)+"."+QString::number(Device::Info().FW_patch);
    }));
    scpi_info->add(new SCPICommand("HWREVision", nullptr, [=](QStringList){
        return QString(Device::Info().HW_Revision);
    }));
    scpi_info->add(new SCPICommand("TEMPeratures", nullptr, [=](QStringList){
        return QString::number(Device::Info().temp_source)+"/"+QString::number(Device::Info().temp_LO1)+"/"+QString::number(Device::Info().temp_MCU);
    }));
    auto scpi_limits = new SCPINode("LIMits");
    scpi_info->add(scpi_limits);
    scpi_limits->add(new SCPICommand("MINFrequency", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_minFreq);
    }));
    scpi_limits->add(new SCPICommand("MAXFrequency", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_maxFreq);
    }));
    scpi_limits->add(new SCPICommand("MINIFBW", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_minIFBW);
    }));
    scpi_limits->add(new SCPICommand("MAXIFBW", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_maxIFBW);
    }));
    scpi_limits->add(new SCPICommand("MAXPoints", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_maxPoints);
    }));
    scpi_limits->add(new SCPICommand("MINPOWer", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_cdbm_min / 100.0);
    }));
    scpi_limits->add(new SCPICommand("MAXPOWer", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_cdbm_max / 100.0);
    }));
    scpi_limits->add(new SCPICommand("MINRBW", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_minRBW);
    }));
    scpi_limits->add(new SCPICommand("MAXRBW", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_maxRBW);
    }));
    scpi_limits->add(new SCPICommand("MAXHARMonicfrequency", nullptr, [=](QStringList){
        return QString::number(Device::Info().limits_maxFreqHarmonic);
    }));

    scpi.add(vna);
    scpi.add(generator);
    scpi.add(spectrumAnalyzer);
}

void AppWindow::StartTCPServer(int port)
{
    server = new TCPServer(port);
    connect(server, &TCPServer::received, &scpi, &SCPI::input);
    connect(&scpi, &SCPI::output, server, &TCPServer::send);
}

void AppWindow::StopTCPServer()
{
    delete server;
    server = nullptr;
}

int AppWindow::UpdateDeviceList()
{
    deviceActionGroup->setExclusive(true);
    ui->menuConnect_to->clear();
    auto devices = Device::GetDevices();
    if(device) {
        devices.insert(device->serial());
    }
    int available = 0;
    if(devices.size()) {
        for(auto d : devices) {
            if(!parser.value("device").isEmpty() && parser.value("device") != d) {
                // specified device does not match, ignore
                continue;
            }
            auto connectAction = ui->menuConnect_to->addAction(d);
            connectAction->setCheckable(true);
            connectAction->setActionGroup(deviceActionGroup);
            if(device && d == device->serial()) {
                connectAction->setChecked(true);
            }
            connect(connectAction, &QAction::triggered, [this, d]() {
               ConnectToDevice(d);
            });
            ui->menuConnect_to->setEnabled(true);
            available++;
        }
    } else {
        // no devices available, disable connection option
        ui->menuConnect_to->setEnabled(false);
    }
    qDebug() << "Updated device list, found" << available;
    return available;
}

void AppWindow::StartManualControl()
{
    auto control = new ManualControlDialog(*device, this);
    connect(control, &QDialog::finished, [=](){
        if(device) {
            Mode::getActiveMode()->initializeDevice();
        }
    });
    control->show();
}

void AppWindow::UpdateReference()
{
    if(!device) {
        // can't update without a device connected
        return;
    }
    Protocol::ReferenceSettings s = {};

    QString txt1 = toolbars.reference.type->currentText();
    if( (txt1=="Ext") || (txt1=="External") ) {
        s.UseExternalRef = 1;
    }
    if( (txt1=="Auto") || (txt1=="Automatic") ) {
        s.AutomaticSwitch = 1;
    }

    QString txt2 = toolbars.reference.outFreq->currentText();
    if(txt2=="10 MHz"){
        s.ExtRefOuputFreq = 10000000;
    }
    if(txt2=="100 MHz"){
        s.ExtRefOuputFreq = 100000000;
    }

    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::Reference;
    p.reference = s;
    device->SendPacket(p);
}

void AppWindow::StartFirmwareUpdateDialog()
{
    if(device) {
        auto fw_update = new FirmwareUpdateDialog(device);
        connect(fw_update, &FirmwareUpdateDialog::DeviceRebooting, this, &AppWindow::DisconnectDevice);
        connect(fw_update, &FirmwareUpdateDialog::DeviceRebooted, this, &AppWindow::ConnectToDevice);
        fw_update->exec();
    }
}

void AppWindow::DeviceNeedsUpdate(int reported, int expected)
{
    auto ret = QMessageBox::warning(this, "Warning",
                                "The device reports a different protocol"
                                "version (" + QString::number(reported) + ") than expected (" + QString::number(expected) + ").\n"
                                "A firmware update is strongly recommended. Do you want to update now?",
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (ret == QMessageBox::Yes) {
        StartFirmwareUpdateDialog();
    }
}

void AppWindow::SourceCalibrationDialog()
{
    auto d = new SourceCalDialog(device);
    d->exec();
}

void AppWindow::ReceiverCalibrationDialog()
{
    auto d = new ReceiverCalDialog(device);
    d->exec();
}

void AppWindow::FrequencyCalibrationDialog()
{
    auto d = new FrequencyCalDialog(device);
    d->exec();
}

nlohmann::json AppWindow::SaveSetup()
{
    nlohmann::json j;
    j["VNA"] = vna->toJSON();
    j["Generator"] = generator->toJSON();
    j["SpectrumAnalyzer"] = spectrumAnalyzer->toJSON();
    return j;
}

void AppWindow::LoadSetup(nlohmann::json j)
{
//    auto d = new JSONPickerDialog(j);
//    d->exec();
    vna->fromJSON(j["VNA"]);
    generator->fromJSON(j["Generator"]);
    spectrumAnalyzer->fromJSON(j["SpectrumAnalyzer"]);
}

Device *AppWindow::getDevice() const
{
    return device;
}

QStackedWidget *AppWindow::getCentral() const
{
    return central;
}

Ui::MainWindow *AppWindow::getUi() const
{
    return ui;
}
