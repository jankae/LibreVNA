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

    virtual void transformDatapoint(Protocol::Datapoint &p) override;
    virtual void edit() override;
    virtual Type getType() override {return DeembeddingOption::Type::TwoThru;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;

private slots:
    void startMeasurement();
    void updateGUI();
    void measurementCompleted(std::vector<Protocol::Datapoint> m) override;
private:
    using Point = struct {
        double freq;
        Tparam inverseP1, inverseP2;
    };

    static std::vector<Protocol::Datapoint> interpolateEvenFrequencySteps(std::vector<Protocol::Datapoint> input);
    static std::vector<Point> calculateErrorBoxes(std::vector<Protocol::Datapoint> data_2xthru);
    static std::vector<Point> calculateErrorBoxes(std::vector<Protocol::Datapoint> data_2xthru, std::vector<Protocol::Datapoint> data_fix_dut_fix, double z0);

    std::vector<Protocol::Datapoint> measurements2xthru;
    std::vector<Protocol::Datapoint> measurementsDUT;
    double Z0;
    std::vector<Point> points;
    bool measuring2xthru;
    bool measuringDUT;
    QMessageBox *msgBox;
    Ui::TwoThruDialog *ui;
};

#endif // TWOTHRU_H
