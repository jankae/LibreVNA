#include "signalgenerator.h"
#include "ui_signalgenerator.h"

Signalgenerator::Signalgenerator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Signalgenerator)
{
    ui->setupUi(this);
    ui->frequency->setUnit("Hz");
    ui->frequency->setPrefixes(" kMG");

    connect(ui->frequency, &SIUnitEdit::valueChanged, [=](double newval) {
        // TODO centralize min/max values
       if(newval < 9000) {
           newval = 9000;
       } else if (newval > 6000000000) {
           newval = 6000000000;
       }
       ui->frequency->setValueQuiet(newval);
       SettingsChanged();
    });
    connect(ui->levelSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Signalgenerator::setLevel);
    connect(ui->levelSlider, &QSlider::valueChanged, [=](int value) {
        setLevel((double) value / 100.0);
    });
    connect(ui->EnablePort1, &QCheckBox::clicked, [=](){
        if(ui->EnablePort1->isChecked() && ui->EnablePort2->isChecked()) {
           ui->EnablePort2->setCheckState(Qt::CheckState::Unchecked);
        }
        SettingsChanged();
    });
    connect(ui->EnablePort2, &QCheckBox::clicked, [=](){
        if(ui->EnablePort1->isChecked() && ui->EnablePort2->isChecked()) {
           ui->EnablePort1->setCheckState(Qt::CheckState::Unchecked);
        }
        SettingsChanged();
    });
}

Signalgenerator::~Signalgenerator()
{
    delete ui;
}

void Signalgenerator::setLevel(double level)
{
    // TODO constrain to frequency dependent levels
    ui->levelSpin->blockSignals(true);
    ui->levelSlider->blockSignals(true);
    ui->levelSpin->setValue(level);
    ui->levelSlider->setValue(level * 100.0);
    ui->levelSpin->blockSignals(false);
    ui->levelSlider->blockSignals(false);
    SettingsChanged();
}

void Signalgenerator::SettingsChanged()
{
    // TODO compile manual settings packet and send
}
