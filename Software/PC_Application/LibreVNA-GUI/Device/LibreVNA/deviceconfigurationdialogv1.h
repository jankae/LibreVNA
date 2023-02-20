#ifndef DEVICECONFIGURATIONDIALOGV1_H
#define DEVICECONFIGURATIONDIALOGV1_H

#include <QDialog>

#include "librevnadriver.h"

namespace Ui {
class DeviceConfigurationDialogV1;
}

class DeviceConfigurationDialogV1 : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceConfigurationDialogV1(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~DeviceConfigurationDialogV1();

private:
    void updateGUI(const Protocol::DeviceConfig &c);
    void updateDevice();

    Ui::DeviceConfigurationDialogV1 *ui;
    LibreVNADriver &dev;
};

#endif // DEVICECONFIGURATIONDIALOGV1_H
