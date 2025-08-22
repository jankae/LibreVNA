#ifndef DEVICECONFIGURATIONDIALOGVFD_H
#define DEVICECONFIGURATIONDIALOGVFD_H

#include "librevnadriver.h"

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class DeviceConfigurationDialogVFD;
}

class DeviceConfigurationDialogVFD : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceConfigurationDialogVFD(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~DeviceConfigurationDialogVFD();

private:
    void updateGUI(const Protocol::DeviceConfig &c);
    void updateDevice();

    Ui::DeviceConfigurationDialogVFD *ui;
    LibreVNADriver &dev;

    QHostAddress ip, mask, gateway;
};

#endif // DEVICECONFIGURATIONDIALOGVFD_H
