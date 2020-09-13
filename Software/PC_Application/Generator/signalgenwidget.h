#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <QWidget>
#include "Device/device.h"

namespace Ui {
class SignalgeneratorWidget;
}

class SignalgeneratorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SignalgeneratorWidget(QWidget *parent = nullptr);
    ~SignalgeneratorWidget();

    Protocol::ManualControl getDeviceStatus();

signals:
    void SettingsChanged();

private slots:
    void setLevel(double level);
private:
    Ui::SignalgeneratorWidget *ui;
};

#endif // SIGNALGENERATOR_H
