#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include "savable.h"
#include "appwindow.h"

#include <QWidget>

namespace Ui {
class SignalgeneratorWidget;
}

class SignalgeneratorWidget : public QWidget, public Savable
{
    Q_OBJECT

public:
    explicit SignalgeneratorWidget(AppWindow *window, QWidget *parent = nullptr);
    ~SignalgeneratorWidget();

    DeviceDriver::SGSettings getDeviceStatus();
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void deviceInfoUpdated();
signals:
    void SettingsChanged();

public slots:
    void setLevel(double level);
    void setFrequency(double frequency);
    void setPort(unsigned int port);

protected:
    void timerEvent(QTimerEvent *) override;

private:
    Ui::SignalgeneratorWidget *ui;
    int m_timerId;
    AppWindow *window;
    std::vector<QCheckBox*> portCheckboxes;
};

#endif // SIGNALGENERATOR_H
