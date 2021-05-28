#include "rawregisterdialog.h"
#include "ui_rawregisterdialog.h"

#include "register.h"
#include "max2871.h"

#include <QPushButton>
#include <QFileDialog>
#include <QDebug>
#include <fstream>
#include <iomanip>

using namespace std;

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

    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getSaveFileName(this, "Save register settigns", "", "Raw register file (*.regs)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
            if(!filename.endsWith(".regs")) {
                filename.append(".regs");
            }
            nlohmann::json j;
            for(auto dev : devices) {
                nlohmann::json jdev;
                jdev["partnumber"] = dev->getPartnumber().toStdString();
                jdev["settings"] = dev->toJSON();
                j[dev->getName().toStdString()] = jdev;
            }
            ofstream file;
            file.open(filename.toStdString());
            file << setw(4) << j << endl;
            file.close();
        }
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getOpenFileName(this, "Load register settigns", "", "Raw register file (*.regs)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
            ifstream file;
            file.open(filename.toStdString());
            if(!file.is_open()) {
                throw runtime_error("Unable to open file");
            }

            nlohmann::json j;
            file >> j;

            for(auto jdev : j.items()) {
                auto name = QString::fromStdString(jdev.key());
                bool found = false;
                for(auto dev : devices) {
                    if(dev->getName() == name) {
                        // potential match
                        auto part = QString::fromStdString(jdev.value()["partnumber"]);
                        if(part == dev->getPartnumber()) {
                            dev->fromJSON(jdev.value()["settings"]);
                        } else {
                            qWarning() << "Got registers for device" << name <<", but partnumber does not match";
                        }
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    qWarning() << "Got registers for device" << name <<", but got no device with that name";
                }
            }
        }
    });
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
    auto regdev = RegisterDevice::create(dev, info.num, info.type, info.name);
    if(!regdev) {
        qWarning() << "Unable to create register device" << info.type <<", unknown type";
        return;
    }
    devices[info.num] = regdev;
    ui->tabs->addTab(regdev->getWidget(), QString(info.type)+": "+QString(info.name));

    if(info.num == devices.size() - 1) {
        // this was the last device, make outputs available as inputs for other devices
        for(auto outer : devices) {
            for(auto inner : devices) {
                if(inner != outer) {
                    inner->addPossibleInputs(outer);
                }
            }
        }
    }
}

void RawRegisterDialog::receivedDirectRegister(Protocol::DirectRegisterWrite reg)
{
    if(reg.device < devices.size() && devices[reg.device]) {
        devices[reg.device]->setRegister(reg.address, reg.data);
    }
}
