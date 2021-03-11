#ifndef MANUALCALIBRATIONDIALOG_H
#define MANUALCALIBRATIONDIALOG_H

#include <QDialog>
#include "calibration.h"

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
