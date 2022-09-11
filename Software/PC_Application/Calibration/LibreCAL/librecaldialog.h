#ifndef LIBRECALDIALOG_H
#define LIBRECALDIALOG_H

#include "Calibration/calibration.h"
#include "caldevice.h"

#include <QDialog>
#include <QTimer>
#include <QComboBox>

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
    void disableUI();
    void enableUI();
    void createPortAssignmentUI();
    Ui::LibreCALDialog *ui;
    Calibration *cal;
    CalDevice *device;
    CalDevice::CoefficientSet coeffSet;
    QTimer updateTimer;
    bool busy;
    std::vector<int> portAssignment;
    std::vector<QComboBox*> portAssignmentComboBoxes;

    int measurementsTaken;
};

#endif // LIBRECALDIALOG_H
