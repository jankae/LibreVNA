#include "manualcontroldialogVE1.h"

#include "ui_manualcontroldialogVE1.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogVE1::ManualControlDialogVE1(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogVE1),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    ui->srcFreq->setUnit("Hz");
    ui->srcFreq->setPrefixes(" kMG");
    ui->srcFreq->setPrecision(6);
    ui->srcFreq->setValueQuiet(100000000);

    ui->LOFreq->setUnit("Hz");
    ui->LOFreq->setPrefixes(" kMG");
    ui->LOFreq->setPrecision(6);
    ui->LOFreq->setValueQuiet(100000000);

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

    connect(ui->srcCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOAmp, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });

    connect(ui->portSel, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P1Path, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P1Amp, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P2Path, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->P2Amp, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->Window, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    connect(ui->srcFreq, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LOFreq, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->srcPwr, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->LOPwr, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->Samples, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });

    UpdateDevice();
}

ManualControlDialogVE1::~ManualControlDialogVE1()
{
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogVE1::NewStatus(Protocol::ManualStatus status)
{
    // ADC values
    auto &s = status.VE1;
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

void ManualControlDialogVE1::UpdateDevice()
{
    Protocol::PacketInfo p = {};
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.VE1;
    // Source
    m.srcFreq = ui->srcFreq->value();
    m.srcPwr = ui->srcPwr->value();
    m.srcCE = ui->srcCE->isChecked();
    m.portSel = ui->portSel->currentIndex();
    // LO
    m.LOFreq = ui->LOFreq->value();
    m.LOPwr = ui->LOPwr->value();
    m.LOCE = ui->LOCE->isChecked();
    m.LOAmp = ui->LOAmp->isChecked();
    // Port 1
    m.P1PathSel = ui->P1Path->currentIndex();
    m.P1AmpOn = ui->P1Amp->currentIndex() == 1 ? 1 : 0;
    m.P1AmpBypass = ui->P1Amp->currentIndex() == 2 ? 1 : 0;
    // Port 2
    m.P2PathSel = ui->P2Path->currentIndex();
    m.P2AmpOn = ui->P2Amp->currentIndex() == 1 ? 1 : 0;
    m.P2AmpBypass = ui->P2Amp->currentIndex() == 2 ? 1 : 0;
    // Acquisition
    m.Samples = ui->Samples->value();
    m.WindowType = ui->Window->currentIndex();

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
