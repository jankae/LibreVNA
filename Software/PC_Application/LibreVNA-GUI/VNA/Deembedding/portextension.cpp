#include "portextension.h"

#include "ui_portextensioneditdialog.h"
#include "Util/util.h"
#include "appwindow.h"

#include <QCheckBox>
#include <cmath>
#include <QDebug>

using namespace std;

PortExtension::PortExtension()
    : DeembeddingOption("PORTEXTension")
{
    ext.frequency = 0;
    ext.loss = 0;
    ext.DCloss = 0;
    ext.delay = 0;
    ext.velocityFactor = 0.66;

    port = 1;
    isIdeal = true;
    isOpen = true;

    kit = nullptr;
    ui = nullptr;

    addUnsignedIntParameter("PORT", port);
    addDoubleParameter("DELAY", ext.delay);
    addDoubleParameter("DCLOSS", ext.DCloss);
    addDoubleParameter("LOSS", ext.loss);
    addDoubleParameter("FREQuency", ext.frequency);
}

std::set<unsigned int> PortExtension::getAffectedPorts()
{
    return {port};
}

void PortExtension::transformDatapoint(DeviceDriver::VNAMeasurement &d)
{
    auto phase = -2 * M_PI * ext.delay * d.frequency;
    auto db_attennuation = ext.DCloss;
    if(ext.frequency != 0) {
        db_attennuation += ext.loss * sqrt(d.frequency / ext.frequency);
    }
    // convert from db to factor
    auto att = pow(10.0, -db_attennuation / 20.0);
    auto correction = polar<double>(att, phase);
    for(auto &m : d.measurements) {
        if(m.first.mid(1, 1).toUInt() == port) {
            // selected port is the destination of this S parameter
            m.second /= correction;
        }
        if(m.first.mid(2, 1).toUInt() == port) {
            // selected port is the source of this S parameter
            m.second /= correction;
        }
    }
}

void PortExtension::edit()
{
    constexpr double c = 299792458;

    auto dialog = new QDialog();
    ui = new Ui::PortExtensionEditDialog();
    ui->setupUi(dialog);
    connect(dialog, &QDialog::finished, [=](){
        delete ui;
        ui = nullptr;
    });

    // set initial values
    ui->Time->setUnit("s");
    ui->Time->setPrefixes("pnum ");
    ui->Distance->setUnit("m");
    ui->Distance->setPrefixes("m ");
    ui->DCloss->setUnit("db");
    ui->Loss->setUnit("db");
    ui->Frequency->setUnit("Hz");
    ui->Frequency->setPrefixes(" kMG");
    ui->Time->setValue(ext.delay);
    ui->Velocity->setValue(ext.velocityFactor);
    ui->Distance->setValue(ext.delay * ext.velocityFactor * c);
    ui->DCloss->setValue(ext.DCloss);
    ui->Loss->setValue(ext.loss);
    ui->Frequency->setValue(ext.frequency);
    ui->port->setValue(port);
    ui->port->setMaximum(DeviceDriver::maximumSupportedPorts);
    if(!kit) {
        ui->calkit->setEnabled(false);
    }

    auto updateValuesFromUI = [=](){
        ext.delay = ui->Time->value();
        ext.velocityFactor = ui->Velocity->value();
        ext.DCloss = ui->DCloss->value();
        ext.loss = ui->Loss->value();
        ext.frequency = ui->Frequency->value();
    };

    // connections to link delay and distance
    connect(ui->Time, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->Distance->setValueQuiet(newval * ui->Velocity->value() * c);
        updateValuesFromUI();
    });
    connect(ui->Distance, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->Time->setValueQuiet(newval / (ui->Velocity->value() * c));
        updateValuesFromUI();
    });
    connect(ui->Velocity, &SIUnitEdit::valueChanged, [=](double newval) {
        ui->Time->setValueQuiet(ui->Distance->value() / (newval * c));
        updateValuesFromUI();
    });
    connect(ui->port, qOverload<int>(&QSpinBox::valueChanged), [=](){
        port = ui->port->value();
    });
    connect(ui->DCloss, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->Loss, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->Frequency, &SIUnitEdit::valueChanged, updateValuesFromUI);
    connect(ui->_short, &QPushButton::pressed, [=](){
        isOpen = false;
        isIdeal = ui->ideal->isChecked();
        startMeasurement();
    });
    connect(ui->open, &QPushButton::pressed, [=](){
        isOpen = true;
        isIdeal = ui->ideal->isChecked();
        startMeasurement();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    if(AppWindow::showGUI()) {
        dialog->show();
    }
}

void PortExtension::measurementCompleted(std::vector<DeviceDriver::VNAMeasurement> m)
{
    if(m.size() > 0) {
        double last_phase = 0.0;
        double phasediff_sum = 0.0;
        vector<double> att_x, att_y;
        double avg_x = 0.0, avg_y = 0.0;
        for(auto p : m) {
            // grab correct measurement
            QString name = "S"+QString::number(port)+QString::number(port);
            auto reflection = p.measurements[name];
            // remove calkit if specified
            if(!isIdeal) {
                complex<double> calStandard = 1.0;
                auto comp = isOpen ? CalStandard::Virtual::Type::Open : CalStandard::Virtual::Type::Short;
                for(auto s : kit->getStandards()) {
                    if(s->getType() == comp) {
                        calStandard = static_cast<CalStandard::OnePort*>(s)->toS11(p.frequency);
                    }
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
            }

            double x = sqrt(p.frequency / m.back().frequency);
            double y = Util::SparamTodB(reflection);
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
        ui->Time->setValue(delay);
        ui->DCloss->setValue(DCloss);
        ui->Loss->setValue(loss);
        ui->Frequency->setValue(freq);
    }
}

void PortExtension::startMeasurement()
{
    emit triggerMeasurement();
}

void PortExtension::setCalkit(Calkit *kit)
{
    this->kit = kit;
}

nlohmann::json PortExtension::toJSON()
{
    nlohmann::json j;
    j["port"] = port;
    j["delay"] = ext.delay;
    j["velocityFactor"] = ext.velocityFactor;
    j["DCloss"] = ext.DCloss;
    j["loss"] = ext.loss;
    j["frequency"] = ext.frequency;
    return j;
}

void PortExtension::fromJSON(nlohmann::json j)
{
    nlohmann::json jfrom;
    if(j.contains("port")) {
        // new format
        jfrom = j;
        port = j.value("port", 1);
    } else {
        jfrom = j[0];
        port = 1;
    }
    ext.delay = jfrom.value("delay", 0.0);
    ext.velocityFactor = jfrom.value("velocityFactor", 0.66);
    ext.DCloss = jfrom.value("DCloss", 0.0);
    ext.loss = jfrom.value("loss", 0.0);
    ext.frequency = jfrom.value("frequency", 6000000000);
}
