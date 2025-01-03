#include "deviceconfigurationdialogv1.h"
#include "ui_deviceconfigurationdialogv1.h"

DeviceConfigurationDialogV1::DeviceConfigurationDialogV1(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceConfigurationDialogV1),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();


    auto updateADCRate = [=]() {
        // update ADC rate, see FPGA protocol for calculation
        ui->ADCRate->setValue(102400000.0 / ui->ADCpresc->value());
    };
    auto updateIF2 = [=]() {
        auto ADCrate = ui->ADCRate->value();
        ui->IF2->setValue(ADCrate * ui->ADCphaseInc->value() / 4096);
    };

    connect(ui->ADCpresc, qOverload<int>(&QSpinBox::valueChanged), updateADCRate);
    connect(ui->ADCpresc, qOverload<int>(&QSpinBox::valueChanged), updateIF2);
    connect(ui->ADCphaseInc, qOverload<int>(&QSpinBox::valueChanged), updateIF2);

    ui->IF1->setUnit("Hz");
    ui->IF1->setPrefixes(" kM");
    ui->IF1->setPrecision(5);
    ui->ADCRate->setUnit("Hz");
    ui->ADCRate->setPrefixes(" kM");
    ui->ADCRate->setPrecision(5);
    ui->IF2->setUnit("Hz");
    ui->IF2->setPrefixes(" kM");
    ui->IF2->setPrecision(5);
    ui->IF1->setValue(62000000);
    ui->ADCpresc->setValue(128);
    ui->ADCphaseInc->setValue(1280);

    updateADCRate();
    updateIF2();

    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p) {
        if(p.type == Protocol::PacketType::DeviceConfiguration) {
            updateGUI(p.deviceConfig);
        }
    });

    dev.sendWithoutPayload(Protocol::PacketType::RequestDeviceConfiguration);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){
        updateDevice();
        accept();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [=](){
        reject();
    });
}

DeviceConfigurationDialogV1::~DeviceConfigurationDialogV1()
{
    dev.releaseControl();
    delete ui;
}

void DeviceConfigurationDialogV1::updateGUI(const Protocol::DeviceConfig &c)
{
    ui->IF1->setEnabled(false);
    ui->IF1->setValue(c.V1.IF1);
    ui->IF1->setEnabled(true);
    ui->ADCpresc->setValue(c.V1.ADCprescaler);
    ui->ADCphaseInc->setValue(c.V1.DFTphaseInc);
    ui->PLLSettlingDelay->setValue(c.V1.PLLSettlingDelay);
}

void DeviceConfigurationDialogV1::updateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::DeviceConfiguration;
    p.deviceConfig.V1.IF1 = ui->IF1->value();
    p.deviceConfig.V1.ADCprescaler = ui->ADCpresc->value();
    p.deviceConfig.V1.DFTphaseInc = ui->ADCphaseInc->value();
    p.deviceConfig.V1.PLLSettlingDelay = ui->PLLSettlingDelay->value();
    dev.SendPacket(p);
}
