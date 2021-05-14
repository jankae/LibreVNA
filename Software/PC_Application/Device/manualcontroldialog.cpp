#include "manualcontroldialog.h"
#include "ui_manualcontroldialog.h"
#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>
#include "unit.h"

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

    auto port1referenced = port1 / ref;
    auto port2referenced = port2 / ref;
    auto port1db = Unit::dB(port1referenced);
    auto port2db = Unit::dB(port2referenced);

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
