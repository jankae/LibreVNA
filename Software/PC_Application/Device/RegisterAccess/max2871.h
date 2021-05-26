#ifndef MAX2871_H
#define MAX2871_H

#include "registerdevice.h"
#include <QWidget>

namespace Ui {
class MAX2871Widget;
}

class MAX2871 : public RegisterDevice
{
public:
    MAX2871();
    ~MAX2871();

private:
    Ui::MAX2871Widget *ui;
};

#endif // MAX2871_H
