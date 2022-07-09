#ifndef CALIBRATIONTRACEDIALOG_H
#define CALIBRATIONTRACEDIALOG_H

#include "calibration.h"
#include "measurementmodel.h"
#include "Device/device.h"

#include <QDialog>

namespace Ui {
class CalibrationTraceDialog;
}

class CalibrationTraceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationTraceDialog(Calibration *cal, double f_min, double f_max, Calibration::Type type = Calibration::Type::None);
    ~CalibrationTraceDialog();

public slots:
    void measurementsComplete(std::set<Calibration::Measurement> m);
    void measurementComplete(Calibration::Measurement m);
signals:
    void triggerMeasurements(std::set<Calibration::Measurement> m);
    void applyCalibration(Calibration::Type type);
    void calibrationInvalidated();

private slots:
    void on_bDelete_clicked();
    void on_bMeasure_clicked();
    void on_bApply_clicked();

private:
    void UpdateCalibrationStatus();
    Ui::CalibrationTraceDialog *ui;
    Calibration *cal;
    Calibration::Type requestedType;
    std::vector<Calibration::Measurement> measurements;
    MeasurementModel *model;
};

#endif // CALIBRATIONTRACEDIALOG_H
