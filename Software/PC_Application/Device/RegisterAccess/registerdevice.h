#ifndef REGISTERDEVICE_H
#define REGISTERDEVICE_H

#include <QString>

#include "register.h"
#include "savable.h"
#include "Device/device.h"

class RegisterDevice //: public Savable
{
public:
    static RegisterDevice *create(Device *dev, int number, QString partnumber);
    ~RegisterDevice();

    void setRegister(int address, unsigned long value);
//    void fromJSON(nlohmann::json j) override;
//    nlohmann::json toJSON() override;

    QWidget *getWidget() const;

protected:
    void addRegister(Register *reg);

    RegisterDevice();
    Device *dev;

    int number;
    QString partnumber;
    std::vector<Register*> regs;
    QWidget *widget;
};

#endif // REGISTERDEVICE_H
