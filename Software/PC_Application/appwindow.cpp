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
#include <QDebug>

using namespace std;

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("VNA");
    QCoreApplication::setApplicationName("Application");

    qSetMessagePattern("%{time process}: [%{type}] %{message}");

    qDebug() << "Application start";

    Preferences::getInstance().load();
    device = nullptr;

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

    connect(ui->actionManual_Control, &QAction::triggered, this, &AppWindow::StartManualControl);
    connect(ui->actionFirmware_Update, &QAction::triggered, this, &AppWindow::StartFirmwareUpdateDialog);
    connect(ui->actionSource_Calibration, &QAction::triggered, this, &AppWindow::SourceCalibrationDialog);
    connect(ui->actionReceiver_Calibration, &QAction::triggered, this, &AppWindow::ReceiverCalibrationDialog);
    connect(ui->actionPreferences, &QAction::triggered, [=](){
        Preferences::getInstance().edit();
        // settings might have changed, update necessary stuff
//        TraceXYPlot::updateGraphColors();
    });
    connect(ui->actionAbout, &QAction::triggered, [=](){
        auto commit = QString(GITHASH);
        commit.truncate(7);
        QMessageBox::about(this, "About", "More information: github.com/jankae/VNA2\n"
                           "\nVersion: " + QString::number(FW_MAJOR) + "." + QString::number(FW_MINOR)
                           + "." + QString::number(FW_PATCH) + FW_SUFFIX + " ("+ commit+")");
    });

    setWindowTitle("VNA");

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    {
        QSettings settings;
        restoreGeometry(settings.value("geometry").toByteArray());
    }

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
}

AppWindow::~AppWindow()
{
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

void AppWindow::ConnectToDevice(QString serial)
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
    } catch (const runtime_error &e) {
        qWarning() << "Failed to connect:" << e.what();
        DisconnectDevice();
        UpdateDeviceList();
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

int AppWindow::UpdateDeviceList()
{
    deviceActionGroup->setExclusive(true);
    ui->menuConnect_to->clear();
    auto devices = Device::GetDevices();
    if(device) {
        devices.insert(device->serial());
    }
    if(devices.size()) {
        for(auto d : devices) {
            auto connectAction = ui->menuConnect_to->addAction(d);
            connectAction->setCheckable(true);
            connectAction->setActionGroup(deviceActionGroup);
            if(device && d == device->serial()) {
                connectAction->setChecked(true);
            }
            connect(connectAction, &QAction::triggered, [this, d]() {
               ConnectToDevice(d);
            });
        }
        ui->menuConnect_to->setEnabled(true);
    } else {
        // no devices available, disable connection option
        ui->menuConnect_to->setEnabled(false);
    }
    qDebug() << "Updated device list, found" << devices.size();
    return devices.size();
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
