#include "calstandard.h"
#include "ui_CalStandardOpenEditDialog.h"
#include "ui_CalStandardShortEditDialog.h"
#include "ui_CalStandardLoadEditDialog.h"
#include "ui_CalStandardReflectEditDialog.h"
#include "ui_CalStandardThroughEditDialog.h"
#include "ui_CalStandardLineEditDialog.h"
#include "unit.h"
#include "Util/util.h"

using namespace std;
using namespace CalStandard;

Virtual::Virtual(QString name) :
    name(name),
    minFreq(std::numeric_limits<double>::lowest()),
    maxFreq(std::numeric_limits<double>::max())
{
    id = Util::random(numeric_limits<unsigned long long>::max());
}

Virtual *Virtual::create(Virtual::Type type)
{
    switch(type) {
    case Type::Open: return new Open;
    case Type::Short: return new Short;
    case Type::Load: return new Load;
    case Type::Reflect: return new Reflect;
    case Type::Through: return new Through;
    case Type::Line: return new Line;
    case Type::Last:
        break;
    }
    return nullptr;
}

std::vector<Virtual::Type> Virtual::availableTypes()
{
    std::vector<Type> ret;
    for(int i=0;i<(int) Type::Last;i++) {
        ret.push_back((Type) i);
    }
    return ret;
}

QString Virtual::TypeToString(Virtual::Type type)
{
    switch(type) {
    case Type::Open: return "Open";
    case Type::Short: return "Short";
    case Type::Load: return "Load";
    case Type::Reflect: return "Reflect";
    case Type::Through: return "Through";
    case Type::Line: return "Line";
    case Type::Last: return "Invalid";
    }
    return "Invalid";
}

Virtual::Type Virtual::TypeFromString(QString s)
{
    for(int i=0;i<(int) Type::Last;i++) {
        if(TypeToString((Type) i) == s) {
            return (Type) i;
        }
    }
    return Type::Last;
}

QString Virtual::getDescription()
{
    return TypeToString(getType())+", "+name;
}

nlohmann::json Virtual::toJSON()
{
    nlohmann::json j;
    j["name"] = name.toStdString();
    j["id"] = id;
    return j;
}

void Virtual::fromJSON(nlohmann::json j)
{
    name = QString::fromStdString(j.value("name", ""));
    id = j.value("id", id);
}

unsigned long long Virtual::getID()
{
    return id;
}

QString Virtual::getName() const
{
    return name;
}

void Virtual::setName(const QString &value)
{
    name = value;
}

void OnePort::setMeasurement(const Touchstone &ts, int port)
{
    if(!touchstone) {
        touchstone = new Touchstone(ts);
    } else {
        *touchstone = ts;
    }
    if(touchstone->ports() > 1) {
        touchstone->reduceTo1Port(port);
    }
    minFreq = touchstone->minFreq();
    maxFreq = touchstone->maxFreq();
}

void OnePort::clearMeasurement()
{
    delete touchstone;
    touchstone = nullptr;
    minFreq = std::numeric_limits<double>::lowest();
    maxFreq = std::numeric_limits<double>::max();
}

nlohmann::json OnePort::toJSON()
{
    auto j = Virtual::toJSON();
    if(touchstone) {
       j["touchstone"] = touchstone->toJSON();
    }
    return j;
}

void OnePort::fromJSON(nlohmann::json j)
{
    Virtual::fromJSON(j);
    if(j.contains("touchstone")) {
        Touchstone ts(1);
        ts.fromJSON(j["touchstone"]);
        setMeasurement(ts);
    }
}

Open::Open()
{
    Z0 = 50.0;
    delay = loss = C0 = C1 = C2 = C3 = 0.0;
}

std::complex<double> Open::toS11(double freq)
{
    if(touchstone) {
        return touchstone->interpolate(freq).S[0];
    } else {
        // calculate fringing capacitance for open
        double Cfringing = C0 * 1e-15 + C1 * 1e-27 * freq + C2 * 1e-36 * pow(freq, 2) + C3 * 1e-45 * pow(freq, 3);
        // convert to impedance
        complex<double> open;
        if (Cfringing == 0) {
            // special case to avoid issues with infinity
            open = complex<double>(1.0, 0);
        } else {
            auto imp_open = complex<double>(0, -1.0 / (freq * 2 * M_PI * Cfringing));
            open = (imp_open - complex<double>(50.0)) / (imp_open + complex<double>(50.0));
        }
        return Util::addTransmissionLine(open, Z0, delay*1e-12, loss*1e9, freq);
    }
}

void Open::edit(std::function<void(void)> finishedCallback)
{
    auto d = new QDialog;
    auto ui = new Ui::CalStandardOpenEditDialog;
    ui->setupUi(d);

    ui->name->setText(name);
    ui->Z0->setUnit("Ω");
    ui->Z0->setPrecision(4);
    ui->Z0->setValue(Z0);
    ui->delay->setValue(delay);
    ui->loss->setValue(loss);
    ui->C0->setValue(C0);
    ui->C1->setValue(C1);
    ui->C2->setValue(C2);
    ui->C3->setValue(C3);

    auto updateMeasurementLabel = [=](){
        QString label;
        if(touchstone) {
            label = QString::number(touchstone->points())+" points from "+Unit::ToString(touchstone->minFreq(), "Hz", " kMG")+" to "+Unit::ToString(touchstone->maxFreq(), "Hz", " kMG");
        } else {
            label = "No measurements stored yet";
        }
        ui->measurementLabel->setText(label);
    };

    QObject::connect(ui->coefficients, &QRadioButton::toggled, [=](bool checked) {
        if(checked) {
            clearMeasurement();
        }
        ui->stackedWidget->setCurrentIndex(checked ? 0 : 1);
    });
    QObject::connect(ui->measurement, &QRadioButton::toggled, [=](bool checked) {
        updateMeasurementLabel();
        ui->stackedWidget->setCurrentIndex(checked ? 1 : 0);
    });

    QObject::connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, ui->updateFile, &QPushButton::setEnabled);

    ui->touchstoneImport->setPorts(1);
    if(touchstone) {
        ui->measurement->setChecked(true);
        ui->touchstoneImport->setFile(touchstone->getFilename());
    } else {
        ui->coefficients->setChecked(true);
    }

    QObject::connect(ui->updateFile, &QPushButton::clicked, [=](){
        setMeasurement(ui->touchstoneImport->getTouchstone(), ui->touchstoneImport->getPorts()[0]);
        updateMeasurementLabel();
    });

    QObject::connect(d, &QDialog::accepted, [=](){
        name = ui->name->text();
        Z0 = ui->Z0->value();
        delay = ui->delay->value();
        loss = ui->loss->value();
        C0 = ui->C0->value();
        C1 = ui->C1->value();
        C2 = ui->C2->value();
        C3 = ui->C3->value();
        if(finishedCallback) {
            finishedCallback();
        }
    });

    d->show();
}

nlohmann::json Open::toJSON()
{
    auto j = OnePort::toJSON();
    j["Z0"] = Z0;
    j["delay"] = delay;
    j["loss"] = loss;
    j["C0"] = C0;
    j["C1"] = C1;
    j["C2"] = C2;
    j["C3"] = C3;
    return j;
}

void Open::fromJSON(nlohmann::json j)
{
    OnePort::fromJSON(j);
    Z0 = j.value("Z0", 50.0);
    delay = j.value("delay", 0.0);
    loss = j.value("loss", 0.0);
    C0 = j.value("C0", 0.0);
    C1 = j.value("C1", 0.0);
    C2 = j.value("C2", 0.0);
    C3 = j.value("C3", 0.0);
}

Short::Short()
{
    Z0 = 50.0;
    delay = loss = L0 = L1 = L2 = L3 = 0.0;
}

std::complex<double> Short::toS11(double freq)
{
    if(touchstone) {
        return touchstone->interpolate(freq).S[0];
    } else {
        // calculate inductance for short
        double Lseries = L0 * 1e-12 + L1 * 1e-24 * freq + L2 * 1e-33 * pow(freq, 2) + L3 * 1e-42 * pow(freq, 3);
        // convert to impedance
        auto imp_short = complex<double>(0, freq * 2 * M_PI * Lseries);
        complex<double> _short = (imp_short - complex<double>(50.0)) / (imp_short + complex<double>(50.0));
        return Util::addTransmissionLine(_short, Z0, delay*1e-12, loss*1e9, freq);
    }
}

void Short::edit(std::function<void(void)> finishedCallback)
{
    auto d = new QDialog;
    auto ui = new Ui::CalStandardShortEditDialog;
    ui->setupUi(d);

    ui->name->setText(name);
    ui->Z0->setUnit("Ω");
    ui->Z0->setPrecision(4);
    ui->Z0->setValue(Z0);
    ui->delay->setValue(delay);
    ui->loss->setValue(loss);
    ui->L0->setValue(L0);
    ui->L1->setValue(L1);
    ui->L2->setValue(L2);
    ui->L3->setValue(L3);

    auto updateMeasurementLabel = [=](){
        QString label;
        if(touchstone) {
            label = QString::number(touchstone->points())+" points from "+Unit::ToString(touchstone->minFreq(), "Hz", " kMG")+" to "+Unit::ToString(touchstone->maxFreq(), "Hz", " kMG");
        } else {
            label = "No measurements stored yet";
        }
        ui->measurementLabel->setText(label);
    };

    QObject::connect(ui->coefficients, &QRadioButton::toggled, [=](bool checked) {
        if(checked) {
            clearMeasurement();
        }
        ui->stackedWidget->setCurrentIndex(checked ? 0 : 1);
    });
    QObject::connect(ui->measurement, &QRadioButton::toggled, [=](bool checked) {
        updateMeasurementLabel();
        ui->stackedWidget->setCurrentIndex(checked ? 1 : 0);
    });

    QObject::connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, ui->updateFile, &QPushButton::setEnabled);

    ui->touchstoneImport->setPorts(1);
    if(touchstone) {
        ui->measurement->setChecked(true);
        ui->touchstoneImport->setFile(touchstone->getFilename());
    } else {
        ui->coefficients->setChecked(true);
    }

    QObject::connect(ui->updateFile, &QPushButton::clicked, [=](){
        setMeasurement(ui->touchstoneImport->getTouchstone(), ui->touchstoneImport->getPorts()[0]);
        updateMeasurementLabel();
    });

    QObject::connect(d, &QDialog::accepted, [=](){
        name = ui->name->text();
        Z0 = ui->Z0->value();
        delay = ui->delay->value();
        loss = ui->loss->value();
        L0 = ui->L0->value();
        L1 = ui->L1->value();
        L2 = ui->L2->value();
        L3 = ui->L3->value();
        if(finishedCallback) {
            finishedCallback();
        }
    });

    d->show();
}

nlohmann::json Short::toJSON()
{
    auto j = OnePort::toJSON();
    j["Z0"] = Z0;
    j["delay"] = delay;
    j["loss"] = loss;
    j["L0"] = L0;
    j["L1"] = L1;
    j["L2"] = L2;
    j["L3"] = L3;
    return j;
}

void Short::fromJSON(nlohmann::json j)
{
    OnePort::fromJSON(j);
    Z0 = j.value("Z0", 50.0);
    delay = j.value("delay", 0.0);
    loss = j.value("loss", 0.0);
    L0 = j.value("L0", 0.0);
    L1 = j.value("L1", 0.0);
    L2 = j.value("L2", 0.0);
    L3 = j.value("L3", 0.0);
}

Load::Load()
{
    Z0 = 50.0;
    resistance = 50.0;
    delay = Cparallel = Lseries = 0;
    Cfirst = true;
}

std::complex<double> Load::toS11(double freq)
{
    if(touchstone) {
        return touchstone->interpolate(freq).S[0];
    } else {
        auto imp_load = complex<double>(resistance, 0);
        if (Cfirst) {
            // C is the first parameter starting from the VNA port. But the load is modeled here starting from
            // the other end, so we need to start with the inductor
            imp_load += complex<double>(0, freq * 2 * M_PI * Lseries);
        }
        // Add parallel capacitor to impedance
        if(Cparallel > 0) {
            auto imp_C = complex<double>(0, -1.0 / (freq * 2 * M_PI * Cparallel));
            imp_load = (imp_load * imp_C) / (imp_load + imp_C);
        }
        if (!Cfirst) {
            // inductor not added yet, do so now
            imp_load += complex<double>(0, freq * 2 * M_PI * Lseries);
        }
        complex<double> load = (imp_load - complex<double>(50.0)) / (imp_load + complex<double>(50.0));
        return Util::addTransmissionLine(load, Z0, delay*1e-12, loss*1e9, freq);
    }
}

void Load::edit(std::function<void(void)> finishedCallback)
{
    auto d = new QDialog;
    auto ui = new Ui::CalStandardLoadEditDialog;
    ui->setupUi(d);

    ui->name->setText(name);
    ui->resistance->setUnit("Ω");
    ui->resistance->setPrecision(4);
    ui->resistance->setValue(resistance);
    ui->Z0->setUnit("Ω");
    ui->Z0->setPrecision(4);
    ui->Z0->setValue(Z0);
    ui->delay->setValue(delay);
    ui->loss->setValue(loss);
    ui->parC->setUnit("F");
    ui->parC->setPrefixes("pnum ");
    ui->parC->setPrecision(4);
    ui->parC->setValue(Cparallel);
    ui->serL->setUnit("H");
    ui->serL->setPrefixes("num ");
    ui->serL->setPrecision(4);
    ui->serL->setValue(Lseries);

    if(Cfirst) {
        ui->C_first->setChecked(true);
    } else {
        ui->L_first->setChecked(true);
    }

    auto updateMeasurementLabel = [=](){
        QString label;
        if(touchstone) {
            label = QString::number(touchstone->points())+" points from "+Unit::ToString(touchstone->minFreq(), "Hz", " kMG")+" to "+Unit::ToString(touchstone->maxFreq(), "Hz", " kMG");
        } else {
            label = "No measurements stored yet";
        }
        ui->measurementLabel->setText(label);
    };

    QObject::connect(ui->coefficients, &QRadioButton::toggled, [=](bool checked) {
        if(checked) {
            clearMeasurement();
        }
        ui->stackedWidget->setCurrentIndex(checked ? 0 : 1);
    });
    QObject::connect(ui->measurement, &QRadioButton::toggled, [=](bool checked) {
        updateMeasurementLabel();
        ui->stackedWidget->setCurrentIndex(checked ? 1 : 0);
    });

    QObject::connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, ui->updateFile, &QPushButton::setEnabled);

    ui->touchstoneImport->setPorts(1);
    if(touchstone) {
        ui->measurement->setChecked(true);
        ui->touchstoneImport->setFile(touchstone->getFilename());
    } else {
        ui->coefficients->setChecked(true);
    }

    QObject::connect(ui->updateFile, &QPushButton::clicked, [=](){
        setMeasurement(ui->touchstoneImport->getTouchstone(), ui->touchstoneImport->getPorts()[0]);
        updateMeasurementLabel();
    });

    QObject::connect(d, &QDialog::accepted, [=](){
        name = ui->name->text();
        resistance = ui->resistance->value();
        Z0 = ui->Z0->value();
        delay = ui->delay->value();
        loss = ui->loss->value();
        Cparallel = ui->parC->value();
        Lseries = ui->serL->value();
        Cfirst = ui->C_first->isChecked();
        if(finishedCallback) {
            finishedCallback();
        }
    });

    d->show();
}

nlohmann::json Load::toJSON()
{
    auto j = OnePort::toJSON();
    j["Z0"] = Z0;
    j["delay"] = delay;
    j["loss"] = loss;
    j["resistance"] = resistance;
    j["Cparallel"] = Cparallel;
    j["Lseries"] = Lseries;
    j["Cfirst"] = Cfirst;
    return j;
}

void Load::fromJSON(nlohmann::json j)
{
    OnePort::fromJSON(j);
    Z0 = j.value("Z0", 50.0);
    delay = j.value("delay", 0.0);
    loss = j.value("loss", 0.0);
    resistance = j.value("resistance", 0.0);
    Cparallel = j.value("Cparallel", 0.0);
    Lseries = j.value("Lseries", 0.0);
    Cfirst = j.value("Cfirst", true);
}

void TwoPort::setMeasurement(const Touchstone &ts, int port1, int port2)
{
    if(!touchstone) {
        touchstone = new Touchstone(ts);
    } else {
        *touchstone = ts;
    }
    if(touchstone->ports() > 2) {
        touchstone->reduceTo2Port(port1, port2);
    }
    minFreq = touchstone->minFreq();
    maxFreq = touchstone->maxFreq();
}

void TwoPort::clearMeasurement()
{
    delete touchstone;
    touchstone = nullptr;
    minFreq = std::numeric_limits<double>::lowest();
    maxFreq = std::numeric_limits<double>::max();
}

nlohmann::json TwoPort::toJSON()
{
    auto j = Virtual::toJSON();
    if(touchstone) {
       j["touchstone"] = touchstone->toJSON();
    }
    return j;
}

void TwoPort::fromJSON(nlohmann::json j)
{
    Virtual::fromJSON(j);
    if(j.contains("touchstone")) {
        Touchstone ts(1);
        ts.fromJSON(j["touchstone"]);
        setMeasurement(ts);
    }
}

Through::Through()
{
    Z0 = 50.0;
    delay = 0.0;
    loss = 0.0;
}

Sparam Through::toSparam(double freq)
{
    if(touchstone) {
        auto interp = touchstone->interpolate(freq).S;
        return Sparam(interp[0], interp[1], interp[2], interp[3]);
    } else {
        // calculate effect of through
        // nomenclature and formulas from https://loco.lab.asu.edu/loco-memos/edges_reports/report_20130807.pdf
        auto w = 2.0 * M_PI * freq;
        auto f_sqrt = sqrt(freq / 1e9);
        auto offset_impedance = Z0;         // lossless characteristic impedance
        auto offset_loss = loss * 1e9;
        auto offset_delay = delay * 1e-12;

        auto Z_c = std::complex<double>(offset_impedance + (offset_loss / (2*w)) * f_sqrt, -(offset_loss / (2*w)) * f_sqrt);
        auto gamma_l = std::complex<double>(offset_loss*offset_delay/(2*offset_impedance)*f_sqrt, w*offset_delay+offset_loss*offset_delay/(2*offset_impedance)*f_sqrt);
        auto p = exp(-gamma_l); // propagation factor

        auto Z_r = std::complex<double>(50.0);
        auto Gamma = (Z_c - Z_r) / (Z_c + Z_r);

        // S-parameter of a transmission line, see e.g. "Specifying Calibration Standards and Kits for Keysight Vector Network Analyzers"
        // Keysight application note equations (6) and (7)
        auto Sxx = Gamma * (1.0 - p*p) / (1.0 - p*p*Gamma*Gamma);
        auto Sxy = p * (1.0 - Gamma*Gamma) / (1.0 - p*p*Gamma*Gamma);
        return Sparam(Sxx, Sxy, Sxy, Sxx);
    }
}

void Through::edit(std::function<void(void)> finishedCallback)
{
    auto d = new QDialog;
    auto ui = new Ui::CalStandardThroughEditDialog;
    ui->setupUi(d);

    ui->name->setText(name);
    ui->Z0->setUnit("Ω");
    ui->Z0->setPrecision(4);
    ui->Z0->setValue(Z0);
    ui->delay->setValue(delay);
    ui->loss->setValue(loss);

    auto updateMeasurementLabel = [=](){
        QString label;
        if(touchstone) {
            label = QString::number(touchstone->points())+" points from "+Unit::ToString(touchstone->minFreq(), "Hz", " kMG")+" to "+Unit::ToString(touchstone->maxFreq(), "Hz", " kMG");
        } else {
            label = "No measurements stored yet";
        }
        ui->measurementLabel->setText(label);
    };

    QObject::connect(ui->coefficients, &QRadioButton::toggled, [=](bool checked) {
        if(checked) {
            clearMeasurement();
        }
        ui->stackedWidget->setCurrentIndex(checked ? 0 : 1);
    });
    QObject::connect(ui->measurement, &QRadioButton::toggled, [=](bool checked) {
        updateMeasurementLabel();
        ui->stackedWidget->setCurrentIndex(checked ? 1 : 0);
    });

    QObject::connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, ui->updateFile, &QPushButton::setEnabled);

    ui->touchstoneImport->setPorts(2);
    if(touchstone) {
        ui->measurement->setChecked(true);
        ui->touchstoneImport->setFile(touchstone->getFilename());
    } else {
        ui->coefficients->setChecked(true);
    }

    QObject::connect(ui->updateFile, &QPushButton::clicked, [=](){
        setMeasurement(ui->touchstoneImport->getTouchstone(), ui->touchstoneImport->getPorts()[0], ui->touchstoneImport->getPorts()[1]);
        updateMeasurementLabel();
    });

    QObject::connect(d, &QDialog::accepted, [=](){
        name = ui->name->text();
        Z0 = ui->Z0->value();
        delay = ui->delay->value();
        loss = ui->loss->value();
        if(finishedCallback) {
            finishedCallback();
        }
    });

    d->show();
}

nlohmann::json Through::toJSON()
{
    auto j = TwoPort::toJSON();
    j["Z0"] = Z0;
    j["delay"] = delay;
    j["loss"] = loss;
    return j;
}

void Through::fromJSON(nlohmann::json j)
{
    TwoPort::fromJSON(j);
    Z0 = j.value("Z0", 50.0);
    delay = j.value("delay", 0.0);
    loss = j.value("loss", 0.0);
}

Reflect::Reflect()
{
    isShort = true;
}

std::complex<double> Reflect::toS11(double freq)
{
    Q_UNUSED(freq)
    return std::numeric_limits<complex<double>>::quiet_NaN();
}

void Reflect::edit(std::function<void ()> finishedCallback)
{
    auto d = new QDialog;
    auto ui = new Ui::CalStandardReflectEditDialog;
    ui->setupUi(d);

    ui->name->setText(name);
    ui->type->setCurrentIndex(isShort ? 1 : 0);

    QObject::connect(d, &QDialog::accepted, [=](){
        name = ui->name->text();
        isShort = ui->type->currentIndex() == 1;
        if(finishedCallback) {
            finishedCallback();
        }
    });

    d->show();
}

nlohmann::json Reflect::toJSON()
{
    auto j = OnePort::toJSON();
    j["isShort"] = isShort;
    return j;
}

void Reflect::fromJSON(nlohmann::json j)
{
    OnePort::fromJSON(j);
    isShort = j.value("isShort", true);
}

bool Reflect::getIsShort() const
{
    return isShort;
}

Line::Line()
{
    Z0 = 50.0;
    setDelay(0.0);
}

Sparam Line::toSparam(double freq)
{
    Q_UNUSED(freq)
    Sparam ret;
    ret.set(1,1,numeric_limits<complex<double>>::quiet_NaN());
    ret.set(1,2,numeric_limits<complex<double>>::quiet_NaN());
    ret.set(2,1,numeric_limits<complex<double>>::quiet_NaN());
    ret.set(2,2,numeric_limits<complex<double>>::quiet_NaN());
    return ret;
}

void Line::edit(std::function<void ()> finishedCallback)
{
    auto d = new QDialog;
    auto ui = new Ui::CalStandardLineEditDialog;
    ui->setupUi(d);

    ui->name->setText(name);
    ui->Z0->setUnit("Ω");
    ui->Z0->setPrecision(4);
    ui->Z0->setValue(Z0);
    ui->delay->setValue(delay);
    ui->delay->setUnit("s");
    ui->delay->setPrefixes("pnum ");
    ui->delay->setPrecision(4);
    ui->minFreq->setUnit("Hz");
    ui->minFreq->setPrecision(4);
    ui->minFreq->setPrefixes(" kMG");
    ui->minFreq->setValue(minFreq);
    ui->maxFreq->setUnit("Hz");
    ui->maxFreq->setPrecision(4);
    ui->maxFreq->setPrefixes(" kMG");
    ui->maxFreq->setValue(maxFreq);

    QObject::connect(ui->delay, &SIUnitEdit::valueChanged, [=](double val){
        ui->minFreq->setValue(1.0 / val * 20 / 360);
        ui->maxFreq->setValue(1.0 / val * 160 / 360);
    });

    QObject::connect(d, &QDialog::accepted, [=](){
        name = ui->name->text();
        Z0 = ui->Z0->value();
        setDelay(ui->delay->value());
        if(finishedCallback) {
            finishedCallback();
        }
    });

    d->show();
}

nlohmann::json Line::toJSON()
{
    auto j = TwoPort::toJSON();
    j["delay"] = delay;
    return j;
}

void Line::fromJSON(nlohmann::json j)
{
    TwoPort::fromJSON(j);
    setDelay(j.value("delay", 0.0));
}

void Line::setDelay(double delay)
{
    this->delay = delay;
    minFreq = 1.0 / delay * 20 / 360;
    maxFreq = 1.0 / delay * 160 / 360;
}
