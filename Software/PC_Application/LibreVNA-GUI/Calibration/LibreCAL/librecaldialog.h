#ifndef LIBRECALDIALOG_H
#define LIBRECALDIALOG_H

#include "Calibration/calibration.h"
#include "caldevice.h"
#include "Device/devicedriver.h"

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
    void autoPortComplete();
private slots:
    bool validatePortSelection(bool autoAllowed);
    bool validateCoefficients();
    void updateCalibrationStartStatus();
    void updateDeviceStatus();
    void determineAutoPorts();
    void loadCoefficients();
    void startCalibration();

    // auto port slots
    void handleIncomingMeasurement(DeviceDriver::VNAMeasurement m);
    void startSweep();
    void stopSweep();
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
    // 0 for unused port, -1 for auto port, otherwise the port number
    std::vector<int> portAssignment;
    std::vector<QComboBox*> portAssignmentComboBoxes;
    std::vector<DeviceDriver::VNAMeasurement> autoPortMeasurements;
    DeviceDriver *driver;

    int measurementsTaken;
};

#endif // LIBRECALDIALOG_H
