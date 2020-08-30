#ifndef FIRMWAREUPDATEDIALOG_H
#define FIRMWAREUPDATEDIALOG_H

#include <QDialog>
#include "device.h"
#include <QFile>
#include <QTimer>

namespace Ui {
class FirmwareUpdateDialog;
}

class FirmwareUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FirmwareUpdateDialog(Device &dev, QWidget *parent = nullptr);
    ~FirmwareUpdateDialog();

private slots:
    void on_bFile_clicked();

    void on_bStart_clicked();

private:
    void addStatus(QString line);
    void abortWithError(QString error);
    void receivedAck();
    void sendNextFirmwareChunk();
    Ui::FirmwareUpdateDialog *ui;
    Device &dev;
    QFile *file;
    QTimer timer;

    enum class State {
        Idle,
        ErasingFLASH,
        TransferringData,
        TriggeringUpdate,
    };
    State state;
    unsigned int transferredBytes;
};

#endif // FIRMWAREUPDATEDIALOG_H
