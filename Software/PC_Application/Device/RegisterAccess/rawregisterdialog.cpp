#include "rawregisterdialog.h"
#include "ui_rawregisterdialog.h"

#include "register.h"
#include "max2871.h"

#include <QDebug>

RawRegisterDialog::RawRegisterDialog(Device *dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RawRegisterDialog),
    dev(dev)
{
    ui->setupUi(this);

    devices.resize(dev->Info().num_directRegisterDevices);

    connect(dev, &Device::ReceivedDirectRegisterInfo, this, &RawRegisterDialog::receivedDirectRegisterInfo);
    connect(dev, &Device::ReceivedDirectRegister, this, &RawRegisterDialog::receivedDirectRegister);

    // trigger extraction of device information, this will trigger the receivedDirectRegisterInfo slot which will further populate the dialog
    dev->SendCommandWithoutPayload(Protocol::PacketType::RequestDirectRegisterInfo);
}

RawRegisterDialog::~RawRegisterDialog()
{
    delete ui;
}

void RawRegisterDialog::receivedDirectRegisterInfo(Protocol::DirectRegisterInfo info)
{
    if(info.num >= devices.size()) {
        qWarning() << "Received invalid register device:" << info.num;
        return;
    }
    auto regdev = RegisterDevice::create(dev, info.num, info.type);
    if(!regdev) {
        qWarning() << "Unable to create register device" << info.type <<", unknown type";
        return;
    }
    devices[info.num] = regdev;
    ui->tabs->addTab(regdev->getWidget(), QString(info.type)+": "+QString(info.name));
}

void RawRegisterDialog::receivedDirectRegister(Protocol::DirectRegisterWrite reg)
{
    if(reg.device < devices.size() && devices[reg.device]) {
        devices[reg.device]->setRegister(reg.address, reg.data);
    }
}
