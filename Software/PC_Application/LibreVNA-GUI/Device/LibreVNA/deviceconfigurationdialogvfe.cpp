#include "deviceconfigurationdialogvfe.h"
#include "ui_deviceconfigurationdialogvfe.h"

#include <QtEndian>

DeviceConfigurationDialogVFE::DeviceConfigurationDialogVFE(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceConfigurationDialogVFE),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p) {
        if(p.type == Protocol::PacketType::DeviceConfiguration) {
            updateGUI(p.deviceConfig);
        }
    });

    connect(ui->autogain, &QCheckBox::toggled, this, [=](){
       ui->portgain->setEnabled(!ui->autogain->isChecked());
       ui->refgain->setEnabled(!ui->autogain->isChecked());
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

DeviceConfigurationDialogVFE::~DeviceConfigurationDialogVFE()
{
    dev.releaseControl();
    delete ui;
}

void DeviceConfigurationDialogVFE::updateGUI(const Protocol::DeviceConfig &c)
{
    ui->autogain->setChecked(c.VFE.autogain);
    ui->portgain->setCurrentIndex(c.VFE.portGain);
    ui->refgain->setCurrentIndex(c.VFE.refGain);
}

void DeviceConfigurationDialogVFE::updateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::DeviceConfiguration;

    p.deviceConfig.VFE.autogain = ui->autogain->isChecked() ? 1 : 0;
    p.deviceConfig.VFE.portGain = ui->portgain->currentIndex();
    p.deviceConfig.VFE.refGain = ui->refgain->currentIndex();
    dev.SendPacket(p);
}
