#ifndef LIBRECALDIALOG_H
#define LIBRECALDIALOG_H

#include "Calibration/calibration.h"
#include "caldevice.h"

#include <QDialog>
#include <QTimer>

namespace Ui {
class LibreCALDialog;
}

class LibreCALDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LibreCALDialog(Calibration *cal);
    ~LibreCALDialog();

private:
signals:
    void portAssignmentChanged();
private slots:
    void updateCalibrationStartStatus();
    void updateDeviceStatus();
    void startCalibration();
private:
    void createPortAssignmentUI();
    Ui::LibreCALDialog *ui;
    Calibration *cal;
    CalDevice *device;
    CalDevice::CoefficientSet coeffSet;
    QTimer updateTimer;
    bool busy;
    std::vector<int> portAssignment;
};

#endif // LIBRECALDIALOG_H
