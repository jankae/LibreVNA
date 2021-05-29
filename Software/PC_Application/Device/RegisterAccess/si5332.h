#ifndef SI5332_H
#define SI5332_H

#include "registerdevice.h"
#include <QWidget>
#include "CustomWidgets/siunitedit.h"

namespace Ui {
class SI5332Widget;
}

class SI5332 : public RegisterDevice
{
public:
    SI5332();
    ~SI5332();

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

    void addPossibleInputs(RegisterDevice *inputDevice) override;
private:
    SIUnitEdit *currentInput;
    SIUnitEdit *currentXTAL;
    Ui::SI5332Widget *ui;
};

#endif // MAX2871_H
