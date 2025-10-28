#include "deviceconfigurationdialogvd0.h"
#include "ui_deviceconfigurationdialogvd0.h"

DeviceConfigurationDialogVD0::DeviceConfigurationDialogVD0(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceConfigurationDialogVD0),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    auto updateIF = [=]() {
        auto ADCrate = ui->ADCRate->value();
        ui->IF->setValue(ADCrate * ui->ADCphaseInc->value() / 65536);
    };

    connect(ui->ADCRate, &SIUnitEdit::valueChanged, this, updateIF);
    connect(ui->ADCphaseInc, qOverload<int>(&QSpinBox::valueChanged), this, updateIF);

    ui->ADCRate->setUnit("Hz");
    ui->ADCRate->setPrefixes(" kM");
    ui->ADCRate->setPrecision(5);
    ui->IF->setUnit("Hz");
    ui->IF->setPrefixes(" kM");
    ui->IF->setPrecision(5);

    ui->ADCRate->setValue(1496000);
    ui->ADCphaseInc->setValue(10240);

    updateIF();

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

DeviceConfigurationDialogVD0::~DeviceConfigurationDialogVD0()
{
    dev.releaseControl();
    delete ui;
}

void DeviceConfigurationDialogVD0::updateGUI(const Protocol::DeviceConfig &c)
{
    ui->ADCRate->setValue(c.VD0.ADCrate);
    ui->ADCphaseInc->setValue(c.VD0.DFTphaseInc);
    ui->PLLSettlingDelay->setValue(c.VD0.PLLSettlingDelay);
}

void DeviceConfigurationDialogVD0::updateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::DeviceConfiguration;
    p.deviceConfig.VD0.ADCrate = ui->ADCRate->value();
    p.deviceConfig.VD0.DFTphaseInc = ui->ADCphaseInc->value();
    p.deviceConfig.VD0.PLLSettlingDelay = ui->PLLSettlingDelay->value();
    dev.SendPacket(p);
}
