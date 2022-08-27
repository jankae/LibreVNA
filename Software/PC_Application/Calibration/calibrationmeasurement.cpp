#include "calibrationmeasurement.h"
#include "unit.h"
#include "calibration2.h"

#include <QDateTime>

using namespace std;

CalibrationMeasurement::Base::Base(Calibration2 *cal)
    : cal(cal)
{
    standard = nullptr;
    timestamp = QDateTime();
}

bool CalibrationMeasurement::Base::setFirstSupportedStandard()
{
    // assign first valid standard
    for(auto s : cal->getKit().getStandards()) {
        if(supportedStandards().count(s->getType())) {
            setStandard(s);
            break;
        }
    }
}

bool CalibrationMeasurement::Base::setStandard(CalStandard::Virtual *standard)
{
    if(standard) {
        if(supportedStandards().count(standard->getType())) {
            // can use this standard
            this->standard = standard;
            return true;
        } else {
            // can't use this standard, leave unchanged
            return false;
        }
    } else {
        // nullptr passed, remove currently used standard
        this->standard = nullptr;
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

int CalibrationMeasurement::TwoPort::getPort1() const
{
    return port1;
}
