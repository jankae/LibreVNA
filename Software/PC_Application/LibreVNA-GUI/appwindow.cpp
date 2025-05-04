#include "appwindow.h"

#include "unit.h"
#include "CustomWidgets/toggleswitch.h"
#include "Traces/tracemodel.h"
#include "Traces/tracewidget.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracexyplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include "CustomWidgets/siunitedit.h"
#include "Traces/Marker/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "ui_main.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"
#include "VNA/vna.h"
#include "Generator/generator.h"
#include "SpectrumAnalyzer/spectrumanalyzer.h"
#include "CustomWidgets/informationbox.h"
#include "Util/app_common.h"
#include "about.h"
#include "mode.h"
#include "modehandler.h"
#include "modewindow.h"
#include "Device/LibreVNA/librevnausbdriver.h"
#include "Device/LibreVNA/librevnatcpdriver.h"

#include <QDockWidget>
#include <QApplication>
#include <QActionGroup>
#include <QDebug>
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
#include <iomanip>
#include <QDateTime>
#include <QCommandLineParser>
#include <QScrollArea>
#include <QStringList>

using namespace std;


static const QString APP_VERSION = QString::number(FW_MAJOR) + "." +
                                   QString::number(FW_MINOR) + "." +
                                   QString::number(FW_PATCH) + QString(FW_SUFFIX);
static const QString APP_GIT_HASH = QString(GITHASH);

static bool noGUIset = false;

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , ui(new Ui::MainWindow)
    , server(nullptr)
    , streamVNARawData(nullptr)
    , streamVNACalibratedData(nullptr)
    , streamVNADeembeddedData(nullptr)
    , streamSARawData(nullptr)
    , streamSANormalizedData(nullptr)
    , appVersion(APP_VERSION)
    , appGitHash(APP_GIT_HASH)
{

//    qDebug().setVerbosity(0);
    qDebug() << "Application start";

    this->setWindowIcon(QIcon(":/app/logo.png"));

    parser.setApplicationDescription(qlibrevnaApp->applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"p","port"}, "Specify port to listen for SCPI commands", "port"));
    parser.addOption(QCommandLineOption({"d","device"}, "Only allow connections to the specified device", "device"));
    parser.addOption(QCommandLineOption("no-gui", "Disables the graphical interface"));
    parser.addOption(QCommandLineOption("cal", "Calibration file to load on startup", "cal"));
    parser.addOption(QCommandLineOption("setup", "Setup file to load on startup", "setup"));
    parser.addOption(QCommandLineOption("reset-preferences", "Resets all preferences to their default values"));

    parser.process(QCoreApplication::arguments());

    if(parser.isSet("reset-preferences")) {
        Preferences::getInstance().setDefault();
    } else {
        Preferences::getInstance().load();
    }

    auto &p = Preferences::getInstance();

    device = nullptr;
//    vdevice = nullptr;
    modeHandler = nullptr;

    if(parser.isSet("port")) {
        bool OK;
        auto port = parser.value("port").toUInt(&OK);
        if(!OK) {
            // set default port
            port = Preferences::getInstance().SCPIServer.port;
        }
        StartTCPServer(port);
        p.manualTCPport();
    } else if(p.SCPIServer.enabled) {
        StartTCPServer(p.SCPIServer.port);
    }

    if(p.StreamingServers.VNARawData.enabled) {
        streamVNARawData = new StreamingServer(p.StreamingServers.VNARawData.port);
    }
    if(p.StreamingServers.VNACalibratedData.enabled) {
        streamVNACalibratedData = new StreamingServer(p.StreamingServers.VNACalibratedData.port);
    }
    if(p.StreamingServers.VNADeembeddedData.enabled) {
        streamVNADeembeddedData = new StreamingServer(p.StreamingServers.VNADeembeddedData.port);
    }
    if(p.StreamingServers.SARawData.enabled) {
        streamSARawData = new StreamingServer(p.StreamingServers.SARawData.port);
    }
    if(p.StreamingServers.SANormalizedData.enabled) {
        streamSANormalizedData = new StreamingServer(p.StreamingServers.SANormalizedData.port);
    }

    ui->setupUi(this);

    SetupStatusBar();
    UpdateStatusBar(DeviceStatusBar::Disconnected);

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

    modeHandler = new ModeHandler(this);
    auto modeWindow = new ModeWindow(modeHandler, this);
    ui->menubar->insertMenu(ui->menuHelp->menuAction(), modeWindow->getMenu());

    central = new QStackedWidget;
    setCentralWidget(central);

    auto setModeStatusbar = [=](const QString &msg) {
        lModeInfo.setText(msg);
    };

    connect(modeHandler, &ModeHandler::StatusBarMessageChanged, setModeStatusbar);
    connect(modeHandler, &ModeHandler::CurrentModeChanged, this, &AppWindow::UpdateImportExportMenus);

    SetupMenu();

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

    SetInitialState();

    auto& pref = Preferences::getInstance();
    // List available devices
    UpdateDeviceList();
    if(pref.Startup.ConnectToFirstDevice && deviceList.size() > 0) {
        // at least one device available
        ConnectToDevice(deviceList[0].serial);
    }

    if(parser.isSet("setup")) {
        LoadSetup(parser.value("setup"));
    }
    if(parser.isSet("cal")) {
        VNA* mode = static_cast<VNA*>(modeHandler->findFirstOfType(Mode::Type::VNA));
        mode->LoadCalibration(parser.value("cal"));
    }
    if(!parser.isSet("no-gui")) {
        InformationBox::setGUI(true);
        resize(1280, 800);
        show();
    } else {
        InformationBox::setGUI(false);
        noGUIset = true;
    }
}

AppWindow::~AppWindow()
{
    StopTCPServer();
    delete ui;
}

void AppWindow::SetupMenu()
{
    // UI connections
    connect(ui->actionUpdate_Device_List, &QAction::triggered, this, &AppWindow::UpdateDeviceList);
    connect(ui->actionDisconnect, &QAction::triggered, this, &AppWindow::DisconnectDevice);
    connect(ui->actionQuit, &QAction::triggered, this, &AppWindow::close);
    connect(ui->actionSave_setup, &QAction::triggered, [=](){
        auto filename = QFileDialog::getSaveFileName(nullptr, "Save setup data", "", "Setup files (*.setup)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        SaveSetup(filename);
    });
    connect(ui->actionLoad_setup, &QAction::triggered, [=](){
        auto filename = QFileDialog::getOpenFileName(nullptr, "Load setup data", "", "Setup files (*.setup)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        LoadSetup(filename);
    });
    connect(ui->actionSave_image, &QAction::triggered, [=](){
        modeHandler->getActiveMode()->saveSreenshot();
    });

    connect(ui->actionPreset, &QAction::triggered, [=](){
        modeHandler->getActiveMode()->preset();
    });

    connect(ui->actionPreferences, &QAction::triggered, [=](){
        // save previous SCPI settings in case they change
        auto &p = Preferences::getInstance();
        p.edit();
        preferencesChanged();
    });

    connect(ui->actionAbout, &QAction::triggered, [=](){
        auto &a = About::getInstance();
        a.about();
    });
}

void AppWindow::closeEvent(QCloseEvent *event)
{
    auto& pref = Preferences::getInstance();
    if(pref.Startup.UseSetupFile && pref.Startup.AutosaveSetupFile) {
        SaveSetup(pref.Startup.SetupFile);
    }
    modeHandler->shutdown();
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    // deactivate currently used mode (stores mode state in settings)
    if(modeHandler->getActiveMode()) {
        modeHandler->deactivate(modeHandler->getActiveMode());
    }
    if(device) {
        device->disconnectDevice();
        device = nullptr;
    }
    delete modeHandler;
    modeHandler = nullptr;
    pref.store();
    for(auto driver : DeviceDriver::getDrivers()) {
        delete driver;
    }
    QMainWindow::closeEvent(event);
}

void AppWindow::SetInitialState()
{
    modeHandler->closeModes();

    auto& pref = Preferences::getInstance();
    if(pref.Startup.UseSetupFile) {
        LoadSetup(pref.Startup.SetupFile);
    } else {
        auto vnaIndex = modeHandler->createMode("Vector Network Analyzer", Mode::Type::VNA);
        modeHandler->createMode("Signal Generator", Mode::Type::SG);
        modeHandler->createMode("Spectrum Analyzer", Mode::Type::SA);
        modeHandler->setCurrentIndex(vnaIndex);
    }
}

void AppWindow::SetResetState()
{
    modeHandler->closeModes();
    auto vnaIndex = modeHandler->createMode("Vector Network Analyzer", Mode::Type::VNA);
    modeHandler->createMode("Signal Generator", Mode::Type::SG);
    modeHandler->createMode("Spectrum Analyzer", Mode::Type::SA);

    for(auto m : modeHandler->getModes()) {
        m->resetSettings();
    }

    modeHandler->setCurrentIndex(vnaIndex);
}

bool AppWindow::ConnectToDevice(QString serial, DeviceDriver *driver)
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
        for(auto d : DeviceDriver::getDrivers()) {
            if(driver && driver != d) {
                // not the specified driver
                continue;
            }
            if(d->GetAvailableDevices().count(serial)) {
                // this driver can connect to the device
                connect(d, &DeviceDriver::InfoUpdated, this, &AppWindow::DeviceInfoUpdated, Qt::QueuedConnection);
                connect(d, &DeviceDriver::LogLineReceived, &deviceLog, &DeviceLog::addLine);
                connect(d, &DeviceDriver::ConnectionLost, this, &AppWindow::DeviceConnectionLost);
                connect(d, &DeviceDriver::StatusUpdated, this, &AppWindow::DeviceStatusUpdated);
                connect(d, &DeviceDriver::FlagsUpdated, this, &AppWindow::DeviceFlagsUpdated);
                connect(d, &DeviceDriver::releaseControl, this, [=](){
                    if(lastActiveMode) {
                        modeHandler->activate(lastActiveMode);
                    }
                });
                connect(d, &DeviceDriver::acquireControl, this, [=](){
                   lastActiveMode = modeHandler->getActiveMode();
                   modeHandler->deactivate(lastActiveMode);
                });
                connect(d, &DeviceDriver::addSCPICommand, this, [=](SCPICommand *cmd){
                    temporaryDeviceCommands.push_back(cmd);
                    scpi.add(cmd);
                });
                connect(d, &DeviceDriver::removeSCPICommand, this, [=](SCPICommand *cmd){
                    auto it = std::find(temporaryDeviceCommands.begin(), temporaryDeviceCommands.end(), cmd);
                    if(it != temporaryDeviceCommands.end()) {
                        temporaryDeviceCommands.erase(it);
                    }
                    scpi.remove(cmd);
                });
                connect(d, &DeviceDriver::addSCPINode, this, [=](SCPINode *node){
                    temporaryDeviceNodes.push_back(node);
                    scpi.add(node);
                });
                connect(d, &DeviceDriver::removeSCPINode, this, [=](SCPINode *node){
                    auto it = std::find(temporaryDeviceNodes.begin(), temporaryDeviceNodes.end(), node);
                    if(it != temporaryDeviceNodes.end()) {
                        temporaryDeviceNodes.erase(it);
                    }
                    scpi.remove(node);
                });

                if(d->connectDevice(serial)) {
                    device = d;
                } else {
                    disconnect(d, nullptr, this, nullptr);
                    UpdateDeviceList();
                    break;
                }
            }
        }
        if(!device) {
            // failed to connect
            InformationBox::ShowError("Failed to connect", "Could not connect to "+serial);
            return false;
        }
        UpdateStatusBar(AppWindow::DeviceStatusBar::Connected);
//        connect(vdevice, &VirtualDevice::NeedsFirmwareUpdate, this, &AppWindow::DeviceNeedsUpdate);
        ui->actionDisconnect->setEnabled(true);
        // find correct position to add device specific actions at
        QAction *before = nullptr;
        for(int i=0;i<ui->menuDevice->actions().size();i++) {
            auto comp = ui->menuDevice->actions()[i];
            if(comp == ui->actionDisconnect) {
                if(i + 2 < ui->menuDevice->actions().size()) {
                    before = ui->menuDevice->actions()[i+2];
                }
                break;
             }
        }
        for(auto a : device->driverSpecificActions()) {
            ui->menuDevice->insertAction(before, a);
        }
        ui->actionPreset->setEnabled(true);

        // Add SCPI nodes/commands
        for(auto n : device->driverSpecificSCPINodes()) {
            scpi.add(n);
        }
        for(auto c : device->driverSpecificSCPICommands()) {
            scpi.add(c);
        }

        DeviceEntry e;
        e.serial = device->getSerial();
        e.driver = device;
        for(auto d : deviceActionGroup->actions()) {
            if(d->text() == e.toString()) {
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
    if(device) {
        // remove menu entries
        for(auto a : device->driverSpecificActions()) {
            ui->menuDevice->removeAction(a);
        }
        // remove SCPI nodes/commands
        for(auto n : device->driverSpecificSCPINodes()) {
            scpi.remove(n);
        }
        for(auto c : device->driverSpecificSCPICommands()) {
            scpi.remove(c);
        }

        // Remove all temporary SCPI nodes/commands
        for(auto n : temporaryDeviceNodes) {
            scpi.remove(n);
        }
        temporaryDeviceNodes.clear();
        for(auto c : temporaryDeviceCommands) {
            scpi.remove(c);
        }
        temporaryDeviceCommands.clear();

        device->disconnectDevice();
        disconnect(device, nullptr, &deviceLog, nullptr);
        disconnect(device, nullptr, this, nullptr);
        device = nullptr;
    }
    ui->actionDisconnect->setEnabled(false);
    ui->actionManual_Control->setEnabled(false);
    ui->actionFirmware_Update->setEnabled(false);
    ui->actionSource_Calibration->setEnabled(false);
    ui->actionReceiver_Calibration->setEnabled(false);
    ui->actionFrequency_Calibration->setEnabled(false);
    ui->actionPreset->setEnabled(false);
    for(auto a : deviceActionGroup->actions()) {
        a->setChecked(false);
    }
    if(deviceActionGroup->checkedAction()) {
        deviceActionGroup->checkedAction()->setChecked(false);
    }
    UpdateStatusBar(DeviceStatusBar::Disconnected);
    if(modeHandler->getActiveMode()) {
        modeHandler->getActiveMode()->deviceDisconnected();
    }
    qDebug() << "Disconnected device";
}

void AppWindow::DeviceConnectionLost()
{
    DisconnectDevice();
    InformationBox::ShowError("Disconnected", "The connection to the device has been lost");
    UpdateDeviceList();
}

void AppWindow::CreateToolbars()
{
    // Reference toolbar
    auto tb_reference = new QToolBar("Reference", this);
    tb_reference->addWidget(new QLabel("Ref in:"));
    toolbars.reference.type = new QComboBox();
    tb_reference->addWidget(toolbars.reference.type);
    tb_reference->addSeparator();
    tb_reference->addWidget(new QLabel("Ref out:"));
    toolbars.reference.outFreq = new QComboBox();
    tb_reference->addWidget(toolbars.reference.outFreq);
    connect(toolbars.reference.type, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppWindow::ReferenceChanged);
    connect(toolbars.reference.outFreq, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppWindow::ReferenceChanged);
    addToolBar(tb_reference);
    tb_reference->setObjectName("Reference Toolbar");

    referenceTimer.setSingleShot(true);
    connect(&referenceTimer, &QTimer::timeout, this, &AppWindow::UpdateReference);
}

void AppWindow::SetupSCPI()
{
    scpi.add(new SCPICommand("*IDN", nullptr, [=](QStringList){
        QString ret = "LibreVNA,LibreVNA-GUI,";
        if(device) {
            ret += device->getSerial();
        } else {
            ret += "Not connected";
        }
        ret += ","+appVersion;
        return ret;
    }));
    scpi.add(new SCPICommand("*RST", [=](QStringList){
        SetResetState();
        ResetReference();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    auto scpi_dev = new SCPINode("DEVice");
    scpi.add(scpi_dev);
    scpi_dev->add(new SCPICommand("DISConnect", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        DisconnectDevice();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    scpi_dev->add(new SCPICommand("CONNect", [=](QStringList params) -> QString {
        QString serial;
        if(params.size() > 0) {
            serial = params[0];
        } else if(UpdateDeviceList() > 0) {
            serial = deviceList[0].serial;
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(!ConnectToDevice(serial)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        if(device) {
            return device->getSerial();
        } else {
            return "Not connected";
        }
    }));
    scpi_dev->add(new SCPICommand("LIST", nullptr, [=](QStringList) -> QString {
        QString ret;
        UpdateDeviceList();
        for(auto entry : deviceList) {
            ret += entry.serial + ",";
        }
        // remove last comma
        ret.chop(1);
        return ret;
    }));
    scpi_dev->add(new SCPICommand("PREFerences", [=](QStringList params) -> QString {
        if(params.size() != 2) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        auto &p = Preferences::getInstance();
        if(p.set(params[0], QVariant(params[1]))) {
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }, [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        auto value = Preferences::getInstance().get(params[0]).toString();
        if(value.isEmpty()) {
            // failed to get setting
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            return value;
        }
    }, false));
    scpi_dev->add(new SCPICommand("APPLYPREFerences", [=](QStringList) -> QString {
        preferencesChanged();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    auto scpi_setup = new SCPINode("SETUP");
    scpi_dev->add(scpi_setup);
    scpi_setup->add(new SCPICommand("SAVE", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            // no filename given
            return SCPI::getResultName(SCPI::Result::Error);
        }
        SaveSetup(params[0]);
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr, false));
    scpi_setup->add(new SCPICommand("LOAD", nullptr, [=](QStringList params) -> QString {
        if(params.size() != 1) {
            // no filename given
            return SCPI::getResultName(SCPI::Result::False);
        }
        if(!LoadSetup(params[0])) {
            // some error when loading the setup file
            return SCPI::getResultName(SCPI::Result::False);
        }
        return SCPI::getResultName(SCPI::Result::True);
    }, false));
    auto scpi_ref = new SCPINode("REFerence");
    scpi_dev->add(scpi_ref);
    scpi_ref->add(new SCPICommand("OUT", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else if(params[0] == "0" || params[0] == "OFF") {
            int index = toolbars.reference.outFreq->findText("Off");
            if(index >= 0) {
                toolbars.reference.outFreq->setCurrentIndex(index);
            } else {
                return SCPI::getResultName(SCPI::Result::Error);
            }
        } else {
            bool isInt;
            params[0].toInt(&isInt);
            if(isInt) {
                params[0].append(" MHz");
                int index = toolbars.reference.outFreq->findText(params[0]);
                if(index >= 0) {
                    toolbars.reference.outFreq->setCurrentIndex(index);
                } else {
                    return SCPI::getResultName(SCPI::Result::Error);
                }
            } else {
                return SCPI::getResultName(SCPI::Result::Error);
            }
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        auto fOutString = toolbars.reference.outFreq->currentText().toUpper();
        if(fOutString.endsWith(" MHZ")) {
            fOutString.chop(4);
        }
        if(fOutString.isEmpty()) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            return fOutString;
        }
    }));
    scpi_ref->add(new SCPICommand("IN", [=](QStringList params) -> QString {
        // reference settings translation
        map<QString, QString> translation {
            make_pair("INT", "Internal"),
            make_pair("EXT", "External"),
            make_pair("AUTO", "Auto"),
        };
        if(params.size() != 1 || translation.count(params[0]) == 0) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            int index = toolbars.reference.type->findText(translation[params[0]]);
            if(index >= 0) {
                toolbars.reference.type->setCurrentIndex(index);
            } else {
                return SCPI::getResultName(SCPI::Result::Error);
            }
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        if(device) {
            return device->asserted(DeviceDriver::Flag::ExtRef) ? "EXT" : "INT";
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_dev->add(new SCPICommand("MODE", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        Mode *mode = nullptr;
        if (params[0] == "VNA") {
            mode = modeHandler->findFirstOfType(Mode::Type::VNA);
        } else if(params[0] == "GEN") {
            mode = modeHandler->findFirstOfType(Mode::Type::SG);
        } else if(params[0] == "SA") {
            mode = modeHandler->findFirstOfType(Mode::Type::SA);
        } else {
            return "INVALID MDOE";
        }
        if(mode) {
            int index = modeHandler->findIndex(mode);
            modeHandler->setCurrentIndex(index);
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }, [=](QStringList) -> QString {
        auto active = modeHandler->getActiveMode();
        if(active) {
            switch(active->getType()) {
            case Mode::Type::VNA: return "VNA";
            case Mode::Type::SG: return "GEN";
            case Mode::Type::SA: return "SA";
            case Mode::Type::Last: return SCPI::getResultName(SCPI::Result::Error);
            }
        }
        return SCPI::getResultName(SCPI::Result::Error);
    }));
    auto scpi_status = new SCPINode("STAtus");
    scpi_dev->add(scpi_status);
    scpi_status->add(new SCPICommand("UNLOcked", nullptr, [=](QStringList){
        if(device) {
            return device->asserted(DeviceDriver::Flag::Unlocked) ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_status->add(new SCPICommand("ADCOVERload", nullptr, [=](QStringList){
        if(device) {
            return device->asserted(DeviceDriver::Flag::Overload) ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_status->add(new SCPICommand("UNLEVel", nullptr, [=](QStringList){
        if(device) {
            return device->asserted(DeviceDriver::Flag::Unlevel) ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    auto scpi_info = new SCPINode("INFo");
    scpi_dev->add(scpi_info);
    scpi_info->add(new SCPICommand("FWREVision", nullptr, [=](QStringList){
        if(device) {
            return device->getInfo().firmware_version;
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_info->add(new SCPICommand("HWREVision", nullptr, [=](QStringList){
        if(device) {
            return device->getInfo().hardware_version;
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    auto scpi_limits = new SCPINode("LIMits");
    scpi_info->add(scpi_limits);
    scpi_limits->add(new SCPICommand("MINFrequency", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.VNA.minFreq);
    }));
    scpi_limits->add(new SCPICommand("MAXFrequency", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.SA.maxFreq);
    }));
    scpi_limits->add(new SCPICommand("MINIFBW", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.VNA.minIFBW);
    }));
    scpi_limits->add(new SCPICommand("MAXIFBW", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.VNA.maxIFBW);
    }));
    scpi_limits->add(new SCPICommand("MAXPoints", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.VNA.maxPoints);
    }));
    scpi_limits->add(new SCPICommand("MINPOWer", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.VNA.mindBm);
    }));
    scpi_limits->add(new SCPICommand("MAXPOWer", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.VNA.maxdBm);
    }));
    scpi_limits->add(new SCPICommand("MINRBW", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.SA.minRBW);
    }));
    scpi_limits->add(new SCPICommand("MAXRBW", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.SA.maxRBW);
    }));
    scpi_limits->add(new SCPICommand("MAXHARMonicfrequency", nullptr, [=](QStringList){
        return QString::number(DeviceDriver::getInfo(getDevice()).Limits.VNA.maxFreq);
    }));
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

void AppWindow::preferencesChanged()
{
    auto &p = Preferences::getInstance();
    p.store();
    if(p.SCPIServer.enabled && !server) {
        StartTCPServer(p.SCPIServer.port);
    } else if(!p.SCPIServer.enabled && server) {
        StopTCPServer();
    } else if(server && server->getPort() != p.SCPIServer.port) {
        // still enabled but the port changed -> needs to restart the SCPI server
        StopTCPServer();
        StartTCPServer(p.SCPIServer.port);
    }

    auto updateStreamingServer = [](StreamingServer **server, bool enabled, int port) {
        if(*server && !enabled) {
            delete *server;
            *server = nullptr;
        } else if(!*server && enabled) {
            *server = new StreamingServer(port);
        } else if(*server && (*server)->getPort() != port) {
            delete *server;
            *server = new StreamingServer(port);
        }
    };

    updateStreamingServer(&streamVNARawData, p.StreamingServers.VNARawData.enabled, p.StreamingServers.VNARawData.port);
    updateStreamingServer(&streamVNACalibratedData, p.StreamingServers.VNACalibratedData.enabled, p.StreamingServers.VNACalibratedData.port);
    updateStreamingServer(&streamVNADeembeddedData, p.StreamingServers.VNADeembeddedData.enabled, p.StreamingServers.VNADeembeddedData.port);
    updateStreamingServer(&streamSARawData, p.StreamingServers.SARawData.enabled, p.StreamingServers.SARawData.port);
    updateStreamingServer(&streamSANormalizedData, p.StreamingServers.SANormalizedData.enabled, p.StreamingServers.SANormalizedData.port);

    // averaging mode may have changed, update for all relevant modes
    for (auto m : modeHandler->getModes())
    {
        switch (m->getType())
        {
            case Mode::Type::VNA:
            case Mode::Type::SA:
                if(p.Acquisition.useMedianAveraging) {
                    m->setAveragingMode(Averaging::Mode::Median);
                }
                else {
                    m->setAveragingMode(Averaging::Mode::Mean);
                }
                break;
            case Mode::Type::SG:
            case Mode::Type::Last:
            default:
                break;
        }
    }

    auto active = modeHandler->getActiveMode();
    if (active)
    {
        active->updateGraphColors();
        if(device) {
            active->initializeDevice();
        }
    }
}

SCPI* AppWindow::getSCPI()
{
    return &scpi;
}

void AppWindow::addStreamingData(const DeviceDriver::VNAMeasurement &m, VNADataType type, bool is_zerospan)
{
    StreamingServer *server = nullptr;
    switch(type) {
    case VNADataType::Raw: server = streamVNARawData; break;
    case VNADataType::Calibrated: server = streamVNACalibratedData; break;
    case VNADataType::Deembedded: server = streamVNADeembeddedData; break;
    }

    if(server) {
        server->addData(m, is_zerospan);
    }
}

void AppWindow::addStreamingData(const DeviceDriver::SAMeasurement &m, SADataType type, bool is_zerospan)
{
    StreamingServer *server = nullptr;
    switch(type) {
    case SADataType::Raw: server = streamSARawData; break;
    case SADataType::Normalized: server = streamSANormalizedData; break;
    }

    if(server) {
        server->addData(m, is_zerospan);
    }
}

void AppWindow::setModeStatus(QString msg)
{
    lModeInfo.setText(msg);
}

int AppWindow::UpdateDeviceList()
{
    deviceActionGroup->setExclusive(true);
    ui->menuConnect_to->clear();
    deviceList.clear();
    for(auto driver : DeviceDriver::getDrivers()) {
        for(auto serial : driver->GetAvailableDevices()) {
            DeviceEntry e;
            e.driver = driver;
            e.serial = serial;
            if(!parser.value("device").isEmpty() && parser.value("device") != e.serial) {
                // specified device does not match, ignore
                continue;
            }
            deviceList.push_back(e);
        }
    }
    if(device) {
        DeviceEntry e;
        e.driver = device;
        e.serial = device->getSerial();
        if(std::find(deviceList.begin(), deviceList.end(), e) == deviceList.end()) {
            // connected device is not in list (this may happen if the driver does not detect a connected device as "available")
            deviceList.push_back(e);
        }
    }
    int available = 0;
    bool found = false;
    for(auto d : deviceList) {
        auto connectAction = ui->menuConnect_to->addAction(d.toString());
        connectAction->setCheckable(true);
        connectAction->setActionGroup(deviceActionGroup);
        if(device && d.serial == device->getSerial()) {
            connectAction->setChecked(true);
        }
        connect(connectAction, &QAction::triggered, [this, d]() {
           ConnectToDevice(d.serial, d.driver);
        });
        found = true;
        available++;
    }
    ui->menuConnect_to->setEnabled(found);
    qDebug() << "Updated device list, found" << available;
    return available;
}

void AppWindow::ResetReference()
{
    toolbars.reference.type->blockSignals(true);
    toolbars.reference.outFreq->blockSignals(true);
    toolbars.reference.type->setCurrentIndex(0);
    toolbars.reference.outFreq->setCurrentIndex(0);
    toolbars.reference.type->blockSignals(false);
    toolbars.reference.outFreq->blockSignals(false);
    ReferenceChanged();
}

//void AppWindow::StartManualControl()
//{
//    if(!vdevice || vdevice->isCompoundDevice()) {
//        return;
//    }
//    if(manual) {
//        // dialog already active, nothing to do
//        return;
//    }
//    manual = new ManualControlDialog(*vdevice->getDevice(), this);
//    connect(manual, &QDialog::finished, [=](){
//        manual = nullptr;
//        if(vdevice) {
//            modeHandler->getActiveMode()->initializeDevice();
//        }
//    });
//    if(AppWindow::showGUI()) {
//        manual->show();
//    }
//}

void AppWindow::UpdateReferenceToolbar()
{
    toolbars.reference.type->blockSignals(true);
    toolbars.reference.outFreq->blockSignals(true);
    toolbars.reference.type->setEnabled(device && device->supports(DeviceDriver::Feature::ExtRefIn));
    toolbars.reference.outFreq->setEnabled(device && device->supports(DeviceDriver::Feature::ExtRefOut));
    if(device) {
        // save current setting
        auto refInBuf = toolbars.reference.type->currentText();
        auto refOutBuf = toolbars.reference.outFreq->currentText();
        toolbars.reference.type->clear();
        for(auto in : device->availableExtRefInSettings()) {
            toolbars.reference.type->addItem(in);
        }
        toolbars.reference.outFreq->clear();
        for(auto out : device->availableExtRefOutSettings()) {
            toolbars.reference.outFreq->addItem(out);
        }
        // restore previous setting if still available
        if(toolbars.reference.type->findText(refInBuf) >= 0) {
            toolbars.reference.type->setCurrentText(refInBuf);
        } else {
            toolbars.reference.type->setCurrentIndex(0);
        }
        if(toolbars.reference.outFreq->findText(refOutBuf) >= 0) {
            toolbars.reference.outFreq->setCurrentText(refOutBuf);
        } else {
            toolbars.reference.outFreq->setCurrentIndex(0);
        }
    }
    toolbars.reference.type->blockSignals(false);
    toolbars.reference.outFreq->blockSignals(false);
    ReferenceChanged();
}

void AppWindow::ReferenceChanged()
{
    if(!device) {
        // can't update without a device connected
        return;
    }
    referenceTimer.start(100);
}

void AppWindow::UpdateReference()
{
    if(!device) {
        // can't update without a device connected
        return;
    }
    device->setExtRef(toolbars.reference.type->currentText(), toolbars.reference.outFreq->currentText());
}

//void AppWindow::DeviceNeedsUpdate(int reported, int expected)
//{
//    auto ret = InformationBox::AskQuestion("Warning",
//                                "The device reports a different protocol"
//                                "version (" + QString::number(reported) + ") than expected (" + QString::number(expected) + ").\n"
//                                "A firmware update is strongly recommended. Do you want to update now?", false);
//    if (ret) {
//        if (vdevice->isCompoundDevice()) {
//            InformationBox::ShowError("Unable to update the firmware", "The connected device is a compound device, direct firmware"
//                                    " update is not supported. Connect to each LibreVNA individually for the update.");
//            return;
//        }
//        StartFirmwareUpdateDialog();
//    }
//}

void AppWindow::DeviceStatusUpdated()
{
    lDeviceInfo.setText(device->getStatus());
}

void AppWindow::DeviceFlagsUpdated()
{
    lADCOverload.setVisible(device->asserted(DeviceDriver::Flag::Overload));
    lUnlevel.setVisible(device->asserted(DeviceDriver::Flag::Unlevel));
    lUnlock.setVisible(device->asserted(DeviceDriver::Flag::Unlocked));
}

void AppWindow::DeviceInfoUpdated()
{
    if (modeHandler->getActiveMode()) {
        modeHandler->getActiveMode()->initializeDevice();
    }
    UpdateReferenceToolbar();
    for(auto m : modeHandler->getModes()) {
        m->deviceInfoUpdated();
    }
}

//void AppWindow::SourceCalibrationDialog()
//{
//    if(!vdevice || vdevice->isCompoundDevice()) {
//        return;
//    }
//    auto d = new SourceCalDialog(vdevice->getDevice(), modeHandler);
//    if(AppWindow::showGUI()) {
//        d->exec();
//    }
//}

//void AppWindow::ReceiverCalibrationDialog()
//{
//    if(!vdevice || vdevice->isCompoundDevice()) {
//        return;
//    }
//    auto d = new ReceiverCalDialog(vdevice->getDevice(), modeHandler);
//    if(AppWindow::showGUI()) {
//        d->exec();
//    }
//}

//void AppWindow::FrequencyCalibrationDialog()
//{
//    if(!vdevice || vdevice->isCompoundDevice()) {
//        return;
//    }
//    auto d = new FrequencyCalDialog(vdevice->getDevice(), modeHandler);
//    if(AppWindow::showGUI()) {
//        d->exec();
//    }
//}

void AppWindow::SaveSetup(QString filename)
{
    if(!filename.endsWith(".setup")) {
        filename.append(".setup");
    }
    ofstream file;
    file.open(filename.toStdString());
    file << setw(4) << SaveSetup() << endl;
    file.close();
    QFileInfo fi(filename);
    lSetupName.setText("Setup: "+fi.fileName());
}

nlohmann::json AppWindow::SaveSetup()
{
    nlohmann::json j;
    nlohmann::json jm;
    for(auto m : modeHandler->getModes()) {
        nlohmann::json jmode;
        jmode["type"] = Mode::TypeToName(m->getType()).toStdString();
        jmode["name"] = m->getName().toStdString();
        jmode["settings"] = m->toJSON();
        jm.push_back(jmode);
    }
    j["Modes"] = jm;
    if(modeHandler->getActiveMode()) {
        j["activeMode"] = modeHandler->getActiveMode()->getName().toStdString();
    }
    nlohmann::json ref;

    ref["Mode"] = toolbars.reference.type->currentText().toStdString();
    ref["Output"] =  toolbars.reference.outFreq->currentText().toStdString();
    j["Reference"] = ref;
    j["version"] = qlibrevnaApp->applicationVersion().toStdString();
    return j;
}

bool AppWindow::LoadSetup(QString filename)
{
    ifstream file;
    file.open(filename.toStdString());
    if(!file.is_open()) {
        qWarning() << "Unable to open file:" << filename;
        return false;
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (exception &e) {
        InformationBox::ShowError("Error", "Failed to parse the setup file (" + QString(e.what()) + ")");
        qWarning() << "Parsing of setup file failed: " << e.what();
        file.close();
        return false;
    }
    file.close();
    LoadSetup(j);
    QFileInfo fi(filename);
    lSetupName.setText("Setup: "+fi.fileName());
    return true;
}

void AppWindow::UpdateImportExportMenus()
{
    // clear menus of all actions first
    ui->menuImport->clear();
    ui->menuExport->clear();

    // add action from currently active mode
    auto active = modeHandler->getActiveMode();
    if(active) {
        for(auto a : active->getImportOptions()) {
            ui->menuImport->addAction(a);
        }
        for(auto a : active->getExportOptions()) {
            ui->menuExport->addAction(a);
        }
    }
    // disable/enable menus
    ui->menuImport->setEnabled(ui->menuImport->actions().size());
    ui->menuExport->setEnabled(ui->menuExport->actions().size());
}

void AppWindow::LoadSetup(nlohmann::json j)
{
//    auto d = new JSONPickerDialog(j);
//    d->exec();
    if(j.contains("Reference")) {
        toolbars.reference.type->setCurrentText(QString::fromStdString(j["Reference"].value("Mode", "Internal")));
        toolbars.reference.outFreq->setCurrentText(QString::fromStdString(j["Reference"].value("Output", "Off")));
    }

    // Disconnect device prior to deleting and creating new modes. This prevents excessice and unnnecessary configuration of the device
    QString serial = QString();
    if(device) {
        serial = device->getSerial();
        DisconnectDevice();
    }

    modeHandler->closeModes();

    /* old style VNA/Generator/Spectrum Analyzer settings,
     * no more than one instance in each mode running */
    if(j.contains("VNA")) {
        auto vnaIndex = modeHandler->createMode("Vector Network Analyzer", Mode::Type::VNA);
        auto *vna = static_cast<VNA*>(modeHandler->getMode(vnaIndex));
        vna->fromJSON(j["VNA"]);
    }
    if(j.contains("Generator")) {
        auto sgIndex = modeHandler->createMode("Generator", Mode::Type::SG);
        auto *generator = static_cast<Generator*>(modeHandler->getMode(sgIndex));
        generator->fromJSON(j["Generator"]);
    }
    if(j.contains("SpectrumAnalyzer")) {
        auto saIndex = modeHandler->createMode("Spectrum Analyzer", Mode::Type::SA);
        auto *spectrumAnalyzer = static_cast<SpectrumAnalyzer*>(modeHandler->getMode(saIndex));
        spectrumAnalyzer->fromJSON(j["SpectrumAnalyzer"]);
    }
    if(j.contains("Modes")) {
        for(auto jm : j["Modes"]) {
            auto type = Mode::TypeFromName(QString::fromStdString(jm.value("type", "Invalid")));
            if(type != Mode::Type::Last && jm.contains("settings")) {
                auto index = modeHandler->createMode(QString::fromStdString(jm.value("name", "")), type);
                auto m = modeHandler->getMode(index);
                m->fromJSON(jm["settings"]);
            }
        }
    }

    // reconnect to device
    if(!serial.isEmpty()) {
        ConnectToDevice(serial);
    }

    // activate the correct mode
    QString modeName = QString::fromStdString(j.value("activeMode", ""));
    for(auto m : modeHandler->getModes()) {
        if(m->getName() == modeName) {
            auto index = modeHandler->findIndex(m);
            modeHandler->setCurrentIndex(index);
            break;
        }
    }
    // if no mode is activated, there might have been a problem with the setup file. Activate the first mode anyway, to prevent invalid GUI state
    if(!modeHandler->getActiveMode() && modeHandler->getModes().size() > 0) {
        modeHandler->activate(modeHandler->getModes()[0]);
    }
}

DeviceDriver *AppWindow::getDevice()
{
    return device;
}

QStackedWidget *AppWindow::getCentral() const
{
    return central;
}

ModeHandler* AppWindow::getModeHandler() const
{
    return modeHandler;
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

bool AppWindow::showGUI()
{
    return !noGUIset;
}

void AppWindow::SetupStatusBar()
{
    ui->statusbar->addWidget(&lConnectionStatus);
    auto div1 = new QFrame;
    div1->setFrameShape(QFrame::VLine);
    ui->statusbar->addWidget(div1);
    ui->statusbar->addWidget(&lDeviceInfo);
    ui->statusbar->addWidget(new QLabel, 1);

    ui->statusbar->addWidget(&lSetupName);
    lSetupName.setText("Setup: -");
    auto div2 = new QFrame;
    div2->setFrameShape(QFrame::VLine);
    ui->statusbar->addWidget(div2);
    ui->statusbar->addWidget(&lModeInfo);
    auto div3 = new QFrame;
    div3->setFrameShape(QFrame::VLine);
    ui->statusbar->addWidget(div3);

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
}

void AppWindow::UpdateStatusBar(DeviceStatusBar status)
{
    switch(status) {
    case DeviceStatusBar::Connected:
        lConnectionStatus.setText("Connected to " + device->getSerial());
        qInfo() << "Connected to" << device->getSerial();
        break;
    case DeviceStatusBar::Disconnected:
        lConnectionStatus.setText("No device connected");
        lDeviceInfo.setText("No status information available yet");
        break;
    default:
        // invalid status
        break;
    }
}


QString AppWindow::DeviceEntry::toString()
{
    return serial + " (" + driver->getDriverName()+")";
}

AppWindow::DeviceEntry AppWindow::DeviceEntry::fromString(QString s, std::vector<DeviceDriver*> drivers)
{
    DeviceEntry e;
    QStringList parts = s.split(" ");
    if(parts.size() < 2) {
        // invalid string
        e.serial = "";
        e.driver = nullptr;
    } else {
        e.serial = parts[0];
        e.driver = nullptr;
        parts[1].chop(1);
        auto driverName = parts[1].mid(1);
        for(auto d : drivers) {
            if(d->getDriverName() == driverName) {
                e.driver = d;
                break;
            }
        }
    }
    return e;
}
