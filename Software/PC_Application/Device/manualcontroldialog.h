#ifndef MANUALCONTROLDIALOG_H
#define MANUALCONTROLDIALOG_H

#include <QDialog>
#include "device.h"

namespace Ui {
class ManualControlDialog;
}

class ManualControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialog(Device &dev, QWidget *parent = nullptr);
    ~ManualControlDialog();

public slots:
    void NewStatus(Protocol::ManualStatus status);

private:
    void UpdateDevice();
    Ui::ManualControlDialog *ui;
    Device &dev;
};

#endif // MANUALCONTROLDIALOG_H
