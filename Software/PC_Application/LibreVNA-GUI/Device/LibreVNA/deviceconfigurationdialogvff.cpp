#include "deviceconfigurationdialogvff.h"
#include "ui_deviceconfigurationdialogvff.h"

#include <QtEndian>

DeviceConfigurationDialogVFF::DeviceConfigurationDialogVFF(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceConfigurationDialogVFF),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    auto updateAddress = [](QLineEdit* line, QHostAddress &address) {
        auto newAddress = QHostAddress(line->text());
        if(newAddress.isNull()) {
            // address is invalid, set edit to old address
            line->setText(address.toString());
        } else {
            // input is valid, store new address
            address = newAddress;
        }
    };

    connect(ui->ip, &QLineEdit::editingFinished, this, [=](){
       updateAddress(ui->ip, ip);
    });
    connect(ui->mask, &QLineEdit::editingFinished, this, [=](){
       updateAddress(ui->mask, mask);
    });
    connect(ui->gateway, &QLineEdit::editingFinished, this, [=](){
       updateAddress(ui->gateway, gateway);
    });

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

DeviceConfigurationDialogVFF::~DeviceConfigurationDialogVFF()
{
    dev.releaseControl();
    delete ui;
}

void DeviceConfigurationDialogVFF::updateGUI(const Protocol::DeviceConfig &c)
{
    ui->dhcp->setChecked(c.VFF.dhcp);
    ip = QHostAddress(qFromBigEndian(c.VFF.ip));
    ui->ip->setText(ip.toString());
    mask = QHostAddress(qFromBigEndian(c.VFF.mask));
    ui->mask->setText(mask.toString());
    gateway = QHostAddress(qFromBigEndian(c.VFF.gw));
    ui->gateway->setText(gateway.toString());

    ui->autogain->setChecked(c.VFF.autogain);
    ui->portgain->setCurrentIndex(c.VFF.portGain);
    ui->refgain->setCurrentIndex(c.VFF.refGain);
}

void DeviceConfigurationDialogVFF::updateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::DeviceConfiguration;
    p.deviceConfig.VFF.dhcp = ui->dhcp->isChecked() ? 1 : 0;
    p.deviceConfig.VFF.ip = qToBigEndian(ip.toIPv4Address());
    p.deviceConfig.VFF.mask = qToBigEndian(mask.toIPv4Address());
    p.deviceConfig.VFF.gw = qToBigEndian(gateway.toIPv4Address());

    p.deviceConfig.VFF.autogain = ui->autogain->isChecked() ? 1 : 0;
    p.deviceConfig.VFF.portGain = ui->portgain->currentIndex();
    p.deviceConfig.VFF.refGain = ui->refgain->currentIndex();
    dev.SendPacket(p);
}
