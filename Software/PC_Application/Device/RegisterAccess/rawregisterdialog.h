#ifndef REGISTERTESTDIALOG_H
#define REGISTERTESTDIALOG_H

#include <QDialog>
#include "Device/device.h"
#include "registerdevice.h"

namespace Ui {
class RawRegisterDialog;
}

class RawRegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RawRegisterDialog(Device *dev, QWidget *parent = nullptr);
    ~RawRegisterDialog();

private slots:
    void receivedDirectRegisterInfo(Protocol::DirectRegisterInfo info);
    void receivedDirectRegister(Protocol::DirectRegisterWrite reg);
private:
    Ui::RawRegisterDialog *ui;
    Device *dev;
    std::vector<RegisterDevice*> devices;
};

#endif // REGISTERTESTDIALOG_H
