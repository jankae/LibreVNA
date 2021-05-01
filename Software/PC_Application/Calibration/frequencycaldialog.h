#ifndef FREQUENCYCALDIALOG_H
#define FREQUENCYCALDIALOG_H

#include <QDialog>
#include "Device/device.h"
#include "mode.h"

namespace Ui {
class FrequencyCalDialog;
}

class FrequencyCalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FrequencyCalDialog(Device *dev, QWidget *parent = nullptr);
    ~FrequencyCalDialog();

private:
    Ui::FrequencyCalDialog *ui;
    Device *dev;
};

#endif // FREQUENCYCALDIALOG_H
