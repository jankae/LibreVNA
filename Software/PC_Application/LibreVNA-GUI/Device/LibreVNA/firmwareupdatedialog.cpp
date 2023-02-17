#include "firmwareupdatedialog.h"

#include "ui_firmwareupdatedialog.h"
#include "../../VNA_embedded/Application/Communication/PacketConstants.h"

#include <QFileDialog>
#include <QStyle>

FirmwareUpdateDialog::FirmwareUpdateDialog(LibreVNADriver *dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirmwareUpdateDialog),
    dev(dev),
    file(),
    timer(),
    state(State::Idle),
    transferredBytes(0)
{
    dev->acquireControl();
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->bFile->setIcon(this->style()->standardPixmap(QStyle::SP_FileDialogStart));
    ui->bStart->setIcon(this->style()->standardPixmap(QStyle::SP_MediaPlay));
    connect(&timer, &QTimer::timeout, this, &FirmwareUpdateDialog::timerCallback);
}

FirmwareUpdateDialog::~FirmwareUpdateDialog()
{
    dev->releaseControl();
    delete ui;
}

void FirmwareUpdateDialog::on_bFile_clicked()
{
    ui->bStart->setEnabled(false);
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open firmware file", "", "Firmware file (*.vnafw)", nullptr, QFileDialog::DontUseNativeDialog);
    if (filename.length() > 0) {
        ui->lFile->setText(filename);
        delete file;
        file = new QFile(filename);
        ui->bStart->setEnabled(true);
    }
}

void FirmwareUpdateDialog::on_bStart_clicked()
{
    ui->status->clear();
    ui->bStart->setEnabled(false);
    if(!file->isOpen()) {
        if(!file->open(QIODevice::ReadOnly)) {
            abortWithError("Unable to open file");
            return;
        }
    }
    file->seek(0);
    addStatus("Evaluating file...");
    if(file->size() % PacketConstants::FW_CHUNK_SIZE != 0) {
        abortWithError("Invalid file size");
        return;
    }
    char header[24];
    file->read(header, sizeof(header));
    if(strncmp(header, dev->getFirmwareMagicString().toStdString().c_str(), 4)) {
        abortWithError("Invalid magic header constant");
        return;
    }
    file->seek(0);
    state = State::ErasingFLASH;
    connect(dev, &LibreVNADriver::receivedAnswer, this, [=](const LibreVNADriver::TransmissionResult &res) {
        if(res == LibreVNADriver::TransmissionResult::Ack) {
            receivedAck();
        } else if(res == LibreVNADriver::TransmissionResult::Nack) {
            receivedNack();
        }
    }, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    addStatus("Erasing device memory...");
    dev->sendWithoutPayload(Protocol::PacketType::ClearFlash);
    timer.setSingleShot(true);
    timer.start(20000);
}

void FirmwareUpdateDialog::addStatus(QString line)
{
    ui->status->appendPlainText(line);
}

void FirmwareUpdateDialog::abortWithError(QString error)
{
    timer.stop();

    QTextCharFormat tf;
    tf = ui->status->currentCharFormat();
    tf.setForeground(QBrush(Qt::red));
    ui->status->setCurrentCharFormat(tf);
    ui->status->appendPlainText(error);
    tf.setForeground(QBrush(Qt::black));
    ui->status->setCurrentCharFormat(tf);
    ui->bStart->setEnabled(true);
    state = State::Idle;
}

void FirmwareUpdateDialog::timerCallback()
{
    switch(state) {
    case State::WaitingForReboot: {
        // Currently waiting for the reboot, check device list
        auto devices = dev->GetAvailableDevices();
        if(devices.find(serialnumber) != devices.end()) {
            // the device rebooted and is available again
            addStatus("...device enumerated, update complete");
            state = State::WaitBeforeInitializing;
            timer.start(3000);
        }
    }
        break;
    case State::WaitBeforeInitializing:
        // Device had enough time to initialize, indicate that rebooted device is ready
        timer.stop();
        dev->connectDevice(serialnumber);
        delete this;
        break;
    default:
        abortWithError("Response timed out");
        break;
    }
}

void FirmwareUpdateDialog::receivedAck()
{
    switch(state) {
    case State::Idle:
        // no firmware update in progress, ignore
        break;
    case State::ErasingFLASH:
        // FLASH erased, begin transferring firmware
        state = State::TransferringData;
        transferredBytes = 0;
        addStatus("Transferring firmware...");
        sendNextFirmwareChunk();
        timer.start(1000);
        break;
    case State::TransferringData:
        transferredBytes += PacketConstants::FW_CHUNK_SIZE;
        ui->progress->setValue(100 * transferredBytes / file->size());
        if(transferredBytes >= file->size()) {
            // complete file transferred
            addStatus("Triggering device update...");
            state = State::TriggeringUpdate;
            dev->sendWithoutPayload(Protocol::PacketType::PerformFirmwareUpdate);
            timer.start(5000);
        } else {
            sendNextFirmwareChunk();
            timer.start(1000);
        }
        break;
    case State::TriggeringUpdate:
        addStatus("Rebooting device...");
        serialnumber = dev->getSerial();
        dev->disconnectDevice();
        state = State::WaitingForReboot;
        timer.setSingleShot(false);
        timer.start(2000);
        break;
    default:
        break;
    }
}

void FirmwareUpdateDialog::receivedNack()
{
    switch(state) {
    case State::ErasingFLASH:
        abortWithError("Nack received, device does not support firmware update");
        break;
    default:
        abortWithError("Nack received, something went wrong");
        break;
    }

}

void FirmwareUpdateDialog::sendNextFirmwareChunk()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::FirmwarePacket;
    p.firmware.address = transferredBytes;
    file->read((char*) &p.firmware.data, PacketConstants::FW_CHUNK_SIZE);
    dev->SendPacket(p);
}
