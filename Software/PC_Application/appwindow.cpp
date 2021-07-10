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
#include "Util/app_common.h"
#include "about.h"

using namespace std;


static const QString APP_VERSION = QString::number(FW_MAJOR) + "." +
                                   QString::number(FW_MINOR) + "." +
                                   QString::number(FW_PATCH);
static const QString APP_GIT_HASH = QString(GITHASH);

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , manual(nullptr)
    , ui(new Ui::MainWindow)
    , server(nullptr)
    , appVersion(APP_VERSION)
    , appGitHash(APP_GIT_HASH)
{
    qSetMessagePattern("%{time process}: [%{type}] %{message}");

//    qDebug().setVerbosity(0);
    qDebug() << "Application start";

    parser.setApplicationDescription(qlibrevnaApp->applicationName());
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
            port = Preferences::getInstance().SCPIServer.port;
        }
        StartTCPServer(port);
        Preferences::getInstance().manualTCPport();
    } else if(Preferences::getInstance().SCPIServer.enabled) {
        StartTCPServer(Preferences::getInstance().SCPIServer.port);
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
        auto SCPIenabled = p.SCPIServer.enabled;
        auto SCPIport = p.SCPIServer.port;
        p.edit();
        if(SCPIenabled != p.SCPIServer.enabled || SCPIport != p.SCPIServer.port) {
            StopTCPServer();
            if(p.SCPIServer.enabled) {
                StartTCPServer(p.SCPIServer.port);
            }
        }
        auto active = Mode::getActiveMode();

        if(active == spectrumAnalyzer) {
            spectrumAnalyzer->updateGraphColors();
        }
        else if (active == vna) {
             vna->updateGraphColors();
        }

    });

    connect(ui->actionAbout, &QAction::triggered, [=](){
        auto &a = About::getInstance();
        a.about();
    });

    setWindowTitle(qlibrevnaApp->applicationName() + " v"  + getAppVersion());

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

    auto scpi_manual = new SCPINode("MANual");
    scpi_manual->add(new SCPICommand("STArt",[=](QStringList) -> QString {
        StartManualControl();
        return "";
    }, nullptr));
    scpi_manual->add(new SCPICommand("STOp",[=](QStringList) -> QString {
        manual->close();
        delete manual;
        return "";
    }, nullptr));

    auto addBooleanManualSetting = [=](QString cmd, void(ManualControlDialog::*set)(bool), bool(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            bool enable;
            if(!manual || !SCPI::paramToBool(params, 0, enable)) {
                return "ERROR";
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            set_fn(enable);
            return "";
        }, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            return get_fn() ? "TRUE" : "FALSE";
        }));
    };

    auto addDoubleManualSetting = [=](QString cmd, void(ManualControlDialog::*set)(double), double(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            double value;
            if(!manual || !SCPI::paramToDouble(params, 0, value)) {
                return "ERROR";
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            set_fn(value);
            return "";
        }, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addIntegerManualSetting = [=](QString cmd, void(ManualControlDialog::*set)(int), int(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            double value;
            if(!manual || !SCPI::paramToDouble(params, 0, value)) {
                return "ERROR";
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            set_fn(value);
            return "";
        }, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addIntegerManualSettingWithReturnValue = [=](QString cmd, bool(ManualControlDialog::*set)(int), int(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            double value;
            if(!manual || !SCPI::paramToDouble(params, 0, value)) {
                return "ERROR";
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            if(set_fn(value)) {
                return "";
            } else {
                return "ERROR";
            }
        }, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addIntegerManualQuery = [=](QString cmd, int(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addDoubleManualQuery = [=](QString cmd, double(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addBooleanManualQuery = [=](QString cmd, bool(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            return get_fn() ? "TRUE" : "FALSE";
        }));
    };
    auto addComplexManualQuery = [=](QString cmd, std::complex<double>(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return "ERROR";
            }
            auto get_fn = std::bind(get, manual);
            auto res = get_fn();
            return QString::number(res.real())+","+QString::number(res.imag());
        }));
    };

    addBooleanManualSetting("HSRC_CE", &ManualControlDialog::setHighSourceChipEnable, &ManualControlDialog::getHighSourceChipEnable);
    addBooleanManualSetting("HSRC_RFEN", &ManualControlDialog::setHighSourceRFEnable, &ManualControlDialog::getHighSourceRFEnable);
    addBooleanManualQuery("HSRC_LOCKed", &ManualControlDialog::getHighSourceLocked);
    addIntegerManualSettingWithReturnValue("HSRC_PWR", &ManualControlDialog::setHighSourcePower, &ManualControlDialog::getHighSourcePower);
    addDoubleManualSetting("HSRC_FREQ", &ManualControlDialog::setHighSourceFrequency, &ManualControlDialog::getHighSourceFrequency);
    scpi_manual->add(new SCPICommand("HSRC_LPF", [=](QStringList params) -> QString {
        long value;
        if(!manual || !SCPI::paramToLong(params, 0, value)) {
            return "ERROR";
        }
        switch(value) {
        case 947:
            manual->setHighSourceLPF(ManualControlDialog::LPF::M947);
            break;
        case 1880:
            manual->setHighSourceLPF(ManualControlDialog::LPF::M1880);
            break;
        case 3500:
            manual->setHighSourceLPF(ManualControlDialog::LPF::M3500);
            break;
        case 0:
            manual->setHighSourceLPF(ManualControlDialog::LPF::None);
            break;
        default:
            return "ERROR";
        }
        return "";
    }, [=](QStringList) -> QString {
        if(!manual) {
            return "ERROR";
        }
        auto lpf = manual->getHighSourceLPF();
        switch(lpf) {
        case ManualControlDialog::LPF::M947: return "947";
        case ManualControlDialog::LPF::M1880: return "1880";
        case ManualControlDialog::LPF::M3500: return "3500";
        case ManualControlDialog::LPF::None: return "0";
        default: return "ERROR";
        }
    }));
    addBooleanManualSetting("LSRC_EN", &ManualControlDialog::setLowSourceEnable, &ManualControlDialog::getLowSourceEnable);
    addIntegerManualSettingWithReturnValue("LSRC_PWR", &ManualControlDialog::setLowSourcePower, &ManualControlDialog::getLowSourcePower);
    addDoubleManualSetting("LSRC_FREQ", &ManualControlDialog::setLowSourceFrequency, &ManualControlDialog::getLowSourceFrequency);
    addBooleanManualSetting("BAND_SW", &ManualControlDialog::setHighband, &ManualControlDialog::getHighband);
    addDoubleManualSetting("ATTenuator", &ManualControlDialog::setAttenuator, &ManualControlDialog::getAttenuator);
    addBooleanManualSetting("AMP_EN", &ManualControlDialog::setAmplifierEnable, &ManualControlDialog::getAmplifierEnable);
    addIntegerManualSettingWithReturnValue("PORT_SW", &ManualControlDialog::setPortSwitch, &ManualControlDialog::getPortSwitch);
    addBooleanManualSetting("LO1_CE", &ManualControlDialog::setLO1ChipEnable, &ManualControlDialog::getLO1ChipEnable);
    addBooleanManualSetting("LO1_RFEN", &ManualControlDialog::setLO1RFEnable, &ManualControlDialog::getLO1RFEnable);
    addBooleanManualQuery("LO1_LOCKed", &ManualControlDialog::getLO1Locked);
    addDoubleManualSetting("LO1_FREQ", &ManualControlDialog::setLO1Frequency, &ManualControlDialog::getLO1Frequency);
    addDoubleManualSetting("IF1_FREQ", &ManualControlDialog::setIF1Frequency, &ManualControlDialog::getIF1Frequency);
    addBooleanManualSetting("LO2_EN", &ManualControlDialog::setLO2Enable, &ManualControlDialog::getLO2Enable);
    addDoubleManualSetting("LO2_FREQ", &ManualControlDialog::setLO2Frequency, &ManualControlDialog::getLO2Frequency);
    addDoubleManualSetting("IF2_FREQ", &ManualControlDialog::setIF2Frequency, &ManualControlDialog::getIF2Frequency);
    addBooleanManualSetting("PORT1_EN", &ManualControlDialog::setPort1Enable, &ManualControlDialog::getPort1Enable);
    addBooleanManualSetting("PORT2_EN", &ManualControlDialog::setPort2Enable, &ManualControlDialog::getPort2Enable);
    addBooleanManualSetting("REF_EN", &ManualControlDialog::setRefEnable, &ManualControlDialog::getRefEnable);
    addIntegerManualSetting("SAMPLES", &ManualControlDialog::setNumSamples, &ManualControlDialog::getNumSamples);
    scpi_manual->add(new SCPICommand("WINdow", [=](QStringList params) -> QString {
        if(!manual || params.size() < 1) {
            return "ERROR";
        }
        if (params[0] == "NONE") {
            manual->setWindow(ManualControlDialog::Window::None);
        } else if(params[0] == "KAISER") {
            manual->setWindow(ManualControlDialog::Window::Kaiser);
        } else if(params[0] == "HANN") {
            manual->setWindow(ManualControlDialog::Window::Hann);
        } else if(params[0] == "FLATTOP") {
            manual->setWindow(ManualControlDialog::Window::FlatTop);
        } else {
            return "INVALID WINDOW";
        }
        return "";
    }, [=](QStringList) -> QString {
        if(!manual) {
            return "ERROR";
        }
        switch((ManualControlDialog::Window) manual->getWindow()) {
        case ManualControlDialog::Window::None: return "NONE";
        case ManualControlDialog::Window::Kaiser: return "KAISER";
        case ManualControlDialog::Window::Hann: return "HANN";
        case ManualControlDialog::Window::FlatTop: return "FLATTOP";
        default: return "ERROR";
        }
    }));
    addIntegerManualQuery("PORT1_MIN", &ManualControlDialog::getPort1MinADC);
    addIntegerManualQuery("PORT1_MAX", &ManualControlDialog::getPort1MaxADC);
    addDoubleManualQuery("PORT1_MAG", &ManualControlDialog::getPort1Magnitude);
    addDoubleManualQuery("PORT1_PHAse", &ManualControlDialog::getPort1Phase);
    addComplexManualQuery("PORT1_REFerenced", &ManualControlDialog::getPort1Referenced);

    addIntegerManualQuery("PORT2_MIN", &ManualControlDialog::getPort2MinADC);
    addIntegerManualQuery("PORT2_MAX", &ManualControlDialog::getPort2MaxADC);
    addDoubleManualQuery("PORT2_MAG", &ManualControlDialog::getPort2Magnitude);
    addDoubleManualQuery("PORT2_PHAse", &ManualControlDialog::getPort2Phase);
    addComplexManualQuery("PORT2_REFerenced", &ManualControlDialog::getPort2Referenced);

    addIntegerManualQuery("REF_MIN", &ManualControlDialog::getRefMinADC);
    addIntegerManualQuery("REF_MAX", &ManualControlDialog::getRefMaxADC);
    addDoubleManualQuery("REF_MAG", &ManualControlDialog::getRefMagnitude);
    addDoubleManualQuery("REF_PHAse", &ManualControlDialog::getRefPhase);

    scpi.add(scpi_manual);
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
    if(manual) {
        // dialog already active, nothing to do
        return;
    }
    manual = new ManualControlDialog(*device, this);
    connect(manual, &QDialog::finished, [=](){
        manual = nullptr;
        if(device) {
            Mode::getActiveMode()->initializeDevice();
        }
    });
    manual->show();
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

const QString& AppWindow::getAppVersion() const
{
    return appVersion;
}

const QString& AppWindow::getAppGitHash() const
{
    return appGitHash;
}
