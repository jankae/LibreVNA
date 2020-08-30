#include "menuvalue.h"

#include <QVBoxLayout>
#include <math.h>
#include <sstream>
#include <iomanip>
#include "valueinput.h"
#include <QMouseEvent>
#include "unit.h"
#include <QDoubleValidator>

using namespace std;

MenuValue::MenuValue(QString name, double defaultValue, QString unit, QString prefixes, int precision)
    : name(name)
{
    if(prefixes.indexOf(' ') < 0) {
        throw runtime_error("Prefix string must contain space");
    }
    auto layout = new QVBoxLayout;
    auto label = new QLabel(name, this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    lvalue = new SIUnitEdit(unit, prefixes, precision);
    // pass on signal
    connect(lvalue, &SIUnitEdit::valueChanged, this, &MenuValue::valueChanged);
    layout->addWidget(lvalue);
    setValue(defaultValue);
    setLayout(layout);
}

void MenuValue::setValue(double value)
{
    lvalue->setValue(value);
}

void MenuValue::setValueQuiet(double value)
{
    lvalue->setValueQuiet(value);
}

void MenuValue::userSelected()
{
    lvalue->setFocus();
    //startInputDialog();
}
