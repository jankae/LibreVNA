#include "deviceconfigurationdialogvfd.h"
#include "ui_deviceconfigurationdialogvfd.h"

#include <QtEndian>

DeviceConfigurationDialogVFD::DeviceConfigurationDialogVFD(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceConfigurationDialogVFD),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    ui->IF->setPrecision(8);
    ui->IF->setUnit("Hz");
    ui->IF->setPrefixes(" kMG");

    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p) {
        if(p.type == Protocol::PacketType::DeviceConfiguration) {
            updateGUI(p.deviceConfig);
        }
    });

    dev.sendWithoutPayload(Protocol::PacketType::RequestDeviceConfiguration);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){
        updateDevice();
        accept();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [=](){
        reject();
    });
}

DeviceConfigurationDialogVFD::~DeviceConfigurationDialogVFD()
{
    dev.releaseControl();
    delete ui;
}

void DeviceConfigurationDialogVFD::updateGUI(const Protocol::DeviceConfig &c)
{
    ui->IF->setValue(c.VFD.IF);
    if(c.VFD.P1PortGain == 0xFF) {
        ui->P1PortGain->setCurrentIndex(0);
    } else if(c.VFD.P1PortGain < 64) {
        ui->P1PortGain->setCurrentIndex(64 - c.VFD.P1PortGain);
    }
    if(c.VFD.P1RefGain == 0xFF) {
        ui->P1RefGain->setCurrentIndex(0);
    } else if(c.VFD.P1RefGain < 64) {
        ui->P1RefGain->setCurrentIndex(64 - c.VFD.P1RefGain);
    }
    if(c.VFD.P2PortGain == 0xFF) {
        ui->P2PortGain->setCurrentIndex(0);
    } else if(c.VFD.P2PortGain < 64) {
        ui->P2PortGain->setCurrentIndex(64 - c.VFD.P2PortGain);
    }
    if(c.VFD.P2RefGain == 0xFF) {
        ui->P2RefGain->setCurrentIndex(0);
    } else if(c.VFD.P2RefGain < 64) {
        ui->P2RefGain->setCurrentIndex(64 - c.VFD.P2RefGain);
    }
}

void DeviceConfigurationDialogVFD::updateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::DeviceConfiguration;

    p.deviceConfig.VFD.IF = ui->IF->value();
    p.deviceConfig.VFD.P1PortGain = ui->P1PortGain->currentIndex() == 0 ? 0xFF : 64 - ui->P1PortGain->currentIndex();
    p.deviceConfig.VFD.P1RefGain = ui->P1RefGain->currentIndex() == 0 ? 0xFF : 64 - ui->P1RefGain->currentIndex();
    p.deviceConfig.VFD.P2PortGain = ui->P2PortGain->currentIndex() == 0 ? 0xFF : 64 - ui->P2PortGain->currentIndex();
    p.deviceConfig.VFD.P2RefGain = ui->P2RefGain->currentIndex() == 0 ? 0xFF : 64 - ui->P2RefGain->currentIndex();
    dev.SendPacket(p);
}
