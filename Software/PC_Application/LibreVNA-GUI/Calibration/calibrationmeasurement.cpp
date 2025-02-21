#include "calibrationmeasurement.h"
#include "unit.h"
#include "calibration.h"
#include "Util/util.h"

#include <QDateTime>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>

using namespace std;

CalibrationMeasurement::Base::Base(Calibration *cal)
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
        return true;
    }
    return false;
}

bool CalibrationMeasurement::Base::setStandard(CalStandard::Virtual *standard)
{
    if(standard) {
        if(supportedStandardTypes().count(standard->getType())) {
            // can use this standard
            this->standard = standard;
            connect(standard, &CalStandard::Virtual::deleted, this, [=](){
                setStandard(nullptr);
            });
            emit standardChanged(standard);
            return true;
        } else {
            // can't use this standard, leave unchanged
            return false;
        }
    } else {
        // nullptr passed, remove currently used standard
        if(this->standard) {
            disconnect(this->standard, &CalStandard::Virtual::deleted, this, nullptr);
        }
        this->standard = nullptr;
        emit standardChanged(nullptr);
        return true;
    }
}

QTableWidgetItem *CalibrationMeasurement::Base::getStatisticsItem()
{
    return new QTableWidgetItem(getStatistics());
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
    case Type::SlidingLoad: return "SlidingLoad";
    case Type::Reflect: return "Reflect";
    case Type::Through: return "Through";
    case Type::Isolation: return "Isolation";
    case Type::Line: return "Line";
    case Type::Last: return "Invalid";
    }
    return "Invalid";
}

CalibrationMeasurement::Base::Type CalibrationMeasurement::Base::TypeFromString(QString s)
{
    for(int i=0;i<(int) Type::Last;i++) {
        if(TypeToString((Type) i).compare(s, Qt::CaseInsensitive) == 0) {
            return (Type) i;
        }
    }
    return Type::Last;
}

QWidget *CalibrationMeasurement::Base::createStandardWidget()
{
    auto cbStandard = new QComboBox();
    for(auto s : supportedStandards()) {
        cbStandard->addItem(s->getDescription(), QVariant::fromValue((void*) s));
        if(standard == s) {
            cbStandard->setCurrentText(s->getDescription());
        }
    }
    if(standard == 0 && cbStandard->count() > 0) {
        // no standard was selected but no there is one available
        setStandard((CalStandard::Virtual*) cbStandard->itemData(0, Qt::UserRole).value<void*>());
    }

    connect(cbStandard, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        auto s = (CalStandard::Virtual*) cbStandard->itemData(cbStandard->currentIndex(), Qt::UserRole).value<void*>();
        setStandard(s);
    });
    connect(this, &CalibrationMeasurement::Base::standardChanged, cbStandard, [=](){
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
        auto standards = cal->getKit().getStandards();
        standard = nullptr;
        unsigned long long id = j.value("standard", 0ULL);
        for(auto s : standards) {
            if(s->getID() == id) {
                setStandard(s);
                break;
            }
        }
    }
    timestamp = QDateTime::fromSecsSinceEpoch(j.value("timestamp", 0));
}

bool CalibrationMeasurement::Base::canMeasureSimultaneously(std::set<CalibrationMeasurement::Base *> measurements)
{
    std::set<int> usedPorts;
    for(auto m : measurements) {
        std::vector<int> ports;
        switch(m->getType()) {
        case Type::Open:
        case Type::Short:
        case Type::Load:
        case Type::SlidingLoad:
        case Type::Reflect:
            // Uses one port
            ports.push_back(static_cast<OnePort*>(m)->getPort());
            break;
        case Type::Through:
        case Type::Line:
            // Uses two ports
            ports.push_back(static_cast<TwoPort*>(m)->getPort1());
            ports.push_back(static_cast<TwoPort*>(m)->getPort2());
            break;
        case Type::Isolation:
            // Uses all ports, unable to measure simultaneously
            if(measurements.size() > 1) {
                return false;
            }
            break;
        case Type::Last:
            // invalid
            return false;
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

QDateTime CalibrationMeasurement::Base::getTimestamp() const
{
    return timestamp;
}

CalStandard::Virtual* CalibrationMeasurement::Base::getStandard() const
{
    return standard;
}

QTableWidgetItem *CalibrationMeasurement::OnePort::getStatisticsItem()
{
    auto ret = Base::getStatisticsItem();
    if(numPoints() > 0) {
        if(!standard) {
            ret->setBackground(Qt::red);
            ret->setToolTip("No calibration standard assigned, unable to use this measurement");
        } else if(standard->minFrequency() > points.front().frequency || standard->maxFrequency() < points.back().frequency) {
            ret->setBackground(Qt::yellow);
            ret->setToolTip("Usable frequency range constrained by calibration standard to "+Unit::ToString(minUsableFreq(), "Hz", " kMG", 4)+" - "+Unit::ToString(maxUsableFreq(), "Hz", " kMG", 4));
        }
    }
    return ret;
}

double CalibrationMeasurement::OnePort::minUsableFreq()
{
    if(points.size() > 0 && standard) {
        return max(points.front().frequency, standard->minFrequency());
    } else {
        return numeric_limits<double>::max();
    }
}

double CalibrationMeasurement::OnePort::maxUsableFreq()
{
    if(points.size() > 0 && standard) {
        return min(points.back().frequency, standard->maxFrequency());
    } else {
        return 0;
    }
}

void CalibrationMeasurement::OnePort::clearPoints()
{
    points.clear();
    timestamp = QDateTime();
}

void CalibrationMeasurement::OnePort::addPoint(const DeviceDriver::VNAMeasurement &m)
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
    auto dev = DeviceDriver::getActiveDriver();
    if(dev) {
        if(port == 0) {
            setPort(1);
        }
        for(unsigned int i=1;i<=dev->getInfo().Limits.VNA.ports;i++) {
            cbPort->addItem(QString::number(i));
            if(port == i) {
                cbPort->setCurrentText(QString::number(i));
            }
        }
    }
    if(port > 0 && cbPort->findText(QString::number(port)) < 0) {
        // set already selected port, even if device is not connected or does not have this port
        cbPort->addItem(QString::number(port));
        cbPort->setCurrentText(QString::number(port));
    }
    connect(cbPort, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        setPort(cbPort->currentText().toInt());
    });
    connect(this, &OnePort::portChanged, cbPort, [=](){
        auto string = QString::number(port);
        if(cbPort->findText(string) < 0) {
            // setting does not exist yet, create (should not happen)
            cbPort->addItem(string);
        }
        cbPort->setCurrentText(string);
    });
    auto ret = new QWidget();
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addWidget(cbPort);
    layout->setStretch(1, 1);
    ret->setLayout(layout);
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
    Point interp = Util::interpolate<Point, double>(points, frequency, [](const Point &p)->double{
        return p.frequency;
    });
    return interp.S;
}

std::complex<double> CalibrationMeasurement::OnePort::getActual(double frequency)
{
    return static_cast<CalStandard::OnePort*>(standard)->toS11(frequency);
}

int CalibrationMeasurement::OnePort::getPort() const
{
    return port;
}

void CalibrationMeasurement::OnePort::setPort(unsigned int p)
{
    if(port != p) {
        port = p;
        emit portChanged(p);
    }
}

std::vector<CalibrationMeasurement::OnePort::Point> CalibrationMeasurement::OnePort::getPoints() const
{
    return points;
}

QTableWidgetItem *CalibrationMeasurement::TwoPort::getStatisticsItem()
{
    auto ret = Base::getStatisticsItem();
    if(numPoints() > 0) {
        if(!standard) {
            ret->setBackground(Qt::red);
            ret->setToolTip("No calibration standard assigned, unable to use this measurement");
        } else if(standard->minFrequency() > points.front().frequency || standard->maxFrequency() < points.back().frequency) {
            ret->setBackground(Qt::yellow);
            ret->setToolTip("Usable frequency range constrained by calibration standard to "+Unit::ToString(minUsableFreq(), "Hz", " kMG", 4)+" - "+Unit::ToString(maxUsableFreq(), "Hz", " kMG", 4));
        }
    }
    return ret;
}

double CalibrationMeasurement::TwoPort::minUsableFreq()
{
    if(points.size() > 0 && standard) {
        return max(points.front().frequency, standard->minFrequency());
    } else {
        return numeric_limits<double>::max();
    }
}

double CalibrationMeasurement::TwoPort::maxUsableFreq()
{
    if(points.size() > 0 && standard) {
        return min(points.back().frequency, standard->maxFrequency());
    } else {
        return 0;
    }
}

void CalibrationMeasurement::TwoPort::clearPoints()
{
    points.clear();
    timestamp = QDateTime();
}

void CalibrationMeasurement::TwoPort::addPoint(const DeviceDriver::VNAMeasurement &m)
{
    Point p;
    p.frequency = m.frequency;
    p.S = m.toSparam().reduceTo({port1, port2});
    points.push_back(p);
    timestamp = QDateTime::currentDateTimeUtc();
}

QWidget *CalibrationMeasurement::TwoPort::createSettingsWidget()
{
    auto label1 = new QLabel("From ");
    auto cbPort1 = new QComboBox();
    auto label2 = new QLabel(" to ");
    auto cbPort2 = new QComboBox();
    auto cbReverse = new QCheckBox("Reversed");
    cbReverse->setToolTip("Enable this option if the calibration standard is defined with the port order swapped");
    auto dev = DeviceDriver::getActiveDriver();
    if(dev) {
        if(port1 == 0) {
            setPort1(1);
        }
        if(port2 == 0) {
            setPort2(2);
        }
        for(unsigned int i=1;i<=dev->getInfo().Limits.VNA.ports;i++) {
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
    if(port1 > 0 && cbPort1->findText(QString::number(port1)) < 0) {
        // set already selected port, even if device is not connected or does not have this port
        cbPort1->addItem(QString::number(port1));
        cbPort1->setCurrentText(QString::number(port1));
    }
    if(port2 > 0 && cbPort2->findText(QString::number(port2)) < 0) {
        // set already selected port, even if device is not connected or does not have this port
        cbPort2->addItem(QString::number(port2));
        cbPort2->setCurrentText(QString::number(port2));
    }
    cbReverse->setChecked(reverseStandard);
    connect(cbPort1, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        setPort1(cbPort1->currentText().toInt());
    });
    connect(cbPort2, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        setPort2(cbPort2->currentText().toInt());
    });
    connect(cbReverse, &QCheckBox::toggled, [=](){
        setReverseStandard(cbReverse->isChecked());
    });
    connect(this, &TwoPort::port1Changed, cbPort1, [=](){
        auto string = QString::number(port1);
        if(cbPort1->findText(string) < 0) {
            // setting does not exist yet, create (should not happen)
            cbPort1->addItem(string);
        }
        cbPort1->setCurrentText(string);
    });
    connect(this, &TwoPort::port2Changed, cbPort2, [=](){
        auto string = QString::number(port2);
        if(cbPort2->findText(string) < 0) {
            // setting does not exist yet, create (should not happen)
            cbPort2->addItem(string);
        }
        cbPort2->setCurrentText(string);
    });
    connect(this, &TwoPort::reverseStandardChanged, cbReverse, [=](){
        cbReverse->setChecked(reverseStandard);
    });
    auto ret = new QWidget();
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label1);
    layout->addWidget(cbPort1);
    layout->addWidget(label2);
    layout->addWidget(cbPort2);
    layout->addWidget(cbReverse);
    layout->setStretch(1, 1);
    layout->setStretch(3, 1);
    ret->setLayout(layout);
    return ret;
}

nlohmann::json CalibrationMeasurement::TwoPort::toJSON()
{
    auto j = Base::toJSON();
    j["port1"] = port1;
    j["port2"] = port2;
    j["reverseStandard"] = reverseStandard;
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
    reverseStandard = j.value("reverseStandard", false);
    if(j.contains("points")) {
        for(auto jpoint : j["points"]) {
            Point p;
            p.frequency = jpoint.value("frequency", 0.0);
            p.S.fromJSON(jpoint["Sparam"]);
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
    Point interp = Util::interpolate<Point, double>(points, frequency, [](const Point &p)->double{
        return p.frequency;
    });
    return interp.S;
}

Sparam CalibrationMeasurement::TwoPort::getActual(double frequency)
{
    auto param = static_cast<CalStandard::TwoPort*>(standard)->toSparam(frequency);
    if(reverseStandard) {
        param.swapPorts(1, 2);
    }
    return param;
}

int CalibrationMeasurement::TwoPort::getPort2() const
{
    return port2;
}

void CalibrationMeasurement::TwoPort::setPort1(unsigned int p)
{
    if(port1 != p) {
        port1 = p;
        emit port1Changed(p);
    }
}

void CalibrationMeasurement::TwoPort::setPort2(unsigned int p)
{
    if(port2 != p) {
        port2 = p;
        emit port2Changed(p);
    }
}

void CalibrationMeasurement::TwoPort::setReverseStandard(bool reverse)
{
    if(reverseStandard != reverse) {
        reverseStandard = reverse;
        emit reverseStandardChanged(reverse);
    }
}

std::vector<CalibrationMeasurement::TwoPort::Point> CalibrationMeasurement::TwoPort::getPoints() const
{
    return points;
}

int CalibrationMeasurement::TwoPort::getPort1() const
{
    return port1;
}

unsigned int CalibrationMeasurement::Isolation::numPoints()
{
    return points.size();
}

void CalibrationMeasurement::Isolation::clearPoints()
{
    points.clear();
    timestamp = QDateTime();
}

void CalibrationMeasurement::Isolation::addPoint(const DeviceDriver::VNAMeasurement &m)
{
    Point p;
    p.frequency = m.frequency;
    for(auto &meas : m.measurements) {
        QString name = meas.first;
        unsigned int rcv = name.mid(1, 1).toInt() - 1;
        unsigned int src = name.mid(2, 1).toInt() - 1;
        if(rcv >= p.S.size()) {
            p.S.resize(rcv + 1);
        }
        if(src >= p.S[rcv].size()) {
            p.S[rcv].resize(src + 1);
        }
        p.S[rcv][src] = meas.second;
    }
    points.push_back(p);
    timestamp = QDateTime::currentDateTimeUtc();
}

QWidget *CalibrationMeasurement::Isolation::createStandardWidget()
{
    return new QLabel("Terminate all ports");
}

QWidget *CalibrationMeasurement::Isolation::createSettingsWidget()
{
    return new QLabel("No settings available");
}

nlohmann::json CalibrationMeasurement::Isolation::toJSON()
{
    auto j = Base::toJSON();
    nlohmann::json jpoints;
    for(auto &p : points) {
        nlohmann::json jpoint;
        jpoint["frequency"] = p.frequency;
        nlohmann::json jdest;
        for(auto dst : p.S) {
            nlohmann::json jsrc;
            for(auto src : dst) {
                nlohmann::json jiso;
                jiso["real"] = src.real();
                jiso["imag"] = src.imag();
                jsrc.push_back(jiso);
            }
            jdest.push_back(jsrc);
        }
        jpoint["S"] = jdest;
        jpoints.push_back(jpoint);
    }
    j["points"] = jpoints;
    return j;
}

void CalibrationMeasurement::Isolation::fromJSON(nlohmann::json j)
{
    clearPoints();
    Base::fromJSON(j);
    if(j.contains("points")) {
        for(auto jpoint : j["points"]) {
            Point p;
            p.frequency = jpoint.value("frequency", 0.0);
            if(jpoint.contains("S")) {
                for(auto jdest : jpoint["S"]) {
                    p.S.push_back(vector<complex<double>>());
                    for(auto jsrc : jdest) {
                        auto S = complex<double>(jsrc.value("real", 0.0), jsrc.value("imag", 0.0));
                        p.S.back().push_back(S);
                    }
                }
            }
            points.push_back(p);
        }
    }
}

std::complex<double> CalibrationMeasurement::Isolation::getMeasured(double frequency, unsigned int portRcv, unsigned int portSrc)
{
    if(points.size() == 0 || frequency < points.front().frequency || frequency > points.back().frequency) {
        return numeric_limits<complex<double>>::quiet_NaN();
    }
    portRcv--;
    portSrc--;
    // find correct point, interpolate
    Point interp = Util::interpolate<Point, double>(points, frequency, [](const Point &p)->double{
        return p.frequency;
    });
    if(portRcv >= interp.S.size() || portSrc >= interp.S[portRcv].size()) {
        return numeric_limits<complex<double>>::quiet_NaN();
    } else {
        return interp.S[portRcv][portSrc];
    }
}

std::vector<CalibrationMeasurement::Isolation::Point> CalibrationMeasurement::Isolation::getPoints() const
{
    return points;
}

QWidget *CalibrationMeasurement::SlidingLoad::createStandardWidget()
{
    return new QLabel("Connect sliding load");
}
