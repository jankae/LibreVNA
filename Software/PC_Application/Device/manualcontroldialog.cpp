#include "manualcontroldialog.h"
#include "ui_manualcontroldialog.h"
#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>
#include "Util/util.h"

using namespace std;

ManualControlDialog::ManualControlDialog(Device &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialog),
    dev(dev)
{
    ui->setupUi(this);

    ui->SourceLowFrequency->setUnit("Hz");
    ui->SourceLowFrequency->setPrefixes(" kM");
    ui->SourceLowFrequency->setPrecision(6);
    ui->SourceLowFrequency->setValueQuiet(1000000);

    ui->SourceHighFrequency->setUnit("Hz");
    ui->SourceHighFrequency->setPrefixes(" kMG");
    ui->SourceHighFrequency->setPrecision(6);
    ui->SourceHighFrequency->setValueQuiet(1000000000);

    ui->IF1->setUnit("Hz");
    ui->IF1->setPrefixes(" kM");
    ui->IF1->setPrecision(6);

    ui->LO1Frequency->setUnit("Hz");
    ui->LO1Frequency->setPrefixes(" kMG");
    ui->LO1Frequency->setPrecision(6);

    ui->IF2->setUnit("Hz");
    ui->IF2->setPrefixes(" kM");
    ui->IF2->setPrecision(6);

    ui->LO2Frequency->setUnit("Hz");
    ui->LO2Frequency->setPrefixes(" kM");
    ui->LO2Frequency->setPrecision(6);

    auto UpdateLO1 = [=]() {
        double sourceFreq;
        if (ui->SwitchLowband->isChecked()) {
            sourceFreq = ui->SourceLowFrequency->value();
        } else {
            sourceFreq = ui->SourceHighFrequency->value();
        }
        if (ui->LO1FreqType->currentIndex() == 0) {
            // fixed IF mode
            ui->LO1Frequency->setValueQuiet(sourceFreq + ui->IF1->value());
        } else {
            // Manual Frequency mode
            ui->IF1->setValueQuiet(ui->LO1Frequency->value() - sourceFreq);
        }
    };
    auto UpdateLO2 = [=]() {
        double IF1 = ui->IF1->value();
        if (ui->LO2FreqType->currentIndex() == 0) {
            // fixed IF mode
            ui->LO2Frequency->setValueQuiet(IF1 + ui->IF2->value());
        } else {
            // Manual Frequency mode
            ui->IF2->setValueQuiet(ui->LO2Frequency->value() - IF1);
        }
    };

    connect(ui->IF1, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->LO1Frequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->IF2, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO2();
    });
    connect(ui->LO2Frequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO2();
    });
    connect(ui->SourceSwitchGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled), [=](int, bool) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->SourceLowFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->SourceHighFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });

    ui->IF1->setValue(60000000);
    ui->IF2->setValue(250000);

    // LO1/LO2 mode switch connections
    connect(ui->LO1FreqType, qOverload<int>(&QComboBox::activated), [=](int index) {
        switch(index) {
        case 0:
            ui->LO1Frequency->setEnabled(false);
            ui->IF1->setEnabled(true);
            break;
        case 1:
            ui->LO1Frequency->setEnabled(true);
            ui->IF1->setEnabled(false);
            break;
        }
    });
    connect(ui->LO2FreqType, qOverload<int>(&QComboBox::activated), [=](int index) {
        switch(index) {
        case 0:
            ui->LO2Frequency->setEnabled(false);
            ui->IF2->setEnabled(true);
            break;
        case 1:
            ui->LO2Frequency->setEnabled(true);
            ui->IF2->setEnabled(false);
            break;
        }
    });

    // Readonly widgets
    auto MakeReadOnly = [](QWidget* w) {
        w->setAttribute(Qt::WA_TransparentForMouseEvents);
        w->setFocusPolicy(Qt::NoFocus);
    };
    MakeReadOnly(ui->SourceLocked);
    MakeReadOnly(ui->LO1locked);
    MakeReadOnly(ui->port1min);
    MakeReadOnly(ui->port1max);
    MakeReadOnly(ui->port1mag);
    MakeReadOnly(ui->port1phase);
    MakeReadOnly(ui->port1referenced);
    MakeReadOnly(ui->port2min);
    MakeReadOnly(ui->port2max);
    MakeReadOnly(ui->port2mag);
    MakeReadOnly(ui->port2phase);
    MakeReadOnly(ui->port2referenced);
    MakeReadOnly(ui->refmin);
    MakeReadOnly(ui->refmax);
    MakeReadOnly(ui->refmag);
    MakeReadOnly(ui->refphase);

    connect(&dev, &Device::ManualStatusReceived, this, &ManualControlDialog::NewStatus);

    connect(ui->SourceCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceRFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO1CE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO1RFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceLowEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->AmplifierEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO2EN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->Port1Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->Port2Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->RefEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });

    connect(ui->SourceHighPower, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->SourceLowpass, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->SourceLowPower, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    connect(ui->SourceHighFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->SourceLowFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LO1Frequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->IF1, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LO2Frequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->IF2, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->PortSwitchGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled), [=](int, bool) { UpdateDevice(); });
    connect(ui->SourceSwitchGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled), [=](int, bool) { UpdateDevice(); });

    connect(ui->Attenuator, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->Samples, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->cbWindow, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    UpdateDevice();
}

ManualControlDialog::~ManualControlDialog()
{
    delete ui;
}

void ManualControlDialog::setHighSourceChipEnable(bool enable)
{
    ui->SourceCE->setChecked(enable);
}

bool ManualControlDialog::getHighSourceChipEnable()
{
    return ui->SourceCE->isChecked();
}

void ManualControlDialog::setHighSourceRFEnable(bool enable)
{
    ui->SourceRFEN->setChecked(enable);
}

bool ManualControlDialog::getHighSourceRFEnable()
{
    return ui->SourceRFEN->isChecked();
}

bool ManualControlDialog::getHighSourceLocked()
{
    return ui->SourceLocked->isChecked();
}

bool ManualControlDialog::setHighSourcePower(int dBm)
{
    switch(dBm) {
    case -4:
        ui->SourceHighPower->setCurrentIndex(0);
        break;
    case -1:
        ui->SourceHighPower->setCurrentIndex(1);
        break;
    case 2:
        ui->SourceHighPower->setCurrentIndex(2);
        break;
    case 5:
        ui->SourceHighPower->setCurrentIndex(3);
        break;
    default:
        // invalid power setting
        return false;
    }
    return true;
}

int ManualControlDialog::getHighSourcePower()
{
    int powers[4] = {-4,-1,2,5};
    return powers[ui->SourceHighPower->currentIndex()];
}

void ManualControlDialog::setHighSourceFrequency(double f)
{
    ui->SourceHighFrequency->setValue(f);
}

double ManualControlDialog::getHighSourceFrequency()
{
    return ui->SourceHighFrequency->value();
}

void ManualControlDialog::setHighSourceLPF(ManualControlDialog::LPF lpf)
{
    switch(lpf) {
    case LPF::M947:
        ui->SourceLowpass->setCurrentIndex(0);
        break;
    case LPF::M1880:
        ui->SourceLowpass->setCurrentIndex(1);
        break;
    case LPF::M3500:
        ui->SourceLowpass->setCurrentIndex(2);
        break;
    case LPF::None:
        ui->SourceLowpass->setCurrentIndex(3);
        break;
    }
}

ManualControlDialog::LPF ManualControlDialog::getHighSourceLPF()
{
    LPF lpfs[4] = {LPF::M947, LPF::M1880, LPF::M3500, LPF::None};
    return lpfs[ui->SourceLowpass->currentIndex()];
}

void ManualControlDialog::setLowSourceEnable(bool enable)
{
    ui->SourceLowEnable->setChecked(enable);
}

bool ManualControlDialog::getLowSourceEnable()
{
    return ui->SourceLowEnable->isChecked();
}

bool ManualControlDialog::setLowSourcePower(int mA)
{
    switch(mA) {
    case 2:
        ui->SourceLowPower->setCurrentIndex(0);
        break;
    case 4:
        ui->SourceLowPower->setCurrentIndex(1);
        break;
    case 6:
        ui->SourceLowPower->setCurrentIndex(2);
        break;
    case 8:
        ui->SourceLowPower->setCurrentIndex(3);
        break;
    default:
        // invalid power setting
        return false;
    }
    return true;
}

int ManualControlDialog::getLowSourcePower()
{
    int powers[4] = {2,4,6,8};
    return powers[ui->SourceLowPower->currentIndex()];
}

void ManualControlDialog::setLowSourceFrequency(double f)
{
    ui->SourceLowFrequency->setValue(f);
}

double ManualControlDialog::getLowSourceFrequency()
{
    return ui->SourceLowFrequency->value();
}

void ManualControlDialog::setHighband(bool high)
{
    if(high) {
        ui->SwitchHighband->setChecked(true);
    } else {
        ui->SwitchLowband->setChecked(true);
    }
}

bool ManualControlDialog::getHighband()
{
    return ui->SwitchHighband->isChecked();
}

void ManualControlDialog::setAttenuator(double att)
{
    ui->Attenuator->setValue(att);
}

double ManualControlDialog::getAttenuator()
{
    return ui->Attenuator->value();
}

void ManualControlDialog::setAmplifierEnable(bool enable)
{
    ui->AmplifierEnable->setChecked(enable);
}

bool ManualControlDialog::getAmplifierEnable()
{
    return ui->AmplifierEnable->isChecked();
}

bool ManualControlDialog::setPortSwitch(int port)
{
    switch(port) {
    case 1:
        ui->Port1Switch->setChecked(true);
        break;
    case 2:
        ui->Port2Switch->setChecked(true);
        break;
    default:
        // invalid port
        return false;
    }
    return true;
}

int ManualControlDialog::getPortSwitch()
{
    if(ui->Port1Switch->isChecked()) {
        return 1;
    } else {
        return 2;
    }
}

void ManualControlDialog::setLO1ChipEnable(bool enable)
{
    ui->LO1CE->setChecked(enable);
}

bool ManualControlDialog::getLO1ChipEnable()
{
    return ui->LO1CE->isChecked();
}

void ManualControlDialog::setLO1RFEnable(bool enable)
{
    ui->LO1RFEN->setChecked(enable);
}

bool ManualControlDialog::getLO1RFEnable()
{
    return ui->LO1RFEN->isChecked();
}

bool ManualControlDialog::getLO1Locked()
{
    return ui->LO1locked->isChecked();
}

void ManualControlDialog::setLO1Frequency(double f)
{
    ui->LO1FreqType->setCurrentIndex(1);
    ui->LO1Frequency->setValue(f);
}

double ManualControlDialog::getLO1Frequency()
{
    return ui->LO1Frequency->value();
}

void ManualControlDialog::setIF1Frequency(double f)
{
    ui->LO1FreqType->setCurrentIndex(0);
    ui->IF1->setValue(f);
}

double ManualControlDialog::getIF1Frequency()
{
    return ui->IF1->value();
}

void ManualControlDialog::setLO2Enable(bool enable)
{
    ui->LO2EN->setChecked(enable);
}

bool ManualControlDialog::getLO2Enable()
{
    return ui->LO2EN->isChecked();
}

void ManualControlDialog::setLO2Frequency(double f)
{
    ui->LO2FreqType->setCurrentIndex(1);
    ui->LO2Frequency->setValue(f);
}

double ManualControlDialog::getLO2Frequency()
{
    return ui->LO2Frequency->value();
}

void ManualControlDialog::setIF2Frequency(double f)
{
    ui->LO2FreqType->setCurrentIndex(0);
    ui->IF2->setValue(f);
}

double ManualControlDialog::getIF2Frequency()
{
    return ui->IF2->value();
}

void ManualControlDialog::setPort1Enable(bool enable)
{
    ui->Port1Enable->setChecked(enable);
}

bool ManualControlDialog::getPort1Enable()
{
    return ui->Port1Enable->isChecked();
}

void ManualControlDialog::setPort2Enable(bool enable)
{
    ui->Port2Enable->setChecked(enable);
}

bool ManualControlDialog::getPort2Enable()
{
    return ui->Port2Enable->isChecked();
}

void ManualControlDialog::setRefEnable(bool enable)
{
    ui->RefEnable->setChecked(enable);
}

bool ManualControlDialog::getRefEnable()
{
    return ui->RefEnable->isChecked();
}

void ManualControlDialog::setNumSamples(int samples)
{
    ui->Samples->setValue(samples);
}

int ManualControlDialog::getNumSamples()
{
    return ui->Samples->value();
}

void ManualControlDialog::setWindow(ManualControlDialog::Window w)
{
    ui->cbWindow->setCurrentIndex((int) w);
}

ManualControlDialog::Window ManualControlDialog::getWindow()
{
    return (Window) ui->cbWindow->currentIndex();
}

int ManualControlDialog::getPort1MinADC()
{
    return ui->port1min->text().toInt();
}

int ManualControlDialog::getPort1MaxADC()
{
    return ui->port1max->text().toInt();
}

double ManualControlDialog::getPort1Magnitude()
{
    return ui->port1mag->text().toDouble();
}

double ManualControlDialog::getPort1Phase()
{
    return ui->port1phase->text().toDouble();
}

std::complex<double> ManualControlDialog::getPort1Referenced()
{
    return port1referenced;
}

int ManualControlDialog::getPort2MinADC()
{
    return ui->port2min->text().toInt();
}

int ManualControlDialog::getPort2MaxADC()
{
    return ui->port2max->text().toInt();
}

double ManualControlDialog::getPort2Magnitude()
{
    return ui->port2mag->text().toDouble();
}

double ManualControlDialog::getPort2Phase()
{
    return ui->port2phase->text().toDouble();
}

std::complex<double> ManualControlDialog::getPort2Referenced()
{
    return port2referenced;
}

int ManualControlDialog::getRefMinADC()
{
    return ui->refmin->text().toInt();
}

int ManualControlDialog::getRefMaxADC()
{
    return ui->refmax->text().toInt();
}

double ManualControlDialog::getRefMagnitude()
{
    return ui->refmag->text().toDouble();
}

double ManualControlDialog::getRefPhase()
{
    return ui->refphase->text().toDouble();
}

void ManualControlDialog::NewStatus(Protocol::ManualStatus status)
{
    // ADC values
    ui->port1min->setText(QString::number(status.port1min));
    ui->port1max->setText(QString::number(status.port1max));
    auto port1 = complex<double>(status.port1real, status.port1imag);
    ui->port1mag->setText(QString::number(abs(port1)));
    ui->port1phase->setText(QString::number(arg(port1)*180/M_PI));

    ui->port2min->setText(QString::number(status.port2min));
    ui->port2max->setText(QString::number(status.port2max));
    auto port2 = complex<double>(status.port2real, status.port2imag);
    ui->port2mag->setText(QString::number(abs(port2)));
    ui->port2phase->setText(QString::number(arg(port2)*180/M_PI));

    ui->refmin->setText(QString::number(status.refmin));
    ui->refmax->setText(QString::number(status.refmax));
    auto ref = complex<double>(status.refreal, status.refimag);
    ui->refmag->setText(QString::number(abs(ref)));
    ui->refphase->setText(QString::number(arg(ref)*180/M_PI));

    port1referenced = port1 / ref;
    port2referenced = port2 / ref;
    auto port1db = Util::SparamTodB(port1referenced);
    auto port2db = Util::SparamTodB(port2referenced);

    ui->port1referenced->setText(QString::number(port1db, 'f', 1) + "db@" + QString::number(arg(port1referenced)*180/M_PI, 'f', 0) + "°");
    ui->port2referenced->setText(QString::number(port2db, 'f', 1) + "db@" + QString::number(arg(port2referenced)*180/M_PI, 'f', 0) + "°");

    // PLL state
    ui->SourceLocked->setChecked(status.source_locked);
    ui->LO1locked->setChecked(status.LO_locked);
}

void ManualControlDialog::UpdateDevice()
{
    Protocol::ManualControl m;
    // Source highband
    m.SourceHighCE = ui->SourceCE->isChecked();
    m.SourceHighRFEN = ui->SourceRFEN->isChecked();
    m.SourceHighPower = ui->SourceHighPower->currentIndex();
    m.SourceHighFrequency = ui->SourceHighFrequency->value();
    m.SourceHighLowpass = ui->SourceLowpass->currentIndex();
    // Source lowband
    m.SourceLowEN = ui->SourceLowEnable->isChecked();
    m.SourceLowPower = ui->SourceLowPower->currentIndex();
    m.SourceLowFrequency = ui->SourceLowFrequency->value();
    // Source signal path
    m.SourceHighband = ui->SwitchHighband->isChecked();
    m.AmplifierEN = ui->AmplifierEnable->isChecked();
    m.PortSwitch = ui->Port2Switch->isChecked();
    m.attenuator = -ui->Attenuator->value() / 0.25;
    // LO1
    m.LO1CE = ui->LO1CE->isChecked();
    m.LO1RFEN = ui->LO1RFEN->isChecked();
    m.LO1Frequency = ui->LO1Frequency->value();
    // LO2
    m.LO2EN = ui->LO2EN->isChecked();
    m.LO2Frequency = ui->LO2Frequency->value();
    // Acquisition
    m.Port1EN = ui->Port1Enable->isChecked();
    m.Port2EN = ui->Port2Enable->isChecked();
    m.RefEN = ui->RefEnable->isChecked();
    m.Samples = ui->Samples->value();
    m.WindowType = ui->cbWindow->currentIndex();

    qDebug() << "Updating manual control state";

    dev.SetManual(m);
}
