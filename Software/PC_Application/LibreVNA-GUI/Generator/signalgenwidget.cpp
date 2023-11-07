#include "signalgenwidget.h"

#include "ui_signalgenwidget.h"

SignalgeneratorWidget::SignalgeneratorWidget(AppWindow *window, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SignalgeneratorWidget),
    m_timerId(0),
    window(window)
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
       if(newval < DeviceDriver::getInfo(window->getDevice()).Limits.Generator.minFreq) {
           newval = DeviceDriver::getInfo(window->getDevice()).Limits.Generator.minFreq;
       } else if (newval > DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq) {
           newval = DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq;
       }
       ui->frequency->setValueQuiet(newval);
       if (newval < ui->span->value()/2)
           ui->span->setValueQuiet(newval/2);
       if (newval + ui->span->value()/2 > DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq)
           ui->span->setValueQuiet((DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq - newval)*2);
       newval = ui->frequency->value() - ui->span->value()/2;
       ui->current->setValueQuiet(newval);
       emit SettingsChanged();
    });

    connect(ui->span, &SIUnitEdit::valueChanged, [=](double newval) {
       if(newval < 0 ) {
           newval = 0;
       } else if (newval > DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq - DeviceDriver::getInfo(window->getDevice()).Limits.Generator.minFreq) {
           newval = DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq - DeviceDriver::getInfo(window->getDevice()).Limits.Generator.minFreq;
       }
       ui->span->setValueQuiet(newval);

       double newF = ui->frequency->value() - ui->span->value()/2;
       if (newF < 0) {
           ui->frequency->setValueQuiet(ui->span->value()/2);
       }
       newF = ui->frequency->value() + ui->span->value()/2;
       if (newF  > DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq) {
           ui->frequency->setValueQuiet(DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq - ui->span->value()/2);
       }

       newval = ui->frequency->value() - ui->span->value()/2;

       emit SettingsChanged();
    });

    connect(ui->current, &SIUnitEdit::valueChanged, [=](double newval) {
       if(newval < 0 ) {
           newval = 0;
       } else if (newval > DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq - DeviceDriver::getInfo(window->getDevice()).Limits.Generator.minFreq) {
           newval = DeviceDriver::getInfo(window->getDevice()).Limits.Generator.maxFreq - DeviceDriver::getInfo(window->getDevice()).Limits.Generator.minFreq;
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

    deviceInfoUpdated();

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

DeviceDriver::SGSettings SignalgeneratorWidget::getDeviceStatus()
{
    DeviceDriver::SGSettings s = {};
    if (ui->EnabledSweep->isChecked())
        s.freq = ui->current->value();
    else
        s.freq = ui->frequency->value();
    s.dBm = ui->levelSpin->value();
    s.port = 0;
    for(unsigned int i=0;i<portCheckboxes.size();i++) {
        if(portCheckboxes[i]->isChecked()) {
            s.port = i+1;
        }
    }
    return s;
}

nlohmann::json SignalgeneratorWidget::toJSON()
{
    auto s = getDeviceStatus();
    nlohmann::json j;
    j["frequency"] = s.freq;
    j["power"] = s.dBm;
    j["port"] = s.port;
    nlohmann::json sweep;
    sweep["span"] = ui->span->value();
    sweep["steps"] = ui->steps->value();
    sweep["dwell"] = ui->dwell->value();
    sweep["enabled"] = ui->EnabledSweep->isChecked();
    j["sweep"] = sweep;
    return j;
}

void SignalgeneratorWidget::fromJSON(nlohmann::json j)
{
    setFrequency(j.value("frequency", ui->frequency->value()));
    setLevel(j.value("power", ui->levelSpin->value()));
    setPort(j.value("port", 0));
    if(j.contains("sweep")) {
        auto sweep = j["sweep"];
        // extract sweep settings, keeping current values as default
        ui->span->setValue(sweep.value("span", ui->span->value()));
        ui->steps->setValue(sweep.value("steps", ui->steps->value()));
        ui->dwell->setValue(sweep.value("dwell", ui->dwell->value()));
        ui->EnabledSweep->setChecked(sweep.value("enabled", false));
    } else {
        ui->EnabledSweep->setChecked(false);
    }
}

void SignalgeneratorWidget::deviceInfoUpdated()
{
    auto port = getDeviceStatus().port;
    for(auto cb : portCheckboxes) {
        delete cb;
    }
    portCheckboxes.clear();
    auto info = DeviceDriver::getInfo(window->getDevice());
    for(unsigned int i=1;i<=info.Limits.Generator.ports;i++) {
        auto cb = new QCheckBox("Port "+QString::number(i));
        ui->portBox->layout()->addWidget(cb);
        portCheckboxes.push_back(cb);
        connect(cb, &QCheckBox::toggled, [=](bool enabled){
            if(enabled) {
                for(auto other : portCheckboxes) {
                    if(other != cb) {
                        other->blockSignals(true);
                        other->setChecked(false);
                        other->blockSignals(false);
                    }
                }
            }
            emit SettingsChanged();
        });
    }

    setFrequency(ui->frequency->value());
    setLevel(ui->levelSpin->value());

    setPort(port);

    ui->levelSlider->setMaximum(info.Limits.Generator.maxdBm * 100);
    ui->levelSlider->setMinimum(info.Limits.Generator.mindBm * 100);
    ui->levelSpin->setMaximum(info.Limits.Generator.maxdBm);
    ui->levelSpin->setMinimum(info.Limits.Generator.mindBm);
}

void SignalgeneratorWidget::setLevel(double level)
{
    auto info = DeviceDriver::getInfo(window->getDevice());
    if(level < info.Limits.Generator.mindBm) {
        level = info.Limits.Generator.mindBm;
    }
    if(level > info.Limits.Generator.maxdBm) {
        level = info.Limits.Generator.maxdBm;
    }
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
    auto info = DeviceDriver::getInfo(window->getDevice());
    if(frequency < info.Limits.Generator.minFreq) {
        frequency = info.Limits.Generator.minFreq;
    }
    if(frequency > info.Limits.Generator.maxFreq) {
        frequency = info.Limits.Generator.maxFreq;
    }
    ui->frequency->setValue(frequency);
}

void SignalgeneratorWidget::setPort(unsigned int port)
{
    if(port > portCheckboxes.size()) {
        return;
    }
    if(port == 0) {
        for(auto cb : portCheckboxes) {
            cb->setChecked(false);
        }
    } else {
        portCheckboxes[port-1]->setChecked(true);
    }
}

