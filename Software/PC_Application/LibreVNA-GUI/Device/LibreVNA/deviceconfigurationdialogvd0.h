#ifndef DEVICECONFIGURATIONDIALOGVD0_H
#define DEVICECONFIGURATIONDIALOGVD0_H

#include <QDialog>

#include "librevnadriver.h"

namespace Ui {
class DeviceConfigurationDialogVD0;
}

class DeviceConfigurationDialogVD0 : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceConfigurationDialogVD0(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~DeviceConfigurationDialogVD0();

private:
    void updateGUI(const Protocol::DeviceConfig &c);
    void updateDevice();

    Ui::DeviceConfigurationDialogVD0 *ui;
    LibreVNADriver &dev;
};

#endif // DEVICECONFIGURATIONDIALOGVD0_H
