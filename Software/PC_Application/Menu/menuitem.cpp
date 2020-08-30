#include "menuitem.h"

#include <QMouseEvent>

MenuItem::MenuItem() : QFrame()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //setStyleSheet("*:focus {background: lightblue}");
    setFocusPolicy(Qt::StrongFocus);
    setFrameStyle(QFrame::Raised | QFrame::Panel);
}

void MenuItem::mouseReleaseEvent(QMouseEvent *me)
{
    userSelected();
    me->accept();
}
