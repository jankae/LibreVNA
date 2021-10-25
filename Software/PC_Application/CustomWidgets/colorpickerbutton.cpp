#include "colorpickerbutton.h"

#include <QColorDialog>

ColorPickerButton::ColorPickerButton(QWidget *parent)
    : QPushButton(parent)
{
    color = Qt::white;
    connect(this, &ColorPickerButton::clicked, this, &ColorPickerButton::changeColor);
    updateBackground();
}

void ColorPickerButton::setColor(const QColor &color)
{
    this->color = color;
    updateBackground();
}

const QColor &ColorPickerButton::getColor()
{
    return color;
}

void ColorPickerButton::changeColor()
{
    auto newColor = QColorDialog::getColor(color, parentWidget(), "Select color", QColorDialog::DontUseNativeDialog);
    if(newColor.isValid() && newColor != color) {
        setColor(newColor);
        emit colorChanged(newColor);
    }
}

void ColorPickerButton::updateBackground()
{
    setStyleSheet("background-color:"+color.name());
}
