#include "manualcontroldialogvff.h"

#include "ui_manualcontroldialogvff.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogVFF::ManualControlDialogVFF(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogVFF),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    ui->SourceFrequency->setUnit("Hz");
    ui->SourceFrequency->setPrefixes(" kMG");
    ui->SourceFrequency->setPrecision(6);
    ui->SourceFrequency->setValueQuiet(1000000000);

    ui->IF->setUnit("Hz");
    ui->IF->setPrefixes(" kM");
    ui->IF->setPrecision(6);

    ui->LOFrequency->setUnit("Hz");
    ui->LOFrequency->setPrefixes(" kMG");
    ui->LOFrequency->setPrecision(6);

    auto UpdateLO = [=]() {
        double sourceFreq = ui->SourceFrequency->value();
        if (ui->LOFreqType->currentIndex() == 0) {
            // fixed IF mode
            ui->LOFrequency->setValueQuiet(sourceFreq + ui->IF->value());
        } else {
            // Manual Frequency mode
            ui->IF->setValueQuiet(ui->LOFrequency->value() - sourceFreq);
        }
    };

    connect(ui->IF, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO();
    });
    connect(ui->LOFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO();
    });
    connect(ui->SourceFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO();
    });

    ui->IF->setValue(100000);

    // LO mode switch connections
    connect(ui->LOFreqType, qOverload<int>(&QComboBox::activated), [=](int index) {
        switch(index) {
        case 0:
            ui->LOFrequency->setEnabled(false);
            ui->IF->setEnabled(true);
            break;
        case 1:
            ui->LOFrequency->setEnabled(true);
            ui->IF->setEnabled(false);
            break;
        }
    });

    // Readonly widgets
    auto MakeReadOnly = [](QWidget* w) {
        w->setAttribute(Qt::WA_TransparentForMouseEvents);
        w->setFocusPolicy(Qt::NoFocus);
    };
    MakeReadOnly(ui->SourceLocked);
    MakeReadOnly(ui->LOlocked);
    MakeReadOnly(ui->portmin);
    MakeReadOnly(ui->portmax);
    MakeReadOnly(ui->portmag);
    MakeReadOnly(ui->portphase);
    MakeReadOnly(ui->portreferenced);
    MakeReadOnly(ui->refmin);
    MakeReadOnly(ui->refmax);
    MakeReadOnly(ui->refmag);
    MakeReadOnly(ui->refphase);

    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p){
        if(p.type == Protocol::PacketType::ManualStatus) {
            NewStatus(p.manualStatus);
        }
    }, Qt::QueuedConnection);

    connect(ui->SourceCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceRFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LORFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceAmplifierEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->Port1Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->RefEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->portgain, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });
    connect(ui->refgain, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });

    connect(ui->SourcePower, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });
    connect(ui->SourceFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LOAmplifierEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOExternal, &QRadioButton::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->IF, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->Attenuator, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->Samples, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->cbWindow, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    UpdateDevice();
}

ManualControlDialogVFF::~ManualControlDialogVFF()
{
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogVFF::setSourceChipEnable(bool enable)
{
    ui->SourceCE->setChecked(enable);
}

bool ManualControlDialogVFF::getSourceChipEnable()
{
    return ui->SourceCE->isChecked();
}

void ManualControlDialogVFF::setSourceRFEnable(bool enable)
{
    ui->SourceRFEN->setChecked(enable);
}

bool ManualControlDialogVFF::getSourceRFEnable()
{
    return ui->SourceRFEN->isChecked();
}

bool ManualControlDialogVFF::getSourceLocked()
{
    return ui->SourceLocked->isChecked();
}

static constexpr double powers[8] = {-1, +1, +2.5, +3.5, +4.5, +5.5, +6.5, +7};

bool ManualControlDialogVFF::setSourcePower(double dBm)
{
    for(unsigned int i=0;i<8;i++) {
        if(dBm == powers[i]) {
            ui->SourcePower->setCurrentIndex(i);
            return true;
        }
    }
    // invalid power setting
    return false;
}

double ManualControlDialogVFF::getSourcePower()
{
    return powers[ui->SourcePower->currentIndex()];
}

void ManualControlDialogVFF::setSourceFrequency(double f)
{
    ui->SourceFrequency->setValue(f);
}

double ManualControlDialogVFF::getSourceFrequency()
{
    return ui->SourceFrequency->value();
}

void ManualControlDialogVFF::setAttenuator(double att)
{
    ui->Attenuator->setValue(att);
}

double ManualControlDialogVFF::getAttenuator()
{
    return ui->Attenuator->value();
}

void ManualControlDialogVFF::setSourceAmplifierEnable(bool enable)
{
    ui->SourceAmplifierEnable->setChecked(enable);
}

bool ManualControlDialogVFF::getSourceAmplifierEnable()
{
    return ui->SourceAmplifierEnable->isChecked();
}

void ManualControlDialogVFF::setLOAmplifierEnable(bool enable)
{
    ui->LOAmplifierEnable->setChecked(enable);
}

bool ManualControlDialogVFF::getLOAmplifierEnable()
{
    return ui->LOAmplifierEnable->isChecked();
}

void ManualControlDialogVFF::setLOPath(bool external)
{
   if(external) {
       ui->LOExternal->setChecked(true);
   } else {
       ui->LOInternal->setChecked(true);
   }
}

bool ManualControlDialogVFF::getLOPath()
{
    return ui->LOExternal->isChecked();
}

void ManualControlDialogVFF::setLOChipEnable(bool enable)
{
    ui->LOCE->setChecked(enable);
}

bool ManualControlDialogVFF::getLOChipEnable()
{
    return ui->LOCE->isChecked();
}

void ManualControlDialogVFF::setLORFEnable(bool enable)
{
    ui->LORFEN->setChecked(enable);
}

bool ManualControlDialogVFF::getLORFEnable()
{
    return ui->LORFEN->isChecked();
}

bool ManualControlDialogVFF::getLOLocked()
{
    return ui->LOlocked->isChecked();
}

void ManualControlDialogVFF::setLOFrequency(double f)
{
    ui->LOFreqType->setCurrentIndex(1);
    ui->LOFrequency->setValue(f);
}

double ManualControlDialogVFF::getLOFrequency()
{
    return ui->LOFrequency->value();
}

void ManualControlDialogVFF::setIFFrequency(double f)
{
    ui->LOFreqType->setCurrentIndex(0);
    ui->IF->setValue(f);
}

double ManualControlDialogVFF::getIFFrequency()
{
    return ui->IF->value();
}

void ManualControlDialogVFF::setPortEnable(bool enable)
{
    ui->Port1Enable->setChecked(enable);
}

bool ManualControlDialogVFF::getPortEnable()
{
    return ui->Port1Enable->isChecked();
}

void ManualControlDialogVFF::setRefEnable(bool enable)
{
    ui->RefEnable->setChecked(enable);
}

bool ManualControlDialogVFF::getRefEnable()
{
    return ui->RefEnable->isChecked();
}

void ManualControlDialogVFF::setPortGain(Gain g)
{
    ui->portgain->setCurrentIndex((int) g);
}

ManualControlDialogVFF::Gain ManualControlDialogVFF::getPortGain()
{
    return (Gain) ui->portgain->currentIndex();
}

void ManualControlDialogVFF::setRefGain(Gain g)
{
    ui->refgain->setCurrentIndex((int) g);
}

ManualControlDialogVFF::Gain ManualControlDialogVFF::getRefGain()
{
    return (Gain) ui->refgain->currentIndex();
}

void ManualControlDialogVFF::setNumSamples(int samples)
{
    ui->Samples->setValue(samples);
}

int ManualControlDialogVFF::getNumSamples()
{
    return ui->Samples->value();
}

void ManualControlDialogVFF::setWindow(ManualControlDialogVFF::Window w)
{
    ui->cbWindow->setCurrentIndex((int) w);
}

ManualControlDialogVFF::Window ManualControlDialogVFF::getWindow()
{
    return (Window) ui->cbWindow->currentIndex();
}

int ManualControlDialogVFF::getPortMinADC()
{
    return ui->portmin->text().toInt();
}

int ManualControlDialogVFF::getPortMaxADC()
{
    return ui->portmax->text().toInt();
}

double ManualControlDialogVFF::getPortMagnitude()
{
    return ui->portmag->text().toDouble();
}

double ManualControlDialogVFF::getPortPhase()
{
    return ui->portphase->text().toDouble();
}

std::complex<double> ManualControlDialogVFF::getPortReferenced()
{
    return portreferenced;
}

int ManualControlDialogVFF::getRefMinADC()
{
    return ui->refmin->text().toInt();
}

int ManualControlDialogVFF::getRefMaxADC()
{
    return ui->refmax->text().toInt();
}

double ManualControlDialogVFF::getRefMagnitude()
{
    return ui->refmag->text().toDouble();
}

double ManualControlDialogVFF::getRefPhase()
{
    return ui->refphase->text().toDouble();
}

void ManualControlDialogVFF::NewStatus(Protocol::ManualStatus status)
{
    // ADC values
    ui->portmin->setText(QString::number(status.VFF.portmin));
    ui->portmax->setText(QString::number(status.VFF.portmax));
    auto port = complex<double>(status.VFF.portreal, status.VFF.portimag);
    ui->portmag->setText(QString::number(abs(port)));
    ui->portphase->setText(QString::number(arg(port)*180/M_PI));

    ui->refmin->setText(QString::number(status.VFF.refmin));
    ui->refmax->setText(QString::number(status.VFF.refmax));
    auto ref = complex<double>(status.VFF.refreal, status.VFF.refimag);
    ui->refmag->setText(QString::number(abs(ref)));
    ui->refphase->setText(QString::number(arg(ref)*180/M_PI));

    portreferenced = port / ref;
    auto portdb = Util::SparamTodB(portreferenced);

    ui->portreferenced->setText(QString::number(portdb, 'f', 1) + "db@" + QString::number(arg(portreferenced)*180/M_PI, 'f', 0) + "°");

    // PLL state
    ui->SourceLocked->setChecked(status.VFF.source_locked);
    ui->LOlocked->setChecked(status.VFF.LO_locked);
}

void ManualControlDialogVFF::UpdateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.VFF;
    // Source highband
    m.SourceCE = ui->SourceCE->isChecked();
    m.SourceRFEN = ui->SourceRFEN->isChecked();
    m.SourcePower = ui->SourcePower->currentIndex();
    m.SourceFrequency = ui->SourceFrequency->value();
    m.SourceAmplifierEN = ui->SourceAmplifierEnable->isChecked();
    m.attenuator = -ui->Attenuator->value() / 0.25;
    // LO
    m.LOCE = ui->LOCE->isChecked();
    m.LORFEN = ui->LORFEN->isChecked();
    m.LOAmplifierEN = ui->LOAmplifierEnable->isChecked();
    m.LOexternal = ui->LOExternal->isChecked();
    m.LOFrequency = ui->LOFrequency->value();

    // Acquisition
    m.PortEN = ui->Port1Enable->isChecked();
    m.PortGain = ui->portgain->currentIndex();
    m.RefEN = ui->RefEnable->isChecked();
    m.RefGain = ui->refgain->currentIndex();
    m.Samples = ui->Samples->value();
    m.WindowType = ui->cbWindow->currentIndex();

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
