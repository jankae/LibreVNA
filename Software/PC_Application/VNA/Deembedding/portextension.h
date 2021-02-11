#ifndef PORTEXTENSION_H
#define PORTEXTENSION_H

#include <QObject>
#include "../VNA_embedded/Application/Communication/Protocol.hpp"
#include <QToolBar>
#include "Calibration/calkit.h"
#include <QMessageBox>
#include "deembeddingoption.h"

namespace Ui {
class PortExtensionEditDialog;
}

class PortExtension : public DeembeddingOption
{
    Q_OBJECT
public:
    PortExtension();
    void transformDatapoint(Protocol::Datapoint& d) override;
    void setCalkit(Calkit *kit);
    Type getType() override {return Type::PortExtension;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;
public slots:
    void edit() override;
    void measurementCompleted(std::vector<Protocol::Datapoint> m) override;

private:
    void startMeasurement();
    class Extension {
    public:
        bool enabled;
        double delay;
        double velocityFactor;
        double DCloss;
        double loss;
        double frequency;
    };
    Extension port1, port2;

    // status variables for automatic measurements
    Calkit *kit;
//    bool measuring;
    bool isPort1;
    bool isOpen;
    bool isIdeal;
//    std::vector<Protocol::Datapoint> measurements;
    QMessageBox *msgBox;
    Ui::PortExtensionEditDialog *ui;
};

#endif // PORTEXTENSION_H
