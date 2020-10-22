#ifndef COLORPICKERBUTTON_H
#define COLORPICKERBUTTON_H

#include <QPushButton>

class ColorPickerButton : public QPushButton
{
    Q_OBJECT
public:
    ColorPickerButton(QWidget *parent = nullptr);

    void setColor(const QColor& color);
    const QColor& getColor();
signals:
    void colorChanged(const QColor& color);
private slots:
    void changeColor();
private:
    void updateBackground();
    QColor color;
};

#endif // COLORPICKERBUTTON_H
