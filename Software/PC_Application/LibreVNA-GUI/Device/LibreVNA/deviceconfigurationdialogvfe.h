#ifndef DEVICECONFIGURATIONDIALOGVFE_H
#define DEVICECONFIGURATIONDIALOGVFE_H

#include "librevnadriver.h"

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class DeviceConfigurationDialogVFE;
}

class DeviceConfigurationDialogVFE : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceConfigurationDialogVFE(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~DeviceConfigurationDialogVFE();

private:
    void updateGUI(const Protocol::DeviceConfig &c);
    void updateDevice();

    Ui::DeviceConfigurationDialogVFE *ui;
    LibreVNADriver &dev;

    QHostAddress ip, mask, gateway;
};

#endif // DEVICECONFIGURATIONDIALOGVFE_H
