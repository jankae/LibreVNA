#include "appwindow.h"

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
#include "Traces/Marker/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "Calibration/calibrationtracedialog.h"
#include "ui_main.h"
#include "Device/firmwareupdatedialog.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"
#include "VNA/vna.h"
#include "Generator/generator.h"
#include "SpectrumAnalyzer/spectrumanalyzer.h"
#include "Calibration/sourcecaldialog.h"
#include "Calibration/receivercaldialog.h"
#include "Calibration/frequencycaldialog.h"
#include "CustomWidgets/jsonpickerdialog.h"
#include "CustomWidgets/informationbox.h"
#include "Util/app_common.h"
#include "about.h"
#include "mode.h"
#include "modehandler.h"
#include "modewindow.h"

#include <QDockWidget>
#include <QDesktopWidget>
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
#include <QDateTime>
#include <QCommandLineParser>

using namespace std;


static const QString APP_VERSION = QString::number(FW_MAJOR) + "." +
                                   QString::number(FW_MINOR) + "." +
                                   QString::number(FW_PATCH);
static const QString APP_GIT_HASH = QString(GITHASH);

static bool noGUIset = false;


class Reference
{
public:

    enum class TypeIn {
        Internal,
        External,
        Auto,
        None
    };

    enum class OutFreq {
        MHZ10,
        MHZ100,
        Off,
        None
    };

    static QString OutFreqToLabel(Reference::OutFreq t)
    {
        switch(t) {
        case OutFreq::MHZ10: return "10 MHz";
        case OutFreq::MHZ100: return "100 MHz";
        case OutFreq::Off: return "Off";
        default: return "Invalid";
        }
    }

    static QString OutFreqToKey(Reference::OutFreq f)
    {
        switch(f) {
        case OutFreq::MHZ10: return "10 MHz";
        case OutFreq::MHZ100: return "100 MHz";
        case OutFreq::Off: return "Off";
        default: return "Invalid";
        }
    }

    static Reference::OutFreq KeyToOutFreq(QString key)
    {
        for (auto r: Reference::getOutFrequencies()) {
            if(OutFreqToKey((Reference::OutFreq) r) == key) {
                return (Reference::OutFreq) r;
            }
        }
        // not found
        return Reference::OutFreq::None;
    }


    static QString TypeToLabel(TypeIn t)
    {
        switch(t) {
        case TypeIn::Internal: return "Internal";
        case TypeIn::External: return "External";
        case TypeIn::Auto: return "Auto";
        default: return "Invalid";
        }
    }

    static const QString TypeToKey(TypeIn t)
    {
        switch(t) {
        case TypeIn::Internal: return "Int";
        case TypeIn::External: return "Ext";
        case TypeIn::Auto: return "Auto";
        default: return "Invalid";
        }
    }

    static TypeIn KeyToType(QString key)
    {
        for (auto r: Reference::getReferencesIn()) {
            if(TypeToKey((TypeIn) r) == key) {
                return (TypeIn) r;
            }
        }
        // not found
        return TypeIn::None;
    }

    static std::vector<Reference::TypeIn> getReferencesIn()
    {
        return {TypeIn::Internal, TypeIn::External, TypeIn::Auto};
    }

    static std::vector<Reference::OutFreq> getOutFrequencies()
    {
        return {OutFreq::Off, OutFreq::MHZ10, OutFreq::MHZ100};
    }
};

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , manual(nullptr)
    , ui(new Ui::MainWindow)
    , server(nullptr)
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
    device = nullptr;
    modeHandler = nullptr;

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
    new ModeWindow(modeHandler, this);

    central = new QStackedWidget;
    setCentralWidget(central);

    auto vnaIndex = modeHandler->createMode("Vector Network Analyzer", Mode::Type::VNA);
    modeHandler->createMode("Signal Generator", Mode::Type::SG);
    modeHandler->createMode("Spectrum Analyzer", Mode::Type::SA);
    modeHandler->setCurrentIndex(vnaIndex);

    auto setModeStatusbar = [=](const QString &msg) {
        lModeInfo.setText(msg);
    };

    connect(modeHandler, &ModeHandler::StatusBarMessageChanged, setModeStatusbar);

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
        SaveSetup(filename);
    });
    connect(ui->actionLoad_setup, &QAction::triggered, [=](){
        auto filename = QFileDialog::getOpenFileName(nullptr, "Load setup data", "", "Setup files (*.setup)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        LoadSetup(filename);
    });
    connect(ui->actionSave_image, &QAction::triggered, [=](){
        modeHandler->getActiveMode()->saveSreenshot();
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

        // acquisition frequencies may have changed, update
        UpdateAcquisitionFrequencies();

        auto active = modeHandler->getActiveMode();
        if (active)
        {
            active->updateGraphColors();

            if(device) {
                active->initializeDevice();
            }
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

    auto pref = Preferences::getInstance();
    if(pref.Startup.UseSetupFile) {
        LoadSetup(pref.Startup.SetupFile);
    }
    // List available devices
    UpdateDeviceList();
    if(pref.Startup.ConnectToFirstDevice) {
        // at least one device available
        ConnectToDevice();
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

void AppWindow::closeEvent(QCloseEvent *event)
{
    auto pref = Preferences::getInstance();
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
    delete device;
    delete modeHandler;
    modeHandler = nullptr;
    pref.store();
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
        UpdateStatusBar(AppWindow::DeviceStatusBar::Connected);
        connect(device, &Device::LogLineReceived, &deviceLog, &DeviceLog::addLine);
        connect(device, &Device::ConnectionLost, this, &AppWindow::DeviceConnectionLost);
        connect(device, &Device::DeviceStatusUpdated, this, &AppWindow::DeviceStatusUpdated);
        connect(device, &Device::NeedsFirmwareUpdate, this, &AppWindow::DeviceNeedsUpdate);
        ui->actionDisconnect->setEnabled(true);
        ui->actionManual_Control->setEnabled(true);
        ui->actionFirmware_Update->setEnabled(true);
        ui->actionSource_Calibration->setEnabled(true);
        ui->actionReceiver_Calibration->setEnabled(true);
        ui->actionFrequency_Calibration->setEnabled(true);

        UpdateAcquisitionFrequencies();
        if (modeHandler->getActiveMode()) {
            modeHandler->getActiveMode()->initializeDevice();
        }
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
    UpdateStatusBar(DeviceStatusBar::Disconnected);
    if(modeHandler->getActiveMode()) {
        modeHandler->getActiveMode()->deviceDisconnected();
    }
    qDebug() << "Disconnected device";
}

void AppWindow::DeviceConnectionLost()
{
    DisconnectDevice();
    InformationBox::ShowError("Disconnected", "The USB connection to the device has been lost");
    UpdateDeviceList();
}

void AppWindow::CreateToolbars()
{
    // Reference toolbar
    auto tb_reference = new QToolBar("Reference", this);
    tb_reference->addWidget(new QLabel("Ref in:"));
    toolbars.reference.type = new QComboBox();
    for (auto r: Reference::getReferencesIn()) {
        toolbars.reference.type->addItem(Reference::TypeToLabel(r), (int)r);
    }
    tb_reference->addWidget(toolbars.reference.type);
    tb_reference->addSeparator();
    tb_reference->addWidget(new QLabel("Ref out:"));
    toolbars.reference.outFreq = new QComboBox();
    for (auto f: Reference::getOutFrequencies()) {

        toolbars.reference.outFreq->addItem(Reference::OutFreqToLabel(f), (int)f);
    }
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
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    scpi_dev->add(new SCPICommand("CONNect", [=](QStringList params) -> QString {
        QString serial;
        if(params.size() > 0) {
            serial = params[0];
        }
        if(!ConnectToDevice(serial)) {
            return "Device not found";
        } else {
            return SCPI::getResultName(SCPI::Result::Empty);
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
            return SCPI::getResultName(SCPI::Result::Error);
        } else if(params[0] == "0" || params[0] == "OFF") {
            int index = toolbars.reference.outFreq->findData((int)Reference::OutFreq::Off);
            toolbars.reference.outFreq->setCurrentIndex(index);
        } else if(params[0] == "10") {
            int index = toolbars.reference.outFreq->findData((int)Reference::OutFreq::MHZ10);
            toolbars.reference.outFreq->setCurrentIndex(index);
        } else if(params[0] == "100") {
            int index = toolbars.reference.outFreq->findData((int)Reference::OutFreq::MHZ100);
            toolbars.reference.outFreq->setCurrentIndex(index);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        Reference::OutFreq f = static_cast<Reference::OutFreq>(toolbars.reference.outFreq->currentData().toInt());
        switch(f) {
        case Reference::OutFreq::Off: return "OFF";
        case Reference::OutFreq::MHZ10: return "10";
        case Reference::OutFreq::MHZ100: return "100";
        default: return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_ref->add(new SCPICommand("IN", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else if(params[0] == "INT") {
            int index = toolbars.reference.type->findData((int)Reference::TypeIn::Internal);
            toolbars.reference.type->setCurrentIndex(index);
        } else if(params[0] == "EXT") {
            int index = toolbars.reference.type->findData((int)Reference::TypeIn::External);
            toolbars.reference.type->setCurrentIndex(index);
        } else if(params[0] == "AUTO") {
            int index = toolbars.reference.type->findData((int)Reference::TypeIn::Auto);
            toolbars.reference.type->setCurrentIndex(index);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        switch(Device::StatusV1(getDevice()).extRefInUse) {
        case 0: return "INT";
        case 1: return "EXT";
        default: return SCPI::getResultName(SCPI::Result::Error);
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
            case Mode::Type::SG: return "SG";
            case Mode::Type::SA: return "SA";
            }
        }
        return SCPI::getResultName(SCPI::Result::Error);
    }));
    auto scpi_status = new SCPINode("STAtus");
    scpi_dev->add(scpi_status);
    scpi_status->add(new SCPICommand("UNLOcked", nullptr, [=](QStringList){
        bool locked = Device::StatusV1(getDevice()).source_locked && Device::StatusV1(getDevice()).LO1_locked;
        return locked ? "FALSE" : "TRUE";
    }));
    scpi_status->add(new SCPICommand("ADCOVERload", nullptr, [=](QStringList){
        return Device::StatusV1(getDevice()).ADC_overload ? "TRUE" : "FALSE";
    }));
    scpi_status->add(new SCPICommand("UNLEVel", nullptr, [=](QStringList){
        return Device::StatusV1(getDevice()).unlevel ? "TRUE" : "FALSE";
    }));
    auto scpi_info = new SCPINode("INFo");
    scpi_dev->add(scpi_info);
    scpi_info->add(new SCPICommand("FWREVision", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).FW_major)+"."+QString::number(Device::Info(getDevice()).FW_minor)+"."+QString::number(Device::Info(getDevice()).FW_patch);
    }));
    scpi_info->add(new SCPICommand("HWREVision", nullptr, [=](QStringList){
        return QString(Device::Info(getDevice()).HW_Revision);
    }));
    scpi_info->add(new SCPICommand("TEMPeratures", nullptr, [=](QStringList){
        return QString::number(Device::StatusV1(getDevice()).temp_source)+"/"+QString::number(Device::StatusV1(getDevice()).temp_LO1)+"/"+QString::number(Device::StatusV1(getDevice()).temp_MCU);
    }));
    auto scpi_limits = new SCPINode("LIMits");
    scpi_info->add(scpi_limits);
    scpi_limits->add(new SCPICommand("MINFrequency", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_minFreq);
    }));
    scpi_limits->add(new SCPICommand("MAXFrequency", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_maxFreq);
    }));
    scpi_limits->add(new SCPICommand("MINIFBW", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_minIFBW);
    }));
    scpi_limits->add(new SCPICommand("MAXIFBW", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_maxIFBW);
    }));
    scpi_limits->add(new SCPICommand("MAXPoints", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_maxPoints);
    }));
    scpi_limits->add(new SCPICommand("MINPOWer", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_cdbm_min / 100.0);
    }));
    scpi_limits->add(new SCPICommand("MAXPOWer", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_cdbm_max / 100.0);
    }));
    scpi_limits->add(new SCPICommand("MINRBW", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_minRBW);
    }));
    scpi_limits->add(new SCPICommand("MAXRBW", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_maxRBW);
    }));
    scpi_limits->add(new SCPICommand("MAXHARMonicfrequency", nullptr, [=](QStringList){
        return QString::number(Device::Info(getDevice()).limits_maxFreqHarmonic);
    }));

    auto scpi_manual = new SCPINode("MANual");
    scpi_manual->add(new SCPICommand("STArt",[=](QStringList) -> QString {
        StartManualControl();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    scpi_manual->add(new SCPICommand("STOp",[=](QStringList) -> QString {
        manual->close();
        delete manual;
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));

    auto addBooleanManualSetting = [=](QString cmd, void(ManualControlDialog::*set)(bool), bool(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            bool enable;
            if(!manual || !SCPI::paramToBool(params, 0, enable)) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            set_fn(enable);
            return SCPI::getResultName(SCPI::Result::Empty);
        }, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto get_fn = std::bind(get, manual);
            return get_fn() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
        }));
    };

    auto addDoubleManualSetting = [=](QString cmd, void(ManualControlDialog::*set)(double), double(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            double value;
            if(!manual || !SCPI::paramToDouble(params, 0, value)) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            set_fn(value);
            return SCPI::getResultName(SCPI::Result::Empty);
        }, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addIntegerManualSetting = [=](QString cmd, void(ManualControlDialog::*set)(int), int(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            double value;
            if(!manual || !SCPI::paramToDouble(params, 0, value)) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            set_fn(value);
            return SCPI::getResultName(SCPI::Result::Empty);
        }, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addIntegerManualSettingWithReturnValue = [=](QString cmd, bool(ManualControlDialog::*set)(int), int(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, [=](QStringList params) -> QString {
            double value;
            if(!manual || !SCPI::paramToDouble(params, 0, value)) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto set_fn = std::bind(set, manual, std::placeholders::_1);
            if(set_fn(value)) {
                return SCPI::getResultName(SCPI::Result::Empty);
            } else {
                return SCPI::getResultName(SCPI::Result::Error);
            }
        }, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addIntegerManualQuery = [=](QString cmd, int(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addDoubleManualQuery = [=](QString cmd, double(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto get_fn = std::bind(get, manual);
            return QString::number(get_fn());
        }));
    };
    auto addBooleanManualQuery = [=](QString cmd, bool(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto get_fn = std::bind(get, manual);
            return get_fn() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
        }));
    };
    auto addComplexManualQuery = [=](QString cmd, std::complex<double>(ManualControlDialog::*get)(void)) {
        scpi_manual->add(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            if(!manual) {
                return SCPI::getResultName(SCPI::Result::Error);
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
            return SCPI::getResultName(SCPI::Result::Error);
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
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        if(!manual) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        auto lpf = manual->getHighSourceLPF();
        switch(lpf) {
        case ManualControlDialog::LPF::M947: return "947";
        case ManualControlDialog::LPF::M1880: return "1880";
        case ManualControlDialog::LPF::M3500: return "3500";
        case ManualControlDialog::LPF::None: return "0";
        default: return SCPI::getResultName(SCPI::Result::Error);
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
            return SCPI::getResultName(SCPI::Result::Error);
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
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        if(!manual) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        switch((ManualControlDialog::Window) manual->getWindow()) {
        case ManualControlDialog::Window::None: return "NONE";
        case ManualControlDialog::Window::Kaiser: return "KAISER";
        case ManualControlDialog::Window::Hann: return "HANN";
        case ManualControlDialog::Window::FlatTop: return "FLATTOP";
        default: return SCPI::getResultName(SCPI::Result::Error);
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

SCPI* AppWindow::getSCPI()
{
    return &scpi;
}

void AppWindow::setModeStatus(QString msg)
{
    lModeInfo.setText(msg);
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
            modeHandler->getActiveMode()->initializeDevice();
        }
    });
    if(AppWindow::showGUI()) {
        manual->show();
    }
}

void AppWindow::UpdateReference()
{
    if(!device) {
        // can't update without a device connected
        return;
    }
    Protocol::ReferenceSettings s = {};

    Reference::TypeIn t = static_cast<Reference::TypeIn>(toolbars.reference.type->currentData().toInt());
    switch (t) {
    case Reference::TypeIn::External: s.UseExternalRef = 1; break;
    case Reference::TypeIn::Auto: s.AutomaticSwitch = 1; break;
    default: break;
    }

    Reference::OutFreq f = static_cast<Reference::OutFreq>(toolbars.reference.outFreq->currentData().toInt());
    switch(f) {
    case Reference::OutFreq::MHZ10: s.ExtRefOuputFreq = 10000000; break;
    case Reference::OutFreq::MHZ100: s.ExtRefOuputFreq = 100000000; break;
    default: break;
    }

    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::Reference;
    p.reference = s;
    device->SendPacket(p);
}

void AppWindow::UpdateAcquisitionFrequencies()
{
    if(!device) {
        return;
    }
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::AcquisitionFrequencySettings;
    auto pref = Preferences::getInstance();
    p.acquisitionFrequencySettings.IF1 = pref.Acquisition.IF1;
    p.acquisitionFrequencySettings.ADCprescaler = pref.Acquisition.ADCprescaler;
    p.acquisitionFrequencySettings.DFTphaseInc = pref.Acquisition.DFTPhaseInc;
    device->SendPacket(p);
}

void AppWindow::StartFirmwareUpdateDialog()
{
    if(device) {
        auto fw_update = new FirmwareUpdateDialog(device);
        connect(fw_update, &FirmwareUpdateDialog::DeviceRebooting, this, &AppWindow::DisconnectDevice);
        connect(fw_update, &FirmwareUpdateDialog::DeviceRebooted, this, &AppWindow::ConnectToDevice);
        if(AppWindow::showGUI()) {
            fw_update->exec();
        }
    }
}

void AppWindow::DeviceNeedsUpdate(int reported, int expected)
{
    auto ret = InformationBox::AskQuestion("Warning",
                                "The device reports a different protocol"
                                "version (" + QString::number(reported) + ") than expected (" + QString::number(expected) + ").\n"
                                "A firmware update is strongly recommended. Do you want to update now?", false);
    if (ret) {
        StartFirmwareUpdateDialog();
    }
}

void AppWindow::DeviceStatusUpdated()
{
    UpdateStatusBar(DeviceStatusBar::Updated);
}

void AppWindow::SourceCalibrationDialog()
{
    auto d = new SourceCalDialog(device, modeHandler);
    if(AppWindow::showGUI()) {
        d->exec();
    }
}

void AppWindow::ReceiverCalibrationDialog()
{
    auto d = new ReceiverCalDialog(device, modeHandler);
    if(AppWindow::showGUI()) {
        d->exec();
    }
}

void AppWindow::FrequencyCalibrationDialog()
{
    auto d = new FrequencyCalDialog(device, modeHandler);
    if(AppWindow::showGUI()) {
        d->exec();
    }
}

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

    Reference::TypeIn t = static_cast<Reference::TypeIn>(toolbars.reference.type->currentData().toInt());
    ref["Mode"] = Reference::TypeToKey(t).toStdString();
    Reference::OutFreq f = static_cast<Reference::OutFreq>(toolbars.reference.outFreq->currentData().toInt());
    ref["Output"] =  Reference::OutFreqToKey(f).toStdString();
    j["Reference"] = ref;
    j["version"] = qlibrevnaApp->applicationVersion().toStdString();
    return j;
}

void AppWindow::LoadSetup(QString filename)
{
    ifstream file;
    file.open(filename.toStdString());
    if(!file.is_open()) {
        qWarning() << "Unable to open file:" << filename;
        return;
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (exception &e) {
        InformationBox::ShowError("Error", "Failed to parse the setup file (" + QString(e.what()) + ")");
        qWarning() << "Parsing of setup file failed: " << e.what();
        file.close();
        return;
    }
    file.close();
    LoadSetup(j);
    QFileInfo fi(filename);
    lSetupName.setText("Setup: "+fi.fileName());
}

void AppWindow::LoadSetup(nlohmann::json j)
{
//    auto d = new JSONPickerDialog(j);
//    d->exec();
    if(j.contains("Reference")) {
        QString fallback = Reference::TypeToKey(Reference::TypeIn::Internal);
        auto mode = QString::fromStdString(j["Reference"].value("Mode",fallback.toStdString()));
        auto index = toolbars.reference.type->findData((int)Reference::KeyToType(mode));
        toolbars.reference.type->setCurrentIndex(index);
        toolbars.reference.outFreq->setCurrentText(QString::fromStdString(j["Reference"].value("Output", "Off")));
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

Device *&AppWindow::getDevice()
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
        lConnectionStatus.setText("Connected to " + device->serial());
        qInfo() << "Connected to" << device->serial();
        lDeviceInfo.setText(device->getLastDeviceInfoString());
        break;
    case DeviceStatusBar::Disconnected:
        lConnectionStatus.setText("No device connected");
        lDeviceInfo.setText("No device information available yet");
        break;
    case DeviceStatusBar::Updated:
        lDeviceInfo.setText(device->getLastDeviceInfoString());
        lADCOverload.setVisible(device->StatusV1().ADC_overload);
        lUnlevel.setVisible(device->StatusV1().unlevel);
        lUnlock.setVisible(!device->StatusV1().LO1_locked || !device->StatusV1().source_locked);
        break;
    default:
        // invalid status
        break;
    }
}

