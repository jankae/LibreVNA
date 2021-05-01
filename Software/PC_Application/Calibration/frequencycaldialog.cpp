#include "frequencycaldialog.h"
#include "ui_frequencycaldialog.h"

FrequencyCalDialog::FrequencyCalDialog(Device *dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrequencyCalDialog),
    dev(dev)
{
    ui->setupUi(this);
    ui->ppm->setUnit("ppm");
    ui->ppm->setPrefixes(" ");
    ui->ppm->setPrecision(4);
    ui->ppm->setValue(0.0);

    connect(dev, &Device::FrequencyCorrectionReceived, ui->ppm, &SIUnitEdit::setValueQuiet);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
        // get value and transfer to device
        Protocol::PacketInfo p;
        p.type = Protocol::PacketType::FrequencyCorrection;
        p.frequencyCorrection.ppm = ui->ppm->value();
        dev->SendPacket(p);
        // force restart of current mode for setting to take effect
        auto activeMode = Mode::getActiveMode();
        activeMode->deactivate();
        activeMode->activate();
        accept();
        delete this;
    });

    // request setting from device
    dev->SendCommandWithoutPayload(Protocol::PacketType::RequestFrequencyCorrection);
}


FrequencyCalDialog::~FrequencyCalDialog()
{
    delete ui;
}
