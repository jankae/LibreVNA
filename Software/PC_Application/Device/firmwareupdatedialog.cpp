#include "firmwareupdatedialog.h"
#include "ui_firmwareupdatedialog.h"
#include <QFileDialog>

FirmwareUpdateDialog::FirmwareUpdateDialog(Device &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirmwareUpdateDialog),
    dev(dev),
    file(),
    timer(),
    state(State::Idle)
{
    ui->setupUi(this);
    ui->bFile->setIcon(this->style()->standardPixmap(QStyle::SP_FileDialogStart));
    ui->bStart->setIcon(this->style()->standardPixmap(QStyle::SP_MediaPlay));
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, [=](){
       abortWithError("Response timed out");
    });
}

FirmwareUpdateDialog::~FirmwareUpdateDialog()
{
    delete ui;
}

void FirmwareUpdateDialog::on_bFile_clicked()
{
    ui->bStart->setEnabled(false);
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open firmware file", "", "Firmware file (*.vnafw)", nullptr, QFileDialog::DontUseNativeDialog);
    if (filename.length() > 0) {
        ui->lFile->setText(filename);
        if(file) {
            delete file;
        }
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
    if(file->size() % Protocol::FirmwareChunkSize != 0) {
        abortWithError("Invalid file size");
        return;
    }
    char header[24];
    file->read(header, sizeof(header));
    if(strncmp(header, "VNA!", 4)) {
        abortWithError("Invalid magic header constant");
        return;
    }
    state = State::ErasingFLASH;
    addStatus("Erasing device memory...");
    dev.SendCommandWithoutPayload(Protocol::PacketType::ClearFlash);
    timer.start(10000);
}

void FirmwareUpdateDialog::addStatus(QString line)
{
    ui->status->appendPlainText(line);
}

void FirmwareUpdateDialog::abortWithError(QString error)
{
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
        transferredBytes += Protocol::FirmwareChunkSize;
        ui->progress->setValue(100 * transferredBytes / file->size());
        if(transferredBytes >= file->size()) {
            // complete file transferred
            addStatus("Triggering device update...");
            state = State::TriggeringUpdate;
            dev.SendCommandWithoutPayload(Protocol::PacketType::PerformFirmwareUpdate);
            timer.start(5000);
        }
        sendNextFirmwareChunk();
        timer.start(1000);
        break;
    case State::TriggeringUpdate:
        addStatus("Rebooting device...");
        // TODO delete current device and listen for reconnect
        state = State::Idle;
        break;
    }
}

void FirmwareUpdateDialog::sendNextFirmwareChunk()
{
    Protocol::FirmwarePacket fw;
    fw.address = transferredBytes;
    file->read((char*) &fw.data, Protocol::FirmwareChunkSize);
    dev.SendFirmwareChunk(fw);
}
