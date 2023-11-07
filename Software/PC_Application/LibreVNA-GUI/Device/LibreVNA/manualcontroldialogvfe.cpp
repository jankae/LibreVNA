#include "manualcontroldialogvfe.h"

#include "ui_manualcontroldialogvfe.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogVFE::ManualControlDialogVFE(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogVFE),
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

    ui->eCalTarget->setUnit("°");
    ui->eCalTarget->setPrecision(4);
    ui->eCalTarget->setValue(0);

    ui->eCalTemp->setUnit("°");
    ui->eCalTemp->setPrecision(4);

    ui->eCalPower->setUnit("W");
    ui->eCalPower->setPrefixes("m ");
    ui->eCalPower->setPrecision(3);

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
    MakeReadOnly(ui->eCalPower);
    MakeReadOnly(ui->eCalTemp);

    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p){
        if(p.type == Protocol::PacketType::ManualStatus) {
            NewStatus(p.manualStatus);
        }
    }, Qt::QueuedConnection);

    connect(ui->SourceCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceRFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LORFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceAmplifier1Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceAmplifier2Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->Port1Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->RefEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->portgain, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });
    connect(ui->refgain, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });

    connect(ui->SourceFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LOFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->IF, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->Attenuator, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->Samples, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->cbWindow, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    connect(ui->eCalState, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });
    connect(ui->eCalTarget, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    UpdateDevice();
}

ManualControlDialogVFE::~ManualControlDialogVFE()
{
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogVFE::setSourceChipEnable(bool enable)
{
    ui->SourceCE->setChecked(enable);
}

bool ManualControlDialogVFE::getSourceChipEnable()
{
    return ui->SourceCE->isChecked();
}

void ManualControlDialogVFE::setSourceRFEnable(bool enable)
{
    ui->SourceRFEN->setChecked(enable);
}

bool ManualControlDialogVFE::getSourceRFEnable()
{
    return ui->SourceRFEN->isChecked();
}

bool ManualControlDialogVFE::getSourceLocked()
{
    return ui->SourceLocked->isChecked();
}

void ManualControlDialogVFE::setSourceFrequency(double f)
{
    ui->SourceFrequency->setValue(f);
}

double ManualControlDialogVFE::getSourceFrequency()
{
    return ui->SourceFrequency->value();
}

void ManualControlDialogVFE::setAttenuator(double att)
{
    ui->Attenuator->setValue(att);
}

double ManualControlDialogVFE::getAttenuator()
{
    return ui->Attenuator->value();
}

void ManualControlDialogVFE::setSourceAmplifier1Enable(bool enable)
{
    ui->SourceAmplifier1Enable->setChecked(enable);
}

bool ManualControlDialogVFE::getSourceAmplifier1Enable()
{
    return ui->SourceAmplifier1Enable->isChecked();
}

void ManualControlDialogVFE::setSourceAmplifier2Enable(bool enable)
{
    ui->SourceAmplifier2Enable->setChecked(enable);
}

bool ManualControlDialogVFE::getSourceAmplifier2Enable()
{
    return ui->SourceAmplifier2Enable->isChecked();
}

void ManualControlDialogVFE::setLOChipEnable(bool enable)
{
    ui->LOCE->setChecked(enable);
}

bool ManualControlDialogVFE::getLOChipEnable()
{
    return ui->LOCE->isChecked();
}

void ManualControlDialogVFE::setLORFEnable(bool enable)
{
    ui->LORFEN->setChecked(enable);
}

bool ManualControlDialogVFE::getLORFEnable()
{
    return ui->LORFEN->isChecked();
}

bool ManualControlDialogVFE::getLOLocked()
{
    return ui->LOlocked->isChecked();
}

void ManualControlDialogVFE::setLOFrequency(double f)
{
    ui->LOFreqType->setCurrentIndex(1);
    ui->LOFrequency->setValue(f);
}

double ManualControlDialogVFE::getLOFrequency()
{
    return ui->LOFrequency->value();
}

void ManualControlDialogVFE::setIFFrequency(double f)
{
    ui->LOFreqType->setCurrentIndex(0);
    ui->IF->setValue(f);
}

double ManualControlDialogVFE::getIFFrequency()
{
    return ui->IF->value();
}

void ManualControlDialogVFE::setPortEnable(bool enable)
{
    ui->Port1Enable->setChecked(enable);
}

bool ManualControlDialogVFE::getPortEnable()
{
    return ui->Port1Enable->isChecked();
}

void ManualControlDialogVFE::setRefEnable(bool enable)
{
    ui->RefEnable->setChecked(enable);
}

bool ManualControlDialogVFE::getRefEnable()
{
    return ui->RefEnable->isChecked();
}

void ManualControlDialogVFE::setPortGain(Gain g)
{
    ui->portgain->setCurrentIndex((int) g);
}

ManualControlDialogVFE::Gain ManualControlDialogVFE::getPortGain()
{
    return (Gain) ui->portgain->currentIndex();
}

void ManualControlDialogVFE::setRefGain(Gain g)
{
    ui->refgain->setCurrentIndex((int) g);
}

ManualControlDialogVFE::Gain ManualControlDialogVFE::getRefGain()
{
    return (Gain) ui->refgain->currentIndex();
}

void ManualControlDialogVFE::setNumSamples(int samples)
{
    ui->Samples->setValue(samples);
}

int ManualControlDialogVFE::getNumSamples()
{
    return ui->Samples->value();
}

void ManualControlDialogVFE::setWindow(ManualControlDialogVFE::Window w)
{
    ui->cbWindow->setCurrentIndex((int) w);
}

ManualControlDialogVFE::Window ManualControlDialogVFE::getWindow()
{
    return (Window) ui->cbWindow->currentIndex();
}

int ManualControlDialogVFE::getPortMinADC()
{
    return ui->portmin->text().toInt();
}

int ManualControlDialogVFE::getPortMaxADC()
{
    return ui->portmax->text().toInt();
}

double ManualControlDialogVFE::getPortMagnitude()
{
    return ui->portmag->text().toDouble();
}

double ManualControlDialogVFE::getPortPhase()
{
    return ui->portphase->text().toDouble();
}

std::complex<double> ManualControlDialogVFE::getPortReferenced()
{
    return portreferenced;
}

int ManualControlDialogVFE::getRefMinADC()
{
    return ui->refmin->text().toInt();
}

int ManualControlDialogVFE::getRefMaxADC()
{
    return ui->refmax->text().toInt();
}

double ManualControlDialogVFE::getRefMagnitude()
{
    return ui->refmag->text().toDouble();
}

double ManualControlDialogVFE::getRefPhase()
{
    return ui->refphase->text().toDouble();
}

void ManualControlDialogVFE::NewStatus(Protocol::ManualStatus status)
{
    // ADC values
    ui->portmin->setText(QString::number(status.VFE.portmin));
    ui->portmax->setText(QString::number(status.VFE.portmax));
    auto port = complex<double>(status.VFE.portreal, status.VFE.portimag);
    ui->portmag->setText(QString::number(abs(port)));
    ui->portphase->setText(QString::number(arg(port)*180/M_PI));

    ui->refmin->setText(QString::number(status.VFE.refmin));
    ui->refmax->setText(QString::number(status.VFE.refmax));
    auto ref = complex<double>(status.VFE.refreal, status.VFE.refimag);
    ui->refmag->setText(QString::number(abs(ref)));
    ui->refphase->setText(QString::number(arg(ref)*180/M_PI));

    portreferenced = port / ref;
    auto portdb = Util::SparamTodB(portreferenced);

    ui->portreferenced->setText(QString::number(portdb, 'f', 1) + "db@" + QString::number(arg(portreferenced)*180/M_PI, 'f', 0) + "°");

    // PLL state
    ui->SourceLocked->setChecked(status.VFE.source_locked);
    ui->LOlocked->setChecked(status.VFE.LO_locked);

    // eCal
    ui->eCalTemp->setValue((double) status.VFE.temp_eCal / 100.0);
    ui->eCalPower->setValue((double) status.VFE.power_heater / 1000.0);
}

void ManualControlDialogVFE::UpdateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.VFE;
    // Source highband
    m.SourceCE = ui->SourceCE->isChecked();
    m.SourceRFEN = ui->SourceRFEN->isChecked();
    m.SourceFrequency = ui->SourceFrequency->value();
    m.SourceAmplifier1EN = ui->SourceAmplifier1Enable->isChecked();
    m.SourceAmplifier2EN = ui->SourceAmplifier2Enable->isChecked();
    m.attenuator = -ui->Attenuator->value() / 0.25;
    // LO
    m.LOCE = ui->LOCE->isChecked();
    m.LORFEN = ui->LORFEN->isChecked();
    m.LOFrequency = ui->LOFrequency->value();

    // Acquisition
    m.PortEN = ui->Port1Enable->isChecked();
    m.PortGain = ui->portgain->currentIndex();
    m.RefEN = ui->RefEnable->isChecked();
    m.RefGain = ui->refgain->currentIndex();
    m.Samples = ui->Samples->value();
    m.WindowType = ui->cbWindow->currentIndex();

    // eCal
    m.eCal_state = ui->eCalState->currentIndex();
    m.eCal_target = ui->eCalTarget->value() * 100;

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
