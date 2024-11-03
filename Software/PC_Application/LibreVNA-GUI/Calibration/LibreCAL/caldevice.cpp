#include "caldevice.h"

#include "Util/util.h"

#include <QDebug>
#include <QDateTime>

using namespace std;

static QString getLocalDateTimeWithUtcOffset()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    int year = currentDateTime.date().year();
    int month = currentDateTime.date().month();
    int day = currentDateTime.date().day();
    int hour = currentDateTime.time().hour();
    int minute = currentDateTime.time().minute();
    int second = currentDateTime.time().second();
    int utcOffset = currentDateTime.offsetFromUtc() / 3600;
    int utcOffsetMinute = (currentDateTime.offsetFromUtc() % 3600) / 60;
    QString dateTimeString = QString("%1/%2/%3 %4:%5:%6 UTC%7%8:%9")
        .arg(year, 4, 10, QChar('0'))
        .arg(month, 2, 10, QChar('0'))
        .arg(day, 2, 10, QChar('0'))
        .arg(hour, 2, 10, QChar('0'))
        .arg(minute, 2, 10, QChar('0'))
        .arg(second, 2, 10, QChar('0'))
        .arg(utcOffset > 0 ? "+" : "-") // Add a plus sign for positive offsets
        .arg(qAbs(utcOffset), 2, 10, QChar('0'))
        .arg(utcOffsetMinute, 2, 10, QChar('0'));
    return dateTimeString;
}

CalDevice::CalDevice(QString serial) :
    usb(new USBDevice(serial))
{
    loadThread = nullptr;

    // Check device identification
    auto id = usb->Query("*IDN?");
    if(!id.startsWith("LibreCAL,")) {
        delete usb;
        throw std::runtime_error("Invalid response to *IDN?: "+id.toStdString());
    }

    firmware = usb->Query(":FIRMWARE?");
    QList<QString> fw_version = firmware.split(".");
    if (fw_version.size() == 3) {
        int firmware_major = fw_version[0].toInt();
        int firmware_minor = fw_version[1].toInt();
        this->firmware_major_minor = firmware_major + ((float)firmware_minor/10.0f);
        if(this->firmware_major_minor >= 0.2)
        {
            /* Set Date Time UTC */
            QString LocalDateTimeWithUtcOffset = getLocalDateTimeWithUtcOffset();
            QString ret = usb->Query(":DATE_TIME "+LocalDateTimeWithUtcOffset);
        }
    } else {
        // fw_version invalid
        this->firmware_major_minor = 0.0;
    }
    QString ports = usb->Query(":PORTS?");
    bool okay;
    numPorts = ports.toInt(&okay);
    if(!okay) {
        numPorts = 0;
    }
    connect(usb, &USBDevice::communicationFailure, this, &CalDevice::disconnected);
}

CalDevice::~CalDevice()
{
    delete usb;
}

QString CalDevice::StandardToString(CalDevice::Standard s)
{
    switch(s.type) {
    case Standard::Type::Open: return "OPEN";
    case Standard::Type::Short: return "SHORT";
    case Standard::Type::Load: return "LOAD";
    case Standard::Type::Through: return "THROUGH "+QString::number(s.throughDest);
    case Standard::Type::None: return "NONE";
    }
    return "Invalid";
}

CalDevice::Standard CalDevice::StandardFromString(QString s)
{
    for(auto standard : availableStandards()) {
        if(s == StandardToString(standard)) {
            return standard;
        }
    }
    return Standard(Standard::Type::None);
}

CalDevice::Standard CalDevice::getStandard(int port)
{
    auto query = ":PORT? "+QString::number(port);
    auto response = usb->Query(query);
    return StandardFromString(response);
}

bool CalDevice::setStandard(int port, CalDevice::Standard s)
{
    auto cmd = ":PORT "+QString::number(port)+" "+StandardToString(s);
    return usb->Cmd(cmd);
}

std::vector<CalDevice::Standard> CalDevice::availableStandards()
{
    return {Standard(Standard::Type::None), Standard(Standard::Type::Open), Standard(Standard::Type::Short), Standard(Standard::Type::Load), Standard(1), Standard(2), Standard(3), Standard(4)};
}

double CalDevice::getTemperature()
{
    QString tempString = usb->Query(":TEMP?");
    bool okay;
    double temp = tempString.toDouble(&okay);
    if(!okay) {
        temp = 0.0;
    }
    return temp;
}

bool CalDevice::stabilized()
{
    auto stable = usb->Query(":TEMPerature:STABLE?");
    return stable == "TRUE";
}

double CalDevice::getHeaterPower()
{
    QString tempString = usb->Query(":HEATER:POWER?");
    bool okay;
    double power = tempString.toDouble(&okay);
    if(!okay) {
        power = 0.0;
    }
    return power;
}

QString CalDevice::serial()
{
    return usb->serial();
}

QString CalDevice::getFirmware() const
{
    return firmware;
}

unsigned int CalDevice::getNumPorts() const
{
    return numPorts;
}

bool CalDevice::enterBootloader()
{
    return usb->Cmd(":BOOTloader");
}

QString CalDevice::getDateTimeUTC()
{
    if(this->firmware_major_minor >= 0.2)
    {
        return usb->Query(":DATE_TIME?");
    }else
    {
        return ""; // Not available
    }
}

void CalDevice::loadCoefficientSets(QStringList names, QList<int> ports, bool fast)
{
    coeffSets.clear();

    if(ports.isEmpty()) {
        for(unsigned int i=1;i<=getNumPorts();i++) {
            ports.append(i);
        }
    } else {
        // make sure we are sorted, otherwise the through coefficient naming is wrong
        std::sort(ports.begin(), ports.end());
    }

    abortLoading = false;
    if(fast && Util::firmwareEqualOrHigher(firmware, "0.2.1")) {
        loadThread = new std::thread(&CalDevice::loadCoefficientSetsThreadFast, this, names, ports);
    } else {
        loadThread = new std::thread(&CalDevice::loadCoefficientSetsThreadSlow, this, names, ports);
    }
}

void CalDevice::abortCoefficientLoading()
{
    if(loadThread) {
        abortLoading = true;
        loadThread->join();
        loadThread = nullptr;
    }
}

void CalDevice::saveCoefficientSets()
{
    if(!hasModifiedCoefficients()) {
        // nothing to do, already done
        emit updateCoefficientsDone(true);
    } else {
        new std::thread(&CalDevice::saveCoefficientSetsThread, this);
    }
}

void CalDevice::loadCoefficientSetsThreadSlow(QStringList names, QList<int> ports)
{
    QStringList coeffList = getCoefficientSetNames();
    if(coeffList.empty()) {
        // something went wrong
        emit updateCoefficientsDone(false);
        return;
    }
    if(names.size() > 0) {
        // check if all the requested names are actually available
        for(auto n : names) {
            if(!coeffList.contains(n)) {
                // this coefficient does not exist
                emit updateCoefficientsDone(false);
                return;
            }
        }
        coeffList = names;
    }
    // get total number of coefficient points for accurate percentage calculation
    unsigned long totalPoints = 0;
    for(auto name : coeffList) {
        for(int idx=0;idx<ports.size();idx++) {
            int i = ports[idx];
            if(abortLoading) {
                return;
            }
            totalPoints += usb->Query(":COEFF:NUM? "+name+" P"+QString::number(i)+"_OPEN").toInt();
            totalPoints += usb->Query(":COEFF:NUM? "+name+" P"+QString::number(i)+"_SHORT").toInt();
            totalPoints += usb->Query(":COEFF:NUM? "+name+" P"+QString::number(i)+"_LOAD").toInt();
            for(int jdx=idx+1;jdx<numPorts;jdx++) {
                int j = ports[jdx];
                totalPoints += usb->Query(":COEFF:NUM? "+name+" P"+QString::number(i)+QString::number(j)+"_THROUGH").toInt();
            }
        }
    }
    unsigned long readPoints = 0;
    int lastPercentage = 0;
    for(auto name : coeffList) {
        // create the coefficient set
        CoefficientSet set;
        set.name = name;
        set.ports = numPorts;
        // Read this coefficient set
        for(int idx=0;idx<ports.size();idx++) {
            int i = ports[idx];
            auto createCoefficient = [&](QString setName, QString paramName) -> CoefficientSet::Coefficient* {
                int points = usb->Query(":COEFF:NUM? "+setName+" "+paramName).toInt();
                CoefficientSet::Coefficient *c = new CoefficientSet::Coefficient();
                if(paramName.endsWith("THROUGH")) {
                    c->t = Touchstone(2);
                } else {
                    c->t = Touchstone(1);
                }
                for(int i=0;i<points;i++) {
                    if(abortLoading) {
                        break;
                    }
                    QString pString = usb->Query(":COEFF:GET? "+setName+" "+paramName+" "+QString::number(i));
                    QStringList values = pString.split(",");
                    Touchstone::Datapoint p;
                    p.frequency = values[0].toDouble() * 1e9;
                    for(int j = 0;j<(values.size()-1)/2;j++) {
                        double real = values[1+j*2].toDouble();
                        double imag = values[2+j*2].toDouble();
                        p.S.push_back(complex<double>(real, imag));
                    }
                    if(p.S.size() == 4) {
                        // S21 and S12 are swapped in the touchstone file order (S21 goes first)
                        // but Touchstone::AddDatapoint expects S11 S12 S21 S22 order. Swap to match that
                        swap(p.S[1], p.S[2]);
                    }
                    c->t.AddDatapoint(p);
                    readPoints++;
                    int newPercentage = readPoints * 100 / totalPoints;
                    if(newPercentage != lastPercentage) {
                        lastPercentage = newPercentage;
                        emit updateCoefficientsPercent(newPercentage);
                    }
                }
                c->t.setFilename("LibreCAL/"+paramName);
                return c;
            };
            set.opens[i] = (createCoefficient(name, "P"+QString::number(i)+"_OPEN"));
            set.shorts[i] = (createCoefficient(name, "P"+QString::number(i)+"_SHORT"));
            set.loads[i] = (createCoefficient(name, "P"+QString::number(i)+"_LOAD"));
            for(int jdx=idx+1;jdx<ports.size();jdx++) {
                int j = ports[jdx];
                set.throughs[set.portsToThroughIndex(i, j)] = createCoefficient(name, "P"+QString::number(i)+QString::number(j)+"_THROUGH");
            }
            if(abortLoading) {
                return;
            }
        }
        coeffSets.push_back(set);
    }
    emit updateCoefficientsDone(true);
}

void CalDevice::loadCoefficientSetsThreadFast(QStringList names, QList<int> ports)
{
    QStringList coeffList = getCoefficientSetNames();
    if(coeffList.empty()) {
        // something went wrong
        emit updateCoefficientsDone(false);
        return;
    }
    if(names.size() > 0) {
        // check if all the requested names are actually available
        for(auto n : names) {
            if(!coeffList.contains(n)) {
                // this coefficient does not exist
                emit updateCoefficientsDone(false);
                return;
            }
        }
        coeffList = names;
    }

    int total_coeffs = (ports.size() * 3 + ports.size() * (ports.size() - 1) / 2) * coeffList.size();
    int read_coeffs = 0;

    for(auto name : coeffList) {
        // create the coefficient set
        CoefficientSet set;
        set.name = name;
        set.ports = numPorts;

        auto createCoefficient = [&](QString setName, QString paramName) -> CoefficientSet::Coefficient* {
            CoefficientSet::Coefficient *c = new CoefficientSet::Coefficient();
            // ask for the whole set at once
            usb->flushReceived();
            usb->send(":COEFF:GET? "+setName+" "+paramName);
            // handle incoming lines
            if(paramName.endsWith("THROUGH")) {
                c->t = Touchstone(2);
            } else {
                c->t = Touchstone(1);
            }
            c->t.setFilename("LibreCAL/"+paramName);
            while(true) {
                QString line;
                if(!usb->receive(&line)) {
                    // failed to receive something, abort
                    return c;
                }
                if(line.startsWith("ERROR")) {
                    // something went wront
                    return c;
                }
                // ignore start, comments and option line
                if(line.startsWith("START") || line.startsWith("!") || line.startsWith("#")) {
                    // ignore
                    continue;
                }
                if(line.startsWith("END")) {
                    // got all data
                    return c;
                }
                // parse the values
                try {
                    QStringList values = line.split(" ");
                    Touchstone::Datapoint p;
                    p.frequency = values[0].toDouble() * 1e9;
                    for(int j = 0;j<(values.size()-1)/2;j++) {
                        double real = values[1+j*2].toDouble();
                        double imag = values[2+j*2].toDouble();
                        p.S.push_back(complex<double>(real, imag));
                    }
                    if(p.S.size() == 4) {
                        // S21 and S12 are swapped in the touchstone file order (S21 goes first)
                        // but Touchstone::AddDatapoint expects S11 S12 S21 S22 order. Swap to match that
                        swap(p.S[1], p.S[2]);
                    }
                    c->t.AddDatapoint(p);
                } catch (...) {
                    return c;
                }
            }
        };

        for(int idx=0;idx<ports.size();idx++) {
            int i = ports[idx];

            auto c = createCoefficient(name, "P"+QString::number(i)+"_OPEN");
            if(abortLoading) {
                return;
            }
            if(c) {
                set.opens[i] = c;
            }
            read_coeffs++;
            emit updateCoefficientsPercent(read_coeffs * 100 / total_coeffs);

            c = createCoefficient(name, "P"+QString::number(i)+"_SHORT");
            if(abortLoading) {
                return;
            }
            if(c) {
                set.shorts[i] = c;
            }
            read_coeffs++;
            emit updateCoefficientsPercent(read_coeffs * 100 / total_coeffs);

            c = createCoefficient(name, "P"+QString::number(i)+"_LOAD");
            if(abortLoading) {
                return;
            }
            if(c) {
                set.loads[i] = c;
            }
            read_coeffs++;
            emit updateCoefficientsPercent(read_coeffs * 100 / total_coeffs);

            for(int jdx=idx+1;jdx<ports.size();jdx++) {
                int j = ports[jdx];

                c = createCoefficient(name, "P"+QString::number(i)+QString::number(j)+"_THROUGH");
                if(abortLoading) {
                    return;
                }
                if(c) {
                    set.throughs[set.portsToThroughIndex(i, j)] = c;
                }
                read_coeffs++;
                emit updateCoefficientsPercent(read_coeffs * 100 / total_coeffs);
            }
        }

        coeffSets.push_back(set);
    }
    emit updateCoefficientsDone(true);
}

void CalDevice::saveCoefficientSetsThread()
{
    // figure out how many points need to be transferred
    unsigned long totalPoints = 0;
    for(auto set : coeffSets) {
        for(auto c : set.opens) {
            if(c.second->modified) {
                totalPoints += c.second->t.points();
            }
        }
        for(auto c : set.shorts) {
            if(c.second->modified) {
                totalPoints += c.second->t.points();
            }
        }
        for(auto c : set.loads) {
            if(c.second->modified) {
                totalPoints += c.second->t.points();
            }
        }
        for(auto c : set.throughs) {
            if(c.second->modified) {
                totalPoints += c.second->t.points();
            }
        }
    }
    unsigned long transferredPoints = 0;
    int lastPercentage = 0;
    bool success = true;
    for(auto set : coeffSets) {
        auto createCoefficient = [&](QString setName, QString paramName, Touchstone &t, bool &modified) -> bool {
            if(!modified) {
                // no changes, nothing to do
                return true;
            }
            int points = t.points();
            if(points > 0) {
                // create the file
                if(!usb->Cmd(":COEFF:CREATE "+setName+" "+paramName)) {
                    return false;
                }
                for(int i=0;i<points;i++) {
                    auto point = t.point(i);
                    if(point.S.size() == 4) {
                        // S parameters in point are in S11 S12 S21 S22 order but the LibreCAL expects
                        // S11 S21 S12 S22 (according to the two port touchstone format. Swap here.
                        swap(point.S[1], point.S[2]);
                    }
                    QString cmd = ":COEFF:ADD "+QString::number(point.frequency / 1000000000.0);
                    for(auto s : point.S) {
                        cmd += " "+QString::number(s.real())+" "+QString::number(s.imag());
                    }
                    if(!usb->Cmd(cmd)) {
                        return false;
                    }
                    transferredPoints++;
                    int newPercentage = transferredPoints * 100 / totalPoints;
                    if(newPercentage != lastPercentage) {
                        lastPercentage = newPercentage;
                        emit updateCoefficientsPercent(newPercentage);
                    }
                }
                if(!usb->Cmd(":COEFF:FIN")) {
                    return false;
                }
            } else {
                // no points, delete coefficient
                if(!usb->Cmd(":COEFF:DEL "+setName+" "+paramName)) {
                    return false;
                }
            }
            modified = false;
            return true;
        };
        for(int i=1;i<=numPorts;i++) {
            if(set.opens.count(i)) {
                success &= createCoefficient(set.name, "P"+QString::number(i)+"_OPEN", set.opens[i]->t, set.opens[i]->modified);
            }
            if(set.shorts.count(i)) {
                success &= createCoefficient(set.name, "P"+QString::number(i)+"_SHORT", set.shorts[i]->t, set.shorts[i]->modified);
            }
            if(set.loads.count(i)) {
                success &= createCoefficient(set.name, "P"+QString::number(i)+"_LOAD", set.loads[i]->t, set.loads[i]->modified);
            }
            for(int j=i+1;j<=numPorts;j++) {
                auto c = set.getThrough(i,j);
                if(c) {
                    success &= createCoefficient(set.name, "P"+QString::number(i)+QString::number(j)+"_THROUGH", c->t, c->modified);
                }
            }
        }
    }
    // prune empty coefficient sets
    auto i = coeffSets.begin();
    while(i != coeffSets.end()) {
        if(i->isEmpty()) {
            i = coeffSets.erase(i);
        } else {
            i++;
        }
    }
    emit updateCoefficientsDone(success);
}

std::vector<CalDevice::CoefficientSet> CalDevice::getCoefficientSets() const
{
    return coeffSets;
}

void CalDevice::addCoefficientSet(QString name)
{
    CoefficientSet set;
    set.name = name;
    set.ports = numPorts;
    set.createEmptyCoefficients();
    coeffSets.push_back(set);
}

QStringList CalDevice::getCoefficientSetNames()
{
    QString resp = usb->Query(":COEFF:LIST?");
    if(!resp.startsWith("FACTORY")) {
        return QStringList();
    }
    return resp.split(",");
}

bool CalDevice::hasModifiedCoefficients()
{
    for(auto set : coeffSets) {
        for(auto c : set.opens) {
            if(c.second->modified) {
                return true;
            }
        }
        for(auto c : set.shorts) {
            if(c.second->modified) {
                return true;
            }
        }
        for(auto c : set.loads) {
            if(c.second->modified) {
                return true;
            }
        }
        for(auto c : set.throughs) {
            if(c.second->modified) {
                return true;
            }
        }
    }
    return false;
}

CalDevice::CoefficientSet::Coefficient *CalDevice::CoefficientSet::getOpen(int port)
{
    if(opens.count(port)) {
        return opens[port];
    } else {
        return nullptr;
    }
}

CalDevice::CoefficientSet::Coefficient *CalDevice::CoefficientSet::getShort(int port)
{
    if(shorts.count(port)) {
        return shorts[port];
    } else {
        return nullptr;
    }
}

CalDevice::CoefficientSet::Coefficient *CalDevice::CoefficientSet::getLoad(int port)
{
    if(loads.count(port)) {
        return loads[port];
    } else {
        return nullptr;
    }
}

CalDevice::CoefficientSet::Coefficient *CalDevice::CoefficientSet::getThrough(int port1, int port2)
{
    auto index = portsToThroughIndex(port1, port2);
    if(throughs.count(index)) {
        return throughs[index];
    } else {
        return nullptr;
    }
}

int CalDevice::CoefficientSet::portsToThroughIndex(int port1, int port2)
{
    if(port1 > ports || port2 > ports || port1 >= port2) {
        return -1;
    }
    return port1 * ports + port2;
}

void CalDevice::CoefficientSet::portsFromThroughIndex(int &port1, int &port2, int index)
{
    port1 = index / ports;
    port2 = index - port1 * ports;
    if(port1 < 1 || port1 > ports) {
        port1 = -1;
    }
    if(port2 < 1 || port2 > ports) {
        port2 = -1;
    }
}

void CalDevice::CoefficientSet::createEmptyCoefficients()
{
    loads.clear();
    shorts.clear();
    opens.clear();
    throughs.clear();
    for(int i=1;i<=ports;i++) {
       opens[i] = new Coefficient();
       shorts[i] = new Coefficient();
       loads[i] = new Coefficient();
       for(int j=i+1;j<=ports;j++) {
           throughs[portsToThroughIndex(i,j)] = new Coefficient();
       }
    }
}

bool CalDevice::CoefficientSet::isEmpty()
{
    for(auto o : opens) {
        if(o.second->t.points() > 0) {
            return false;
        }
    }
    for(auto s : shorts) {
        if(s.second->t.points() > 0) {
            return false;
        }
    }
    for(auto l : loads) {
        if(l.second->t.points() > 0) {
            return false;
        }
    }
    for(auto t : throughs) {
        if(t.second->t.points() > 0) {
            return false;
        }
    }
    // no coefficients or all coefficients empty
    return true;
}
