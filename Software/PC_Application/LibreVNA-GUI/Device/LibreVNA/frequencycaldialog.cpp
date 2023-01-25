#include "frequencycaldialog.h"

#include "ui_frequencycaldialog.h"

FrequencyCalDialog::FrequencyCalDialog(LibreVNADriver *dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrequencyCalDialog),
    dev(dev)
{
    emit dev->acquireControl();
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->ppm->setUnit("ppm");
    ui->ppm->setPrefixes(" ");
    ui->ppm->setPrecision(4);
    ui->ppm->setValue(0.0);

    connect(dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p){
        if(p.type == Protocol::PacketType::FrequencyCorrection) {
            ui->ppm->setValueQuiet(p.frequencyCorrection.ppm);
        }
    }, Qt::QueuedConnection);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
        // get value and transfer to device
        Protocol::PacketInfo p;
        p.type = Protocol::PacketType::FrequencyCorrection;
        p.frequencyCorrection.ppm = ui->ppm->value();
        dev->SendPacket(p);
        accept();
        delete this;
    });

    // request setting from device
    dev->sendWithoutPayload(Protocol::PacketType::RequestFrequencyCorrection);
}

FrequencyCalDialog::~FrequencyCalDialog()
{
    emit dev->releaseControl();
    delete ui;
}
