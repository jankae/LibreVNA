#ifndef FIRMWAREUPDATEDIALOG_H
#define FIRMWAREUPDATEDIALOG_H

#include "device.h"

#include <QDialog>
#include <QFile>
#include <QTimer>

namespace Ui {
class FirmwareUpdateDialog;
}

class FirmwareUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    /*
     * Depending on the result of the firmware update, the device pointer will be modified:
     * - In case of user-aborted firmware update, the device pointer will be unchanged
     * - If the update fails during transmission of firmware data, the device pointer will be unchanged
     * - If the update fails during device reboot, the device pointer is set to zero and the device deleted
     * - If the update succeeds, the device pointer will be set to the new device instance
     */
    explicit FirmwareUpdateDialog(Device *dev, QWidget *parent = nullptr);
    ~FirmwareUpdateDialog();

signals:
    void DeviceRebooting(); // emitted when the update process is triggered, the device should be disconnected
    void DeviceRebooted(QString serial); // emitted when an updated device is enumerated after the update

private slots:
    void on_bFile_clicked();
    void on_bStart_clicked();
    void timerCallback();
    void receivedAck();
    void receivedNack();

private:
    void addStatus(QString line);
    void abortWithError(QString error);
    void sendNextFirmwareChunk();
    Ui::FirmwareUpdateDialog *ui;
    Device *dev;
    QFile *file;
    QTimer timer;

    enum class State {
        Idle,
        ErasingFLASH,
        TransferringData,
        TriggeringUpdate,
        WaitingForReboot,
        WaitBeforeInitializing,
    };
    State state;
    unsigned int transferredBytes;
    QString serialnumber;
};

#endif // FIRMWAREUPDATEDIALOG_H
