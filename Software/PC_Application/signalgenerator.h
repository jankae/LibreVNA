#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <QWidget>
#include "Device/device.h"

namespace Ui {
class Signalgenerator;
}

class Signalgenerator : public QWidget
{
    Q_OBJECT

public:
    explicit Signalgenerator(QWidget *parent = nullptr);
    ~Signalgenerator();

signals:
    void NewManualState(Protocol::ManualStatus s);

private slots:
    void setLevel(double level);
private:
    void SettingsChanged();
    Ui::Signalgenerator *ui;
};

#endif // SIGNALGENERATOR_H
