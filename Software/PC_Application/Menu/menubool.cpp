#include "menubool.h"

#include <QLabel>

MenuBool::MenuBool(QString name, bool defaultValue)
{
    auto label = new QLabel(name, this);
    label->setAlignment(Qt::AlignCenter);
    layout.addWidget(label);
    sw = new ToggleSwitch(this, defaultValue);
    layout.addWidget(sw);
    setLayout(&layout);
    connect(sw, &ToggleSwitch::toggled, this, &MenuBool::valueChanged);
    sw->setFocusPolicy(Qt::NoFocus);
}

void MenuBool::setValue(bool value)
{
    sw->setState(value);
}

void MenuBool::userSelected()
{
    sw->toggle();
}

void MenuBool::mouseReleaseEvent(QMouseEvent *me)
{
    setFrameStyle(QFrame::Raised | QFrame::Panel);
    MenuItem::mouseReleaseEvent(me);
}

void MenuBool::mousePressEvent(QMouseEvent *)
{
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
}
