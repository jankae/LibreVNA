#ifndef REGISTERDEVICE_H
#define REGISTERDEVICE_H

#include <QString>

#include "register.h"
#include "savable.h"
#include "Device/device.h"
#include "CustomWidgets/siunitedit.h"

class RegisterDevice : public Savable
{
public:
    static RegisterDevice *create(Device *dev, int number, QString partnumber, QString name);
    ~RegisterDevice();

    void setRegister(int address, unsigned long long value);

    QWidget *getWidget() const;
    QString getPartnumber() const;
    QString getName() const;

    virtual void addPossibleInputs(RegisterDevice *inputDevice);

protected:
    void addRegister(Register *reg);

    nlohmann::json registersToJSON();
    void registersFromJSON(nlohmann::json j);

    RegisterDevice();
    Device *dev;

    int number;
    QString partnumber;
    QString name;
    std::vector<Register*> regs;
    QWidget *widget;
    std::map<QString, SIUnitEdit*> outputs;
    std::map<QString, SIUnitEdit*> possibleInputs;
};

#endif // REGISTERDEVICE_H
