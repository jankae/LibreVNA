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
#include "Traces/tracebodeplot.h"
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

using namespace std;

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , deviceActionGroup(new QActionGroup(this))
    , ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("VNA");
    QCoreApplication::setApplicationName("Application");

    pref.load();
    device = nullptr;

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
    auto logDock = new QDockWidget("Device Log");
    logDock->setWidget(&deviceLog);
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
    auto vna = new VNA(this);
    new Generator(this);
//    auto signalGenWidget = new Signalgenerator;
//    modeSGen = new GUIMode(this, "Signal Generator", signalGenWidget);

    // UI connections
    connect(ui->actionUpdate_Device_List, &QAction::triggered, this, &AppWindow::UpdateDeviceList);
    connect(ui->actionDisconnect, &QAction::triggered, this, &AppWindow::DisconnectDevice);
    connect(ui->actionQuit, &QAction::triggered, this, &AppWindow::close);
    connect(ui->actionManual_Control, &QAction::triggered, this, &AppWindow::StartManualControl);
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

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

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
    vna->activate();

    qRegisterMetaType<Protocol::Datapoint>("Datapoint");

    // List available devices
    if(UpdateDeviceList() && pref.Startup.ConnectToFirstDevice) {
        // at least one device available
        ConnectToDevice();
    }
}

void AppWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    // deactivate currently used mode (stores mode state in settings)
    if(Mode::getActiveMode()) {
        Mode::getActiveMode()->deactivate();
    }
    pref.store();
    QMainWindow::closeEvent(event);
}

void AppWindow::ConnectToDevice(QString serial)
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
        connect(device, &Device::LogLineReceived, &deviceLog, &DeviceLog::addLine);
        connect(device, &Device::ConnectionLost, this, &AppWindow::DeviceConnectionLost);
        connect(device, &Device::DeviceInfoUpdated, [this]() {
           lDeviceInfo.setText(device->getLastDeviceInfoString());
        });
        ui->actionDisconnect->setEnabled(true);
        ui->actionManual_Control->setEnabled(true);
        ui->actionFirmware_Update->setEnabled(true);

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
    } catch (const runtime_error e) {
        DisconnectDevice();
        UpdateDeviceList();
    }
}

void AppWindow::DisconnectDevice()
{
    if(device) {
        delete device;
        device = nullptr;
    }
    ui->actionDisconnect->setEnabled(false);
    ui->actionManual_Control->setEnabled(false);
    ui->actionFirmware_Update->setEnabled(false);
    if(deviceActionGroup->checkedAction()) {
        deviceActionGroup->checkedAction()->setChecked(false);
    }
    lConnectionStatus.setText("No device connected");
    lDeviceInfo.setText("No device information available yet");
    Mode::getActiveMode()->deviceDisconnected();
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
    connect(toolbars.reference.type, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppWindow::UpdateReference);
    connect(toolbars.reference.outFreq, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppWindow::UpdateReference);
    connect(toolbars.reference.outputEnabled, &QCheckBox::clicked, this, &AppWindow::UpdateReference);

    addToolBar(tb_reference);
}

int AppWindow::UpdateDeviceList()
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

void AppWindow::StartManualControl()
{
    auto control = new ManualControlDialog(*device, this);
    connect(control, &QDialog::finished, [=](){
        Mode::getActiveMode()->initializeDevice();
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
