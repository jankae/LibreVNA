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
    void updateLabel();
private:
    using Point = struct {
        double freq;
        Tparam inverseP1, inverseP2;
    };
    std::vector<Protocol::Datapoint> measurements;
    std::vector<Point> points;
    bool measuring;
    QMessageBox *msgBox;
    Ui::TwoThruDialog *ui;
};

#endif // TWOTHRU_H
