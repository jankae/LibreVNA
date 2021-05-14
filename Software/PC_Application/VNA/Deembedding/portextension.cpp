#include "portextension.h"
#include "ui_portextensioneditdialog.h"
#include <QCheckBox>
#include <cmath>
#include <QDebug>
#include "unit.h"

using namespace std;

PortExtension::PortExtension()
    : DeembeddingOption()
{
    port1.enabled = false;
    port1.frequency = 0;
    port1.loss = 0;
    port1.DCloss = 0;
    port1.delay = 0;
    port1.velocityFactor = 0.66;
    port2.enabled = false;
    port2.frequency = 0;
    port2.loss = 0;
    port2.DCloss = 0;
    port2.delay = 0;
    port2.velocityFactor = 0.66;

    kit = nullptr;
}

void PortExtension::transformDatapoint(Protocol::Datapoint &d)
{
    if(port1.enabled || port2.enabled) {
        // Convert measurements to complex variables
        auto S11 = complex<double>(d.real_S11, d.imag_S11);
        auto S21 = complex<double>(d.real_S21, d.imag_S21);
        auto S22 = complex<double>(d.real_S22, d.imag_S22);
        auto S12 = complex<double>(d.real_S12, d.imag_S12);

        if(port1.enabled) {
            auto phase = -2 * M_PI * port1.delay * d.frequency;
            auto db_attennuation = port1.DCloss;
            if(port1.frequency != 0) {
                db_attennuation += port1.loss * sqrt(d.frequency / port1.frequency);
            }
            // convert from db to factor
            auto att = pow(10.0, -db_attennuation / 20.0);
            auto correction = polar<double>(att, phase);
            S11 /= correction * correction;
            S21 /= correction;
            S12 /= correction;
        }
        if(port2.enabled) {
            auto phase = -2 * M_PI * port2.delay * d.frequency;
            auto db_attennuation = port2.DCloss;
            if(port2.frequency != 0) {
                db_attennuation += port2.loss * sqrt(d.frequency / port2.frequency);
            }
            // convert from db to factor
            auto att = pow(10.0, -db_attennuation / 20.0);
            auto correction = polar<double>(att, phase);
            S22 /= correction * correction;
            S21 /= correction;
            S12 /= correction;
        }
        d.real_S11 = S11.real();
        d.imag_S11 = S11.imag();
        d.real_S12 = S12.real();
        d.imag_S12 = S12.imag();
        d.real_S21 = S21.real();
        d.imag_S21 = S21.imag();
        d.real_S22 = S22.real();
        d.imag_S22 = S22.imag();
    }
}

void PortExtension::edit()
{
    constexpr double c = 299792458;

    auto dialog = new QDialog();
    ui = new Ui::PortExtensionEditDialog();
    ui->setupUi(dialog);

    // set initial values
    ui->P1Enabled->setChecked(port1.enabled);
    ui->P1Time->setUnit("s");
    ui->P1Time->setPrefixes("pnum ");
    ui->P1Distance->setUnit("m");
    ui->P1Distance->setPrefixes("m ");
    ui->P1DCloss->setUnit("db");
    ui->P1Loss->setUnit("db");
    ui->P1Frequency->setUnit("Hz");
    ui->P1Frequency->setPrefixes(" kMG");
    ui->P1Time->setValue(port1.delay);
    ui->P1Velocity->setValue(port1.velocityFactor);
    ui->P1Distance->setValue(port1.delay * port1.velocityFactor * c);
    ui->P1DCloss->setValue(port1.DCloss);
    ui->P1Loss->setValue(port1.loss);
    ui->P1Frequency->setValue(port1.frequency);
    if(!kit) {
        ui->P1calkit->setEnabled(false);
    }

    ui->P2Enabled->setChecked(port2.enabled);
    ui->P2Time->setUnit("s");
    ui->P2Time->setPrefixes("pnum ");
    ui->P2Distance->setUnit("m");
    ui->P2Distance->setPrefixes("m ");
    ui->P2DCloss->setUnit("db");
    ui->P2Loss->setUnit("db");
    ui->P2Frequency->setUnit("Hz");
    ui->P2Frequency->setPrefixes(" kMG");
    ui->P2Time->setValue(port2.delay);
    ui->P2Velocity->setValue(port2.velocityFactor);
    ui->P2Distance->setValue(port2.delay * port2.velocityFactor * c);
    ui->P2DCloss->setValue(port2.DCloss);
    ui->P2Loss->setValue(port2.loss);
    ui->P2Frequency->setValue(port2.frequency);
    if(!kit) {
        ui->P2calkit->setEnabled(false);
    }

    auto updateValuesFromUI = [=](){
        port1.delay = ui->P1Time->value();
        port1.velocityFactor = ui->P1Velocity->value();
        port1.DCloss = ui->P1DCloss->value();
        port1.loss = ui->P1Loss->value();
        port1.frequency = ui->P1Frequency->value();
        port2.delay = ui->P2Time->value();
        port2.velocityFactor = ui->P2Velocity->value();
        port2.DCloss = ui->P2DCloss->value();
        port2.loss = ui->P2Loss->value();
        port2.frequency = ui->P2Frequency->value();
    };

    connect(ui->P1Enabled, &QCheckBox::toggled, [=](bool enabled) {
        port1.enabled = enabled;
    });
    // connections to link delay and distance
    connect(ui->P1Time, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->P1Distance->setValueQuiet(newval * ui->P1Velocity->value() * c);
        updateValuesFromUI();
    });
    connect(ui->P1Distance, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->P1Time->setValueQuiet(newval / (ui->P1Velocity->value() * c));
        updateValuesFromUI();
    });
    connect(ui->P1Velocity, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->P1Time->setValueQuiet(ui->P1Distance->value() / (newval * c));
        updateValuesFromUI();
    });
    connect(ui->P1DCloss, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->P1Loss, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->P1Frequency, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->P1short, &QPushButton::pressed, [=](){
        isOpen = false;
        isPort1 = true;
        isIdeal = ui->P1ideal->isChecked();
        startMeasurement();
    });
    connect(ui->P1open, &QPushButton::pressed, [=](){
        isOpen = true;
        isPort1 = true;
        isIdeal = ui->P1ideal->isChecked();
        startMeasurement();
    });

    connect(ui->P2Enabled, &QCheckBox::toggled, [=](bool enabled) {
        port2.enabled = enabled;
    });
    connect(ui->P2Time, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->P2Distance->setValueQuiet(newval * ui->P2Velocity->value() * c);
        updateValuesFromUI();
    });
    connect(ui->P2Distance, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->P2Time->setValueQuiet(newval / (ui->P2Velocity->value() * c));
        updateValuesFromUI();
    });
    connect(ui->P2Velocity, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->P2Time->setValueQuiet(ui->P2Distance->value() / (newval * c));
        updateValuesFromUI();
    });
    connect(ui->P2DCloss, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->P2Loss, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->P2Frequency, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->P2short, &QPushButton::pressed, [=](){
        isOpen = false;
        isPort1 = false;
        isIdeal = ui->P2ideal->isChecked();
        startMeasurement();
    });
    connect(ui->P2open, &QPushButton::pressed, [=](){
        isOpen = true;
        isPort1 = false;
        isIdeal = ui->P2ideal->isChecked();
        startMeasurement();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    dialog->show();
}

void PortExtension::measurementCompleted(std::vector<Protocol::Datapoint> m)
{
    if(m.size() > 0) {
        double last_phase = 0.0;
        double phasediff_sum = 0.0;
        vector<double> att_x, att_y;
        double avg_x = 0.0, avg_y = 0.0;
        for(auto p : m) {
            // grab correct measurement
            complex<double> reflection;
            if(isPort1) {
                reflection = complex<double>(p.real_S11, p.imag_S11);
            } else {
                reflection = complex<double>(p.real_S22, p.imag_S22);
            }
            // remove calkit if specified
            if(!isIdeal) {
                complex<double> calStandard;
                auto standards = kit->toSOLT(p.frequency);
                if(isOpen) {
                    calStandard = standards.Open;
                } else {
                    calStandard = standards.Short;
                }
                // remove effect of calibration standard
                reflection /= calStandard;
            }
            // sum phase differences to previous point
            auto phase = arg(reflection);
            if(p.pointNum == 0) {
                last_phase = phase;
            } else {
                auto phasediff = phase - last_phase;
                last_phase = phase;
                if(phasediff > M_PI) {
                    phasediff -= 2 * M_PI;
                } else if(phasediff <= -M_PI) {
                    phasediff += 2 * M_PI;
                }
                phasediff_sum += phasediff;
                qDebug() << phasediff;
            }

            double x = sqrt(p.frequency / m.back().frequency);
            double y = Unit::dB(reflection);
            att_x.push_back(x);
            att_y.push_back(y);
            avg_x += x;
            avg_y += y;
        }
        auto phase = phasediff_sum / (m.size() - 1);
        auto freq_diff = m[1].frequency - m[0].frequency;
        auto delay = -phase / (2 * M_PI * freq_diff);
        // measured delay is two-way but port extension expects one-way
        delay /= 2;

        // calculate linear regression with transformed square root model
        avg_x /= m.size();
        avg_y /= m.size();
        double sum_top = 0.0;
        double sum_bottom = 0.0;
        for(unsigned int i=0;i<att_x.size();i++) {
            sum_top += (att_x[i] - avg_x)*(att_y[i] - avg_y);
            sum_bottom += (att_x[i] - avg_x)*(att_x[i] - avg_x);
        }
        double beta = sum_top / sum_bottom;
        double alpha = avg_y - beta * avg_x;

        double DCloss = -alpha / 2;
        double loss = -beta / 2;
        double freq = m.back().frequency;
        if(isPort1) {
            ui->P1Time->setValue(delay);
            ui->P1DCloss->setValue(DCloss);
            ui->P1Loss->setValue(loss);
            ui->P1Frequency->setValue(freq);
        } else {
            ui->P2Time->setValue(delay);
            ui->P2DCloss->setValue(DCloss);
            ui->P2Loss->setValue(loss);
            ui->P2Frequency->setValue(freq);
        }
    }
}

void PortExtension::startMeasurement()
{
    emit triggerMeasurement(isPort1, false, false, !isPort1);
}

void PortExtension::setCalkit(Calkit *kit)
{
    this->kit = kit;
}

nlohmann::json PortExtension::toJSON()
{
    nlohmann::json j;
    for(int i=0;i<2;i++) {
        auto ext = i == 0 ? port1 : port2;
        nlohmann::json je;
        je["enabled"] = ext.enabled;
        je["delay"] = ext.delay;
        je["velocityFactor"] = ext.velocityFactor;
        je["DCloss"] = ext.DCloss;
        je["loss"] = ext.loss;
        je["frequency"] = ext.frequency;
        j.push_back(je);
    }
    return j;
}

void PortExtension::fromJSON(nlohmann::json j)
{
    for(int i=0;i<2;i++) {
        Extension ext;
        nlohmann::json je = j[i];
        ext.enabled = je.value("enabled", false);
        ext.delay = je.value("delay", 0.0);
        ext.velocityFactor = je.value("velocityFactor", 0.66);
        ext.DCloss = je.value("DCloss", 0.0);
        ext.loss = je.value("loss", 0.0);
        ext.frequency = je.value("frequency", 6000000000);
        if(i==0) {
            port1 = ext;
        } else {
            port2 = ext;
        }
    }
}
