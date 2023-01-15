#ifndef IMPEDANCERENORMALIZATION_H
#define IMPEDANCERENORMALIZATION_H

#include "deembeddingoption.h"

namespace Ui {
class ImpedanceRenormalizationDialog;
}

class ImpedanceRenormalization : public DeembeddingOption
{
    Q_OBJECT
public:
    ImpedanceRenormalization();

    std::set<unsigned int> getAffectedPorts() override;
    void transformDatapoint(DeviceDriver::VNAMeasurement &p) override;
    Type getType() override { return Type::ImpedanceRenormalization;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;

public slots:
    virtual void edit() override;
private:
    double impedance;

    Ui::ImpedanceRenormalizationDialog *ui;
};

#endif // IMPEDANCERENORMALIZATION_H
