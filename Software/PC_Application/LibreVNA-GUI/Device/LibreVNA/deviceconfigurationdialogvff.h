#ifndef DEVICECONFIGURATIONDIALOGVFF_H
#define DEVICECONFIGURATIONDIALOGVFF_H

#include "librevnadriver.h"

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class DeviceConfigurationDialogVFF;
}

class DeviceConfigurationDialogVFF : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceConfigurationDialogVFF(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~DeviceConfigurationDialogVFF();

private:
    void updateGUI(const Protocol::DeviceConfig &c);
    void updateDevice();

    Ui::DeviceConfigurationDialogVFF *ui;
    LibreVNADriver &dev;

    QHostAddress ip, mask, gateway;
};

#endif // DEVICECONFIGURATIONDIALOGVFF_H
