#include "registerdevice.h"

#include "max2871.h"
#include "Device/device.h"

RegisterDevice *RegisterDevice::create(Device *dev, int number, QString partnumber)
{
    RegisterDevice *regdev = nullptr;
    if(partnumber == "MAX2871") {
        regdev = new MAX2871();
    }
    if(regdev) {
        regdev->dev = dev;
        regdev->number = number;
        regdev->partnumber = partnumber;

        // read initial register content
        Protocol::PacketInfo p;
        p.type = Protocol::PacketType::DirectRegisterRead;
        p.directRegRead.device = number;
        for(unsigned int i=0;i<regdev->regs.size();i++) {
            p.directRegRead.address = regdev->regs[i]->getAddress();
            dev->SendPacket(p);
        }
    }
    return regdev;
}

RegisterDevice::~RegisterDevice()
{
    delete widget;
}

void RegisterDevice::setRegister(int address, unsigned long value)
{
    for(auto reg : regs) {
        if(reg->getAddress() == address) {
            reg->setValue(value);
        }
    }
}

RegisterDevice::RegisterDevice()
{
    widget = new QWidget;
}

QWidget *RegisterDevice::getWidget() const
{
    return widget;
}

void RegisterDevice::addRegister(Register *reg)
{
    regs.push_back(reg);
    QObject::connect(reg, &Register::valueChanged, [=]() {
        // transfer register content to device
        Protocol::PacketInfo p;
        p.type = Protocol::PacketType::DirectRegisterWrite;
        p.directRegWrite.device = number;
        p.directRegWrite.address = reg->getAddress();
        p.directRegWrite.data = reg->getValue();
        dev->SendPacket(p);
    });
}

