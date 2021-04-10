#include "signalgenwidget.h"
#include "ui_signalgenwidget.h"

SignalgeneratorWidget::SignalgeneratorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SignalgeneratorWidget)
{
    ui->setupUi(this);
    ui->frequency->setUnit("Hz");
    ui->frequency->setPrefixes(" kMG");
    ui->frequency->setPrecision(6); // show enough digits

    ui->span->setUnit("Hz");
    ui->span->setPrefixes(" kMG");
    ui->span->setPrecision(6); // show enough digits
    ui->span->setValue(0);

    ui->current->setUnit("Hz");
    ui->current->setPrefixes(" kMG");
    ui->current->setPrecision(6); // show enough digits

    ui->dwell->setUnit("s");
    ui->dwell->setPrefixes(" m");
    ui->dwell->setPrecision(6); // show enough digits
    ui->dwell->setValue(1);
    m_timerId = startTimer(1000);

    ui->steps->setValue(100);
    ui->steps->setPrefixes(" k");
    ui->steps->setPrecision(0);

    connect(ui->frequency, &SIUnitEdit::valueChanged, [=](double newval) {
       if(newval < Device::Info().limits_minFreq) {
           newval = Device::Info().limits_minFreq;
       } else if (newval > Device::Info().limits_maxFreq) {
           newval = Device::Info().limits_maxFreq;
       }
       ui->frequency->setValueQuiet(newval);
       if (newval < ui->span->value()/2)
           ui->span->setValueQuiet(newval/2);
       if (newval + ui->span->value()/2 > Device::Info().limits_maxFreq)
           ui->span->setValueQuiet((Device::Info().limits_maxFreq - newval)*2);
       newval = ui->frequency->value() - ui->span->value()/2;
       ui->current->setValueQuiet(newval);
       emit SettingsChanged();
    });

    connect(ui->span, &SIUnitEdit::valueChanged, [=](double newval) {
       if(newval < 0 ) {
           newval = 0;
       } else if (newval > Device::Info().limits_maxFreq - Device::Info().limits_minFreq) {
           newval = Device::Info().limits_maxFreq - Device::Info().limits_minFreq;
       }
       ui->span->setValueQuiet(newval);

       double newF = ui->frequency->value() - ui->span->value()/2;
       if (newF < 0) {
           ui->frequency->setValueQuiet(ui->span->value()/2);
       }
       newF = ui->frequency->value() + ui->span->value()/2;
       if (newF  > Device::Info().limits_maxFreq) {
           ui->frequency->setValueQuiet(Device::Info().limits_maxFreq - ui->span->value()/2);
       }

       newval = ui->frequency->value() - ui->span->value()/2;

       emit SettingsChanged();
    });

    connect(ui->current, &SIUnitEdit::valueChanged, [=](double newval) {
       if(newval < 0 ) {
           newval = 0;
       } else if (newval > Device::Info().limits_maxFreq - Device::Info().limits_minFreq) {
           newval = Device::Info().limits_maxFreq - Device::Info().limits_minFreq;
       }
       ui->current->setValueQuiet(newval);
       emit SettingsChanged();
    });

    connect(ui->dwell, &SIUnitEdit::valueChanged, [=](double newval) {
       if(newval < 0.01 ) {
           newval = 0.01;
       } else if (newval > 60) {
           newval = 60;
       }
       ui->dwell->setValueQuiet(newval);
       m_timerId = startTimer(newval*1000);
       emit SettingsChanged();
    });

    connect(ui->levelSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SignalgeneratorWidget::setLevel);
    connect(ui->levelSlider, &QSlider::valueChanged, [=](int value) {
        setLevel((double) value / 100.0);
    });
    connect(ui->EnablePort1, &QCheckBox::toggled, [=](){
        if(ui->EnablePort1->isChecked() && ui->EnablePort2->isChecked()) {
           ui->EnablePort2->setCheckState(Qt::CheckState::Unchecked);
        }
        emit SettingsChanged();
    });
    connect(ui->EnablePort2, &QCheckBox::toggled, [=](){
        if(ui->EnablePort1->isChecked() && ui->EnablePort2->isChecked()) {
           ui->EnablePort1->setCheckState(Qt::CheckState::Unchecked);
        }
        emit SettingsChanged();
    });
    connect(ui->EnabledSweep, &QCheckBox::toggled, [=](bool enabled){
        ui->current->setEnabled(enabled);
        if(enabled) {
            double newF = ui->frequency->value() - ui->span->value()/2;
            if (newF < 0) {
                ui->frequency->setValueQuiet(ui->frequency->value() - newF);
                newF = 0;
            }
            ui->current->setValueQuiet(newF);
        } else {
        }
        emit SettingsChanged();
    });
}


SignalgeneratorWidget::~SignalgeneratorWidget()
{
    delete ui;
}

void SignalgeneratorWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId) {
        if (ui->EnabledSweep->isChecked()) {
            double newF = ui->current->value() + ui->span->value()/ui->steps->value();
            if (newF > ui->frequency->value() + ui->span->value()/2)
                newF = ui->frequency->value() - ui->span->value()/2;
            ui->current->setValueQuiet(newF);
            SettingsChanged();
        }
    }
}

Protocol::GeneratorSettings SignalgeneratorWidget::getDeviceStatus()
{
    Protocol::GeneratorSettings s = {};
    if (ui->EnabledSweep->isChecked())
        s.frequency = ui->current->value();
    else
        s.frequency = ui->frequency->value();
    s.cdbm_level = ui->levelSpin->value() * 100.0;
    if(ui->EnablePort1->isChecked()) {
        s.activePort = 1;
    } else if(ui->EnablePort2->isChecked()) {
        s.activePort = 2;
    } else {
        s.activePort = 0;
    }
    s.applyAmplitudeCorrection = 1;
    return s;
}

void SignalgeneratorWidget::setLevel(double level)
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

void SignalgeneratorWidget::setFrequency(double frequency)
{
    ui->frequency->setValue(frequency);
}

void SignalgeneratorWidget::setPort(int port)
{
    if(port < 0 || port > 2) {
        return;
    }
    switch(port) {
    case 0:
        ui->EnablePort1->setChecked(false);
        ui->EnablePort2->setChecked(false);
        break;
    case 1:
        ui->EnablePort1->setChecked(true);
        ui->EnablePort2->setChecked(false);
        break;
    case 2:
        ui->EnablePort1->setChecked(false);
        ui->EnablePort2->setChecked(true);
        break;
    }
}

