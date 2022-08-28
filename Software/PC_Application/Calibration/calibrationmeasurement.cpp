#include "calibrationmeasurement.h"
#include "unit.h"
#include "calibration2.h"

#include <QDateTime>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

using namespace std;

CalibrationMeasurement::Base::Base(Calibration2 *cal)
    : cal(cal)
{
    standard = nullptr;
    timestamp = QDateTime();
}

std::vector<CalStandard::Virtual *> CalibrationMeasurement::Base::supportedStandards()
{
    vector<CalStandard::Virtual*> ret;
    for(auto s : cal->getKit().getStandards()) {
        if(supportedStandardTypes().count(s->getType())) {
            ret.push_back(s);
        }
    }
    return ret;
}

bool CalibrationMeasurement::Base::setFirstSupportedStandard()
{
    // assign first valid standard
    auto supported = supportedStandards();
    if(supported.size() > 0) {
        setStandard(supported[0]);
    }
}

bool CalibrationMeasurement::Base::setStandard(CalStandard::Virtual *standard)
{
    if(standard) {
        if(supportedStandardTypes().count(standard->getType())) {
            // can use this standard
            this->standard = standard;
            emit standardChanged(standard);
            return true;
        } else {
            // can't use this standard, leave unchanged
            return false;
        }
    } else {
        // nullptr passed, remove currently used standard
        this->standard = nullptr;
        emit standardChanged(nullptr);
        return true;
    }
}

QString CalibrationMeasurement::Base::getStatistics()
{
    if(numPoints() > 0) {
        QString data = QString::number(numPoints());
        data.append(" points from ");
        data.append(Unit::ToString(minFreq(), "Hz", " kMG"));
        data.append(" to ");
        data.append(Unit::ToString(maxFreq(), "Hz", " kMG"));
        return data;
    } else {
        return "Not available";
    }
}

std::vector<CalibrationMeasurement::Base::Type> CalibrationMeasurement::Base::availableTypes()
{
    std::vector<Type> ret;
    for(int i=0;i<(int) Type::Last;i++) {
        ret.push_back((Type) i);
    }
    return ret;
}

QString CalibrationMeasurement::Base::TypeToString(CalibrationMeasurement::Base::Type type)
{
    switch(type) {
    case Type::Open: return "Open";
    case Type::Short: return "Short";
    case Type::Load: return "Load";
    case Type::Through: return "Through";
    case Type::Last: return "Invalid";
    }
}

CalibrationMeasurement::Base::Type CalibrationMeasurement::Base::TypeFromString(QString s)
{
    for(int i=0;i<(int) Type::Last;i++) {
        if(TypeToString((Type) i) == s) {
            return (Type) i;
        }
    }
    return Type::Last;
}

QWidget *CalibrationMeasurement::Base::createStandardWidget()
{
    auto cbStandard = new QComboBox();
    for(auto s : supportedStandards()) {
        cbStandard->addItem(s->getDescription(), qVariantFromValue((void*) s));
        if(standard == s) {
            cbStandard->setCurrentText(s->getDescription());
        }
    }

    connect(cbStandard, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        auto s = (CalStandard::Virtual*) cbStandard->itemData(cbStandard->currentIndex(), Qt::UserRole).value<void*>();
        setStandard(s);
    });
    connect(this, &CalibrationMeasurement::Base::standardChanged, [=](){
        for(int i=0;i<cbStandard->count();i++) {
            if((CalStandard::Virtual*) cbStandard->itemData(i, Qt::UserRole).value<void*>() == standard) {
                cbStandard->setCurrentIndex(i);
            }
        }
    });
    return cbStandard;
}

nlohmann::json CalibrationMeasurement::Base::toJSON()
{
    nlohmann::json j;
    if(standard) {
        j["standard"] = standard->getID();
    }
    j["timestamp"] = timestamp.toSecsSinceEpoch();
    return j;
}

void CalibrationMeasurement::Base::fromJSON(nlohmann::json j)
{
    if(j.contains("standard")) {
        // TODO find standard from ID
    }
    timestamp = QDateTime::fromSecsSinceEpoch(j.value("timestamp", 0));
}

bool CalibrationMeasurement::Base::canMeasureSimultaneously(std::vector<CalibrationMeasurement::Base *> measurements)
{
    std::set<int> usedPorts;
    for(auto m : measurements) {
        std::vector<int> ports;
        switch(m->getType()) {
        case Type::Open:
        case Type::Short:
        case Type::Load:
            // Uses one port
            ports.push_back(static_cast<OnePort*>(m)->getPort());
            break;
        case Type::Through:
            // Uses two ports
            ports.push_back(static_cast<TwoPort*>(m)->getPort1());
            ports.push_back(static_cast<TwoPort*>(m)->getPort2());
            break;
        }
        for(auto p : ports) {
            if(usedPorts.count(p)) {
                // port already used for another measurement
                return false;
            } else {
                usedPorts.insert(p);
            }
        }
    }
    // if we get here, no port collisions occurred
    return true;
}

double CalibrationMeasurement::OnePort::minFreq()
{
    if(points.size() > 0) {
        return points.front().frequency;
    } else {
        return numeric_limits<double>::max();
    }
}

double CalibrationMeasurement::OnePort::maxFreq()
{
    if(points.size() > 0) {
        return points.back().frequency;
    } else {
        return 0;
    }
}

void CalibrationMeasurement::OnePort::clearPoints()
{
    points.clear();
    timestamp = QDateTime();
}

void CalibrationMeasurement::OnePort::addPoint(const VirtualDevice::VNAMeasurement &m)
{
    QString measurementName = "S"+QString::number(port)+QString::number(port);
    if(m.measurements.count(measurementName) > 0) {
        Point p;
        p.frequency = m.frequency;
        p.S = m.measurements.at(measurementName);
        points.push_back(p);
        timestamp = QDateTime::currentDateTimeUtc();
    }
}

QWidget *CalibrationMeasurement::OnePort::createSettingsWidget()
{
    auto label = new QLabel("Port:");
    auto cbPort = new QComboBox();
    auto dev = VirtualDevice::getConnected();
    if(dev) {
        for(int i=1;i<=dev->getInfo().ports;i++) {
            cbPort->addItem(QString::number(i));
            if(port == i) {
                cbPort->setCurrentText(QString::number(i));
            }
        }
    }
    connect(cbPort, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        setPort(cbPort->currentText().toInt());
    });
    connect(this, &OnePort::portChanged, [=](){
        auto string = QString::number(port);
        if(cbPort->findText(string) < 0) {
            // setting does not exist yet, create (should not happen)
            cbPort->addItem(string);
        }
        cbPort->setCurrentText(string);
    });
    auto ret = new QWidget();
    ret->setLayout(new QHBoxLayout);
    ret->layout()->addWidget(label);
    ret->layout()->addWidget(cbPort);
    return ret;
}

nlohmann::json CalibrationMeasurement::OnePort::toJSON()
{
    auto j = Base::toJSON();
    j["port"] = port;
    nlohmann::json jpoints;
    for(auto &p : points) {
        nlohmann::json jpoint;
        jpoint["frequency"] = p.frequency;
        jpoint["real"] = p.S.real();
        jpoint["imag"] = p.S.imag();
        jpoints.push_back(jpoint);
    }
    j["points"] = jpoints;
    return j;
}

void CalibrationMeasurement::OnePort::fromJSON(nlohmann::json j)
{
    clearPoints();
    Base::fromJSON(j);
    port = j.value("port", 0);
    if(j.contains("points")) {
        for(auto jpoint : j["points"]) {
            Point p;
            p.frequency = jpoint.value("frequency", 0.0);
            p.S = complex<double>(jpoint.value("real", 0.0), jpoint.value("imag", 0.0));
            points.push_back(p);
        }
    }
}

std::complex<double> CalibrationMeasurement::OnePort::getMeasured(double frequency)
{
    if(points.size() == 0 || frequency < points.front().frequency || frequency > points.back().frequency) {
        return numeric_limits<complex<double>>::quiet_NaN();
    }
    // frequency within points, interpolate
    auto lower = lower_bound(points.begin(), points.end(), frequency, [](const Point &lhs, double rhs) -> bool {
        return lhs.frequency < rhs;
    });
    auto lowPoint = *lower;
    advance(lower, 1);
    auto highPoint = *lower;
    double alpha = (frequency - lowPoint.frequency) / (highPoint.frequency - lowPoint.frequency);
    complex<double> ret;
    return lowPoint.S * (1.0 - alpha) + highPoint.S * alpha;
}

std::complex<double> CalibrationMeasurement::OnePort::getActual(double frequency)
{
    return static_cast<CalStandard::OnePort*>(standard)->toS11(frequency);
}

int CalibrationMeasurement::OnePort::getPort() const
{
    return port;
}

int CalibrationMeasurement::OnePort::setPort(int p)
{
    if(port != p) {
        port = p;
        emit portChanged(p);
    }
}

double CalibrationMeasurement::TwoPort::minFreq()
{
    if(points.size() > 0) {
        return points.front().frequency;
    } else {
        return numeric_limits<double>::max();
    }
}

double CalibrationMeasurement::TwoPort::maxFreq()
{
    if(points.size() > 0) {
        return points.back().frequency;
    } else {
        return 0;
    }
}

void CalibrationMeasurement::TwoPort::clearPoints()
{
    points.clear();
    timestamp = QDateTime();
}

void CalibrationMeasurement::TwoPort::addPoint(const VirtualDevice::VNAMeasurement &m)
{
    Point p;
    p.frequency = m.frequency;
    p.S = m.toSparam(port1, port2);
    points.push_back(p);
    timestamp = QDateTime::currentDateTimeUtc();
}

QWidget *CalibrationMeasurement::TwoPort::createSettingsWidget()
{
    auto label1 = new QLabel("From port ");
    auto cbPort1 = new QComboBox();
    auto label2 = new QLabel(" to port ");
    auto cbPort2 = new QComboBox();
    auto dev = VirtualDevice::getConnected();
    if(dev) {
        for(int i=1;i<=dev->getInfo().ports;i++) {
            cbPort1->addItem(QString::number(i));
            cbPort2->addItem(QString::number(i));
            if(port1 == i) {
                cbPort1->setCurrentText(QString::number(i));
            }
            if(port2 == i) {
                cbPort2->setCurrentText(QString::number(i));
            }
        }
    }
    connect(cbPort1, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        setPort1(cbPort1->currentText().toInt());
    });
    connect(cbPort2, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        setPort2(cbPort2->currentText().toInt());
    });
    connect(this, &TwoPort::port1Changed, [=](){
        auto string = QString::number(port1);
        if(cbPort1->findText(string) < 0) {
            // setting does not exist yet, create (should not happen)
            cbPort1->addItem(string);
        }
        cbPort1->setCurrentText(string);
    });
    connect(this, &TwoPort::port2Changed, [=](){
        auto string = QString::number(port2);
        if(cbPort2->findText(string) < 0) {
            // setting does not exist yet, create (should not happen)
            cbPort2->addItem(string);
        }
        cbPort2->setCurrentText(string);
    });
    auto ret = new QWidget();
    ret->setLayout(new QHBoxLayout);
    ret->layout()->addWidget(label1);
    ret->layout()->addWidget(cbPort1);
    ret->layout()->addWidget(label2);
    ret->layout()->addWidget(cbPort2);
    return ret;
}

nlohmann::json CalibrationMeasurement::TwoPort::toJSON()
{
    auto j = Base::toJSON();
    j["port1"] = port1;
    j["port2"] = port2;
    nlohmann::json jpoints;
    for(auto &p : points) {
        nlohmann::json jpoint;
        jpoint["frequency"] = p.frequency;
        jpoint["Sparam"] = p.S.toJSON();
        jpoints.push_back(jpoint);
    }
    j["points"] = jpoints;
    return j;
}

void CalibrationMeasurement::TwoPort::fromJSON(nlohmann::json j)
{
    clearPoints();
    Base::fromJSON(j);
    port1 = j.value("port1", 0);
    port2 = j.value("port2", 0);
    if(j.contains("points")) {
        for(auto jpoint : j["points"]) {
            Point p;
            p.frequency = jpoint.value("frequency", 0.0);
            p.S.fromJSON(j["Sparam"]);
            points.push_back(p);
        }
    }
}

Sparam CalibrationMeasurement::TwoPort::getMeasured(double frequency)
{
    if(points.size() == 0 || frequency < points.front().frequency || frequency > points.back().frequency) {
        return Sparam();
    }
    // frequency within points, interpolate
    auto lower = lower_bound(points.begin(), points.end(), frequency, [](const Point &lhs, double rhs) -> bool {
        return lhs.frequency < rhs;
    });
    auto lowPoint = *lower;
    advance(lower, 1);
    auto highPoint = *lower;
    double alpha = (frequency - lowPoint.frequency) / (highPoint.frequency - lowPoint.frequency);
    Sparam ret;
    ret.m11 = lowPoint.S.m11 * (1.0 - alpha) + highPoint.S.m11 * alpha;
    ret.m12 = lowPoint.S.m12 * (1.0 - alpha) + highPoint.S.m12 * alpha;
    ret.m21 = lowPoint.S.m21 * (1.0 - alpha) + highPoint.S.m21 * alpha;
    ret.m22 = lowPoint.S.m22 * (1.0 - alpha) + highPoint.S.m22 * alpha;
    return ret;
}

Sparam CalibrationMeasurement::TwoPort::getActual(double frequency)
{
    return static_cast<CalStandard::TwoPort*>(standard)->toSparam(frequency);
}

int CalibrationMeasurement::TwoPort::getPort2() const
{
    return port2;
}

int CalibrationMeasurement::TwoPort::setPort1(int p)
{
    if(port1 = p) {
        port1 = p;
        emit port1Changed(p);
    }
}

int CalibrationMeasurement::TwoPort::setPort2(int p)
{
    if(port1 = p) {
        port1 = p;
        emit port1Changed(p);
    }
}

int CalibrationMeasurement::TwoPort::getPort1() const
{
    return port1;
}
