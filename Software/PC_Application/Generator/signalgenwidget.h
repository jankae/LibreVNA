#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include "Device/virtualdevice.h"
#include "savable.h"

#include <QWidget>

namespace Ui {
class SignalgeneratorWidget;
}

class SignalgeneratorWidget : public QWidget, public Savable
{
    Q_OBJECT

public:
    explicit SignalgeneratorWidget(VirtualDevice *dev, QWidget *parent = nullptr);
    ~SignalgeneratorWidget();

    VirtualDevice::SGSettings getDeviceStatus();
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

signals:
    void SettingsChanged();

public slots:
    void setLevel(double level);
    void setFrequency(double frequency);
    void setPort(int port);

protected:
    void timerEvent(QTimerEvent *) override;

private:
    Ui::SignalgeneratorWidget *ui;
    int m_timerId;
    VirtualDevice *dev;
};

#endif // SIGNALGENERATOR_H
