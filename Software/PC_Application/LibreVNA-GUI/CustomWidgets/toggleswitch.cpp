#include "toggleswitch.h"

#include <QPainter>
#include <QMouseEvent>

ToggleSwitch::ToggleSwitch(QWidget *parent, bool state) : QAbstractButton(parent),
    _height(24),
    _width(128),
    state(state)
{

}

QSize ToggleSwitch::sizeHint() const
{
    return QSize(_width, _height);
}

void ToggleSwitch::toggle()
{
    state = !state;
    emit toggled(state);
}

void ToggleSwitch::setState(bool state)
{
    if(this->state != state) {
        this->state = state;
        emit toggled(state);
    }
}

void ToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.setOpacity(isEnabled() ? 0.38 : 0.12);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(QRect(0, 0, width(), height()), 8.0, 8.0);
    p.setOpacity(1.0);
    QRect rect;
    QString statename;
    if(state) {
        p.setBrush(isEnabled() ? Qt::darkGreen : Qt::gray);
        rect = QRect(width()/2, 0, width()/2, height());
        statename = "ON";
    } else {
        p.setBrush(isEnabled() ? QColor("#AA090E") : Qt::lightGray);
        rect = QRect(0, 0, width()/2, height());
        statename = "OFF";
    }
    p.drawRoundedRect(rect, 8.0, 8.0);
    QFont font = p.font();
    p.setPen(Qt::SolidLine);
    p.setPen(isEnabled() ? Qt::black : Qt::gray);
    p.drawText(rect, Qt::AlignCenter, statename);
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() & Qt::LeftButton) {
        toggle();
    }
    QAbstractButton::mouseReleaseEvent(e);
}
