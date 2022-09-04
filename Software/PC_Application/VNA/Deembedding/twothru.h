#ifndef TWOTHRU_H
#define TWOTHRU_H

#include "deembeddingoption.h"
#include "Tools/parameters.h"

#include <complex>
#include <QMessageBox>

namespace Ui {
class TwoThruDialog;
}

class TwoThru : public DeembeddingOption
{
public:
    TwoThru();

    virtual void transformDatapoint(VirtualDevice::VNAMeasurement& p) override;
    virtual void edit() override;
    virtual Type getType() override {return DeembeddingOption::Type::TwoThru;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;

private slots:
    void startMeasurement();
    void updateGUI();
    void measurementCompleted(std::vector<VirtualDevice::VNAMeasurement> m) override;
private:
    using Point = struct {
        double freq;
        Tparam inverseP1, inverseP2;
    };

    static std::vector<VirtualDevice::VNAMeasurement> interpolateEvenFrequencySteps(std::vector<VirtualDevice::VNAMeasurement> input);
    std::vector<Point> calculateErrorBoxes(std::vector<VirtualDevice::VNAMeasurement> data_2xthru);
    std::vector<Point> calculateErrorBoxes(std::vector<VirtualDevice::VNAMeasurement> data_2xthru, std::vector<VirtualDevice::VNAMeasurement> data_fix_dut_fix, double z0);

    std::vector<VirtualDevice::VNAMeasurement> measurements2xthru;
    std::vector<VirtualDevice::VNAMeasurement> measurementsDUT;
    double Z0;
    int port1, port2;
    std::vector<Point> points;
    bool measuring2xthru;
    bool measuringDUT;
    Ui::TwoThruDialog *ui;
};

#endif // TWOTHRU_H
