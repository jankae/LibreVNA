#ifndef AD9913_H
#define AD9913_H

#include "registerdevice.h"
#include <QWidget>
#include "CustomWidgets/siunitedit.h"

namespace Ui {
class AD9913Widget;
}

class AD9913 : public RegisterDevice
{
public:
    AD9913();
    ~AD9913();

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

    void addPossibleInputs(RegisterDevice *inputDevice) override;
private:
    SIUnitEdit *currentInput;
    Ui::AD9913Widget *ui;
};

#endif // MAX2871_H
