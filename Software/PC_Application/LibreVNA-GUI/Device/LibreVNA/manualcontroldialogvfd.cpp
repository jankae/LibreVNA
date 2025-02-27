#include "manualcontroldialogVFD.h"

#include "ui_manualcontroldialogVFD.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogVFD::ManualControlDialogVFD(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogVFD),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    ui->SourceFrequency->setUnit("Hz");
    ui->SourceFrequency->setPrefixes(" MG");
    ui->SourceFrequency->setPrecision(6);
    ui->SourceFrequency->setValueQuiet(1000000000);

    ui->LOFrequency->setUnit("Hz");
    ui->LOFrequency->setPrefixes(" MG");
    ui->LOFrequency->setPrecision(6);
    ui->LOFrequency->setValueQuiet(1000000000);

    ui->DACFrequencyA->setUnit("Hz");
    ui->DACFrequencyA->setPrefixes(" kM");
    ui->DACFrequencyA->setPrecision(6);
    ui->DACFrequencyA->setValueQuiet(1000000);

    ui->DACFrequencyB->setUnit("Hz");
    ui->DACFrequencyB->setPrefixes(" kM");
    ui->DACFrequencyB->setPrecision(6);
    ui->DACFrequencyB->setValueQuiet(1000000);

    ui->DACAmplitudeA->setValue(2047);
    ui->DACAmplitudeB->setValue(2047);

    auto updateVariableAtt = [=](unsigned int value){
        ui->SourceVariableAttSlider->setValue(value);
        ui->SourceVariableAttEntry->setValueQuiet(value);
        UpdateDevice();
    };
    connect(ui->SourceVariableAttEntry, &SIUnitEdit::valueChanged, this, [=](double newval){
        updateVariableAtt(newval);
    });
    connect(ui->SourceVariableAttSlider, &QSlider::sliderMoved, this, [=](int pos){
        updateVariableAtt(pos);
    });

    // Readonly widgets
    auto MakeReadOnly = [](QWidget* w) {
        w->setAttribute(Qt::WA_TransparentForMouseEvents);
        w->setFocusPolicy(Qt::NoFocus);
    };
    MakeReadOnly(ui->SourceLocked);
    MakeReadOnly(ui->LOLocked);

//    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p){
//            if(p.type == Protocol::PacketType::ManualStatus) {
//                NewStatus(p.manualStatus);
//            }
//    }, Qt::QueuedConnection);

    connect(ui->SourceCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceRFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LORFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceModEn, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceAmp1En, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceAmp2En, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOAmpEn, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->DACEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });

    connect(ui->SourceFilter, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->SourceBandsel, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->SourcePortSel, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->LOMode, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    connect(ui->SourceFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LOFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->DACFrequencyA, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->DACFrequencyB, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->SourceStepAtt, qOverload<int>(&QSpinBox::valueChanged), [=](int) { UpdateDevice(); });
    connect(ui->DACAmplitudeA, qOverload<int>(&QSpinBox::valueChanged), [=](int) { UpdateDevice(); });
    connect(ui->DACAmplitudeB, qOverload<int>(&QSpinBox::valueChanged), [=](int) { UpdateDevice(); });

    UpdateDevice();
}

ManualControlDialogVFD::~ManualControlDialogVFD()
{
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogVFD::UpdateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.VFD;
    // Source
    m.SourceFrequency = ui->SourceFrequency->value();
    m.SourceCE = ui->SourceCE->isChecked();
    m.SourceRFEN = ui->SourceRFEN->isChecked();
    m.SourceModEn = ui->SourceModEn->isChecked();
    m.SourceAmp1En = ui->SourceAmp1En->isChecked();
    m.SourceAmp2En = ui->SourceAmp2En->isChecked();
    m.SourceBandSel = ui->SourceBandsel->currentIndex();
    m.SourceFilter = ui->SourceFilter->currentIndex();
    m.SourceVariableAttenuator = ui->SourceVariableAttSlider->value();
    m.SourceStepAttenuator = ui->SourceStepAtt->value();
    m.SourcePortSel = ui->SourcePortSel->currentIndex();

    // LO
    m.LOCE = ui->LOCE->isChecked();
    m.LORFEN = ui->LOCE->isChecked();
    m.LOAmplifierEN = ui->LOAmpEn->isChecked();
    m.LOMode = ui->LOMode->currentIndex();
    m.LOFrequency = ui->LOFrequency->value();

    // DAC
    m.DACFreqA = ui->DACFrequencyA->value();
    m.DACFreqB = ui->DACFrequencyB->value();
    m.DACAmpA = ui->DACAmplitudeA->value();
    m.DACAmpB = ui->DACAmplitudeB->value();
    m.DACEn = ui->DACEnable->isChecked();

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
