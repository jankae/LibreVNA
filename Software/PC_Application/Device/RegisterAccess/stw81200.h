#ifndef STW81200_H
#define STW81200_H

#include "registerdevice.h"
#include <QWidget>
#include "CustomWidgets/siunitedit.h"

namespace Ui {
class STW81200Widget;
}

class STW81200 : public RegisterDevice
{
public:
    STW81200();
    ~STW81200();

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

    void addPossibleInputs(RegisterDevice *inputDevice) override;
private:
    SIUnitEdit *currentInput;
    Ui::STW81200Widget *ui;
};

#endif // MAX2871_H
