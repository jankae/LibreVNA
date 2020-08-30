#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QAbstractButton>

class ToggleSwitch : public QAbstractButton
{
    Q_OBJECT
public:
    ToggleSwitch(QWidget* parent = nullptr, bool state = false);

    QSize sizeHint() const override;
signals:
    void toggled(bool newstate);
public slots:
    void toggle();
    void setState(bool state);

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    int _height, _width;
    bool state;
};

#endif // TOGGLESWITCH_H
