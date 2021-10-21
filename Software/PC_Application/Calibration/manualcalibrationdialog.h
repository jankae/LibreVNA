#ifndef MANUALCALIBRATIONDIALOG_H
#define MANUALCALIBRATIONDIALOG_H

#include "calibration.h"

#include <QDialog>

namespace Ui {
class ManualCalibrationDialog;
}

class ManualCalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualCalibrationDialog(const TraceModel &model, Calibration *cal);
    ~ManualCalibrationDialog();

private:
    Ui::ManualCalibrationDialog *ui;
};

#endif // MANUALCALIBRATIONDIALOG_H
