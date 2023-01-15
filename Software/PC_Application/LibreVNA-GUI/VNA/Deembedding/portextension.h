#ifndef PORTEXTENSION_H
#define PORTEXTENSION_H

#include "../../VNA_embedded/Application/Communication/Protocol.hpp"
#include "Calibration/calkit.h"
#include "deembeddingoption.h"

#include <QObject>
#include <QMessageBox>
#include <QToolBar>

namespace Ui {
class PortExtensionEditDialog;
}

class PortExtension : public DeembeddingOption
{
    Q_OBJECT
public:
    PortExtension();
    std::set<unsigned int> getAffectedPorts() override;
    void transformDatapoint(DeviceDriver::VNAMeasurement& d) override;
    void setCalkit(Calkit *kit);
    Type getType() override {return Type::PortExtension;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;
public slots:
    void edit() override;
    void measurementCompleted(std::vector<DeviceDriver::VNAMeasurement> m) override;

private:
    void startMeasurement();
    class Extension {
    public:
        double delay;
        double velocityFactor;
        double DCloss;
        double loss;
        double frequency;
    };
    Extension ext;

    // status variables for automatic measurements
    Calkit *kit;
    unsigned int port;
    bool isOpen;
    bool isIdeal;

    Ui::PortExtensionEditDialog *ui;
};

#endif // PORTEXTENSION_H
