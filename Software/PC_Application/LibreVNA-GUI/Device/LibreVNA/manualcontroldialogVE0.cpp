#include "manualcontroldialogVE0.h"

#include "ui_manualcontroldialogVE0.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogVE0::ManualControlDialogVE0(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogVE0),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    ui->src1Freq->setUnit("Hz");
    ui->src1Freq->setPrefixes(" kMG");
    ui->src1Freq->setPrecision(6);
    ui->src1Freq->setValueQuiet(100000000);

    ui->src2Freq->setUnit("Hz");
    ui->src2Freq->setPrefixes(" kMG");
    ui->src2Freq->setPrecision(6);
    ui->src2Freq->setValueQuiet(100000000);

    ui->LO1Freq->setUnit("Hz");
    ui->LO1Freq->setPrefixes(" kMG");
    ui->LO1Freq->setPrecision(6);
    ui->LO1Freq->setValueQuiet(100000000);

    ui->LO2Freq->setUnit("Hz");
    ui->LO2Freq->setPrefixes(" kMG");
    ui->LO2Freq->setPrecision(6);
    ui->LO2Freq->setValueQuiet(100000000);

    // Readonly widgets
    auto MakeReadOnly = [](QWidget* w) {
        w->setAttribute(Qt::WA_TransparentForMouseEvents);
        w->setFocusPolicy(Qt::NoFocus);
    };
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

    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p){
        if(p.type == Protocol::PacketType::ManualStatus) {
            NewStatus(p.manualStatus);
        }
    }, Qt::QueuedConnection);

    connect(ui->src1CE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->src2CE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO1CE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO2CE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->srcAmp, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO1CE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });

    connect(ui->srcSel, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->portSel, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->LOSel, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P1Path, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P1Amp, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P2Path, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P2Amp, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->RefAmp, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->Window, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    connect(ui->src1Freq, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->src2Freq, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LO1Freq, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LO2Freq, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->src1Pwr, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->src2Pwr, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->LO1Pwr, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->LO2Pwr, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->Samples, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });

    UpdateDevice();
}

ManualControlDialogVE0::~ManualControlDialogVE0()
{
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogVE0::NewStatus(Protocol::ManualStatus status)
{
    // ADC values
    auto &s = status.VE0;
    ui->port1min->setText(QString::number(s.port1min));
    ui->port1max->setText(QString::number(s.port1max));
    auto port1 = complex<double>(s.port1real, s.port1imag);
    ui->port1mag->setText(QString::number(abs(port1)));
    ui->port1phase->setText(QString::number(arg(port1)*180/M_PI));

    ui->port2min->setText(QString::number(s.port2min));
    ui->port2max->setText(QString::number(s.port2max));
    auto port2 = complex<double>(s.port2real, s.port2imag);
    ui->port2mag->setText(QString::number(abs(port2)));
    ui->port2phase->setText(QString::number(arg(port2)*180/M_PI));

    ui->refmin->setText(QString::number(s.refmin));
    ui->refmax->setText(QString::number(s.refmax));
    auto ref = complex<double>(s.refreal, s.refimag);
    ui->refmag->setText(QString::number(abs(ref)));
    ui->refphase->setText(QString::number(arg(ref)*180/M_PI));

    port1referenced = port1 / ref;
    port2referenced = port2 / ref;
    auto port1db = Util::SparamTodB(port1referenced);
    auto port2db = Util::SparamTodB(port2referenced);

    ui->port1referenced->setText(QString::number(port1db, 'f', 2) + "db@" + QString::number(arg(port1referenced)*180/M_PI, 'f', 2) + "°");
    ui->port2referenced->setText(QString::number(port2db, 'f', 2) + "db@" + QString::number(arg(port2referenced)*180/M_PI, 'f', 2) + "°");
}

void ManualControlDialogVE0::UpdateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.VE0;
    // Source
    m.src1Freq = ui->src1Freq->value();
    m.src2Freq = ui->src2Freq->value();
    m.src1Pwr = ui->src1Pwr->value();
    m.src2Pwr = ui->src2Pwr->value();
    m.src1CE = ui->src1CE->isChecked();
    m.src2CE = ui->src2CE->isChecked();
    m.srcSel = ui->srcSel->currentIndex();
    m.portSel = ui->portSel->currentIndex();
    m.srcAmp = ui->srcAmp->isChecked();
    // LO
    m.LO1Freq = ui->LO1Freq->value();
    m.LO2Freq = ui->LO2Freq->value();
    m.LO1Pwr = ui->LO1Pwr->value();
    m.LO2Pwr = ui->LO2Pwr->value();
    m.LO1CE = ui->LO1CE->isChecked();
    m.LO2CE = ui->LO2CE->isChecked();
    m.LOSel = ui->LOSel->currentIndex();
    // Port 1
    m.P1PathSel = ui->P1Path->currentIndex();
    m.P1AmpOn = ui->P1Amp->currentIndex() == 1 ? 1 : 0;
    m.P1AmpBypass = ui->P1Amp->currentIndex() == 2 ? 1 : 0;
    // Port 2
    m.P2PathSel = ui->P2Path->currentIndex();
    m.P2AmpOn = ui->P2Amp->currentIndex() == 1 ? 1 : 0;
    m.P2AmpBypass = ui->P2Amp->currentIndex() == 2 ? 1 : 0;
    // Reference
    m.RefAmpOn = ui->RefAmp->currentIndex() == 1 ? 1 : 0;
    m.RefAmpBypass = ui->RefAmp->currentIndex() == 2 ? 1 : 0;
    // Acquisition
    m.Samples = ui->Samples->value();
    m.WindowType = ui->Window->currentIndex();

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
