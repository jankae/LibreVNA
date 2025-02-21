#include "caldevice.h"

#include "Util/util.h"
#include "ui_factoryUpdateDialog.h"
#include "Util/QMicroz/qmicroz.h"
#include "CustomWidgets/informationbox.h"

#include <QDebug>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QFileDialog>

using namespace std;

static const QStringList factoryProblemSerials = {
    "5mNkR0s+NzMA", "5mNkR0s2DTcA", "5mNkR0s2gzcA", "5mNkR0s2HTMA", "5mNkR0s2STMA",
    "5mNkR0s3pzcA", "5mNkR0s8ijcA", "5mNkR0s9jjMA", "5mNkR0sGdCsA", "5mNkR0slqjEA",
    "5mNkR0srVDMA", "5mNkR0sssjcA", "5mNkR0suHDcA", "5mNkR0svOTcA", "5mNkR0svrzcA",
    "5mNkR0swkTcA", "5mNkR0s_JzMA", "5mNkR0t+HTUA", "5mNkR0t+JzcA", "5mNkR0t+mjUA",
    "5mNkR0t0IzIA", "5mNkR0t0TjAA", "5mNkR0t1bTAA", "5mNkR0t1cjUA", "5mNkR0t1HDAA",
    "5mNkR0t2oTIA", "5mNkR0t2rTAA", "5mNkR0t2uyoA", "5mNkR0t3HC0A", "5mNkR0t3HjIA",
    "5mNkR0t3MS0A", "5mNkR0t3vC0A", "5mNkR0t3VzUA", "5mNkR0t4aS0A", "5mNkR0t4nDUA",
    "5mNkR0t5fDcA", "5mNkR0t6cDcA", "5mNkR0t9kzcA", "5mNkR0t9OTcA", "5mNkR0t9UTUA",
    "5mNkR0tbPTUA", "5mNkR0tbqTUA", "5mNkR0tcHzUA", "5mNkR0tctDUA", "5mNkR0tcwjcA",
    "5mNkR0tdrDUA", "5mNkR0telDUA", "5mNkR0tgaTkA", "5mNkR0tgRzcA", "5mNkR0tgWTkA",
    "5mNkR0tikTUA", "5mNkR0tivDcA", "5mNkR0tIXS8A", "5mNkR0tLOTkA", "5mNkR0tqtTkA",
    "5mNkR0tshS0A", "5mNkR0tsnioA", "5mNkR0tSOzQA", "5mNkR0tSqjQA", "5mNkR0tThjUA",
    "5mNkR0tTxTUA", "5mNkR0tuVS0A", "5mNkR0tvGC0A", "5mNkR0tvYyoA", "5mNkR0tWljIA",
    "5mNkR0tXMDIA", "5mNkR0t_UzUA", "5mNkR0uBiC0A", "5mNkR0uBNS0A", "5mNkR0uCPTYA",
    "5mNkR0uCrTYA", "5mNkR0uDgDIA", "5mNkR0uFjTUA", "5mNkR0uFTjUA", "5mNkR0ugbCoA",
    "5mNkR0uGIy0A", "5mNkR0uGkzUA", "5mNkR0uGpi0A", "5mNkR0uGRC0A", "5mNkR0uGrjUA",
    "5mNkR0uGSzUA", "5mNkR0uGVCoA", "5mNkR0uHIyoA", "5mNkR0uHJjUA", "5mNkR0uHmC0A",
    "5mNkR0uHpzUA", "5mNkR0uHTzUA", "5mNkR0uIdzUA", "5mNkR0uIPDYA", "5mNkR0uIpy0A",
    "5mNkR0uIqioA", "5mNkR0uISioA", "5mNkR0uITjUA", "5mNkR0uIVi0A", "5mNkR0uJejYA",
    "5mNkR0uJSDUA", "5mNkR0uKNDYA", "5mNkR0uKnzYA", "5mNkR0uLfTIA", "5mNkR0uLPjIA",
    "5mNkR0uMbTIA", "5mNkR0uNpy0A", "5mNkR0uOTzAA", "5mNkR0uPhTAA", "5mOUNss0EysA",
    "5mOUNss1XisA", "5mOUNsseICwA", "5mOUNssiNiwA", "5mOUNssnISwA", "5mOUNssnIyUA",
    "5mOUNssrmCQA", "5mOUNssRmicA", "5mOUNssUlycA", "5mOUNssURCcA", "5mOUNssvjyQA",
    "5mOUNssvNSUA", "5mOUNssVXSwA", "5mOUNsszOiQA", "5mOUNst9liQA", "5mOUNstAhSsA",
    "5mOUNstFSysA", "5mOUNstFwCsA", "5mOUNstGmCUA", "5mOUNstHaCUA", "5mOUNstJkScA",
    "5mOUNstOxysA", "5mOUNstsYiIA", "5mOUNstYRi0A", "5mOUNsuHqioA", "5mOUNsuhVSgA",
    "5mOUNsuJLSoA", "5mOUNsuJrioA", "5mOUNsuVdCcA", "5mNkR0s1sjMA", "5mNkR0s3MjMA",
    "5mNkR0s3XDcA", "5mNkR0slWjEA", "5mNkR0sxkC8A", "5mNkR0t+bzcA", "5mNkR0t1azIA",
    "5mNkR0taKzUA", "5mNkR0tbbTcA", "5mNkR0tgGTkA", "5mNkR0thGDcA", "5mNkR0thiTcA",
    "5mNkR0thKDcA", "5mNkR0tRYzQA", "5mNkR0tWBTIA", "5mNkR0uCpDYA", "5mNkR0uDXDYA",
    "5mNkR0uJUDUA", "5mNkR0uLfjYA", "5mNkR0uMfjIA",
};

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
    transferActive = false;

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
    connect(this, &CalDevice::updateCoefficientsDone, this, [=]() {
        transferActive = false;
    });

    // Check if this device was affected by the factory calibration problem
    if(factoryProblemSerials.contains(usb->serial())) {
        // it was, check if it still has bad factory coefficients

        struct point {
            uint16_t pointNum;
            double freq;
        };
        std::array<point, 3> pointsToCheck = {{
            {.pointNum = 451, .freq = 1.0},
            {.pointNum = 551, .freq = 2.0},
            {.pointNum = 651, .freq = 3.0},
        }};

        bool hasBadData = false;
        for(auto point : pointsToCheck) {
            // grab the point (just checking P12_THROUGH is enough
            QString ret = usb->Query(":COEFF:GET? FACTORY P12_THROUGH "+QString::number(point.pointNum));
            auto parts = ret.split(",");
            if (parts.size() == 9 && parts[0].toDouble() == point.freq) {
                // got the correct data, check S12 phase
                auto S12 = std::complex(parts[3].toDouble(), parts[4].toDouble());
                auto constexpr expectedS12Delay = 498e-12;
                auto expectedS12Phase = expectedS12Delay * (point.freq * 1e9) * 2*M_PI;
                auto phase = -arg(S12);
                // unwrap to expected phase
                while(phase < expectedS12Phase - M_PI) {
                    phase += 2*M_PI;
                }
                double S12Delay = phase / (2*M_PI) / (point.freq * 1e9);
                qDebug() << "expected delay:" << expectedS12Delay << "factory calibration delay:" << S12Delay;
                auto error = S12Delay - expectedS12Delay;
                double tolerance = 17e-12;
                if(abs(error) > tolerance) {
                    hasBadData = true;
                    break;
                }
//                QList<double> possibleErrors({0, -94.7e-12, 34.8e-12});
//                for(auto e : possibleErrors) {
//                    if(abs(error-e) <= tolerance) {
//                        // this is the error of the LibreCAL
//                        if(e != 0) {
//                            // still has the wrong coefficients
//                            hasBadData = true;
//                        }
//                        break;
//                    }
//                }
            } else {
                qWarning() << "Unexpected point frequency, factory calibration data is dubious";
            }
            if(hasBadData) {
                break;
            }
        }
        if(hasBadData) {
            if(InformationBox::AskQuestion("Update Factory Coefficients?", "Your LibreCAL with serial number "+usb->serial()+" is affected by "
                                          "a mistake in the factory calibration and its factory coefficients (specifically the phase of all "
                                          "THRU standard definitions) are slightly wrong. The mistake has been corrected and updated factory "
                                          "coefficients are available. Do you want to update the factory coefficients now?", false)) {
                factoryUpdateDialog();
            }
        }
    }
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
    transferActive = true;
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
        transferActive = false;
    }
}

void CalDevice::saveCoefficientSets()
{
    if(!hasModifiedCoefficients()) {
        // nothing to do, already done
        emit updateCoefficientsDone(true);
    } else {
        transferActive = true;
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
                    return nullptr;
                }
                if(line.startsWith("ERROR")) {
                    // something went wront
                    return nullptr;
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
                    return nullptr;
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
                if(!success) {
                    break;
                }
            }
            if(set.shorts.count(i)) {
                success &= createCoefficient(set.name, "P"+QString::number(i)+"_SHORT", set.shorts[i]->t, set.shorts[i]->modified);
                if(!success) {
                    break;
                }
            }
            if(set.loads.count(i)) {
                success &= createCoefficient(set.name, "P"+QString::number(i)+"_LOAD", set.loads[i]->t, set.loads[i]->modified);
                if(!success) {
                    break;
                }
            }
            for(int j=i+1;j<=numPorts;j++) {
                auto c = set.getThrough(i,j);
                if(c) {
                    success &= createCoefficient(set.name, "P"+QString::number(i)+QString::number(j)+"_THROUGH", c->t, c->modified);
                    if(!success) {
                        break;
                    }
                }
            }
            if(!success) {
                break;
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

void CalDevice::factoryUpdateDialog()
{
    auto d = new QDialog();
    auto ui = new Ui::FactoryUpdateDialog();
    ui->setupUi(d);

    ui->progress->setValue(0);
    connect(this, &CalDevice::updateCoefficientsPercent, ui->progress, &QProgressBar::setValue, Qt::QueuedConnection);

    auto updateEnableState = [=]() {
        ui->updateFile->setEnabled(true);
        ui->updateServer->setEnabled(true);
        if(ui->updateServer->isChecked()) {
            // can start
            ui->startUpdate->setEnabled(true);
            // file selection disabled
            ui->file->setEnabled(false);
            ui->browse->setEnabled(false);
        } else {
            // file selection enabled
            ui->file->setEnabled(true);
            ui->browse->setEnabled(true);
            // start possible if some file is selected
            ui->startUpdate->setEnabled(!ui->file->text().isEmpty());
        }
    };

    connect(ui->updateServer, &QRadioButton::toggled, this, updateEnableState);
    connect(ui->file, &QLineEdit::textChanged, this, updateEnableState);
    connect(ui->browse, &QPushButton::clicked, this, [=](){
        auto filename = QFileDialog::getOpenFileName(nullptr, "Select factory coefficient file", "", "Zip files (*.zip)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        } else {
            ui->file->setText(filename);
            updateEnableState();
        }
    });

    auto addStatus = [=](QString status) {
        ui->msg->appendPlainText(status);
    };
    auto abortWithError = [=](QString error) {
        ui->progress->setValue(0);

        QTextCharFormat tf;
        tf = ui->msg->currentCharFormat();
        tf.setForeground(QBrush(Qt::red));
        ui->msg->setCurrentCharFormat(tf);
        ui->msg->appendPlainText(error);
        tf.setForeground(QBrush(Qt::black));
        ui->msg->setCurrentCharFormat(tf);

        updateEnableState();
    };

    connect(this, &CalDevice::updateCoefficientsDone, this, [=](bool success) {
        // transfer complete

        // pass on communication failures again
        connect(usb, &USBDevice::communicationFailure, this, &CalDevice::disconnected);
        if(success) {
            ui->progress->setValue(100);
            addStatus("...done");
            updateEnableState();
        } else {
            abortWithError("Transferring coefficients to LibreCAL failed");
        }
    });

    auto updateFromFile = [=](QString filename) {
        addStatus("Unzipping file...");
        if(!QMicroz::extract(filename, ".")) {
            abortWithError("Extracting zip file failed");
            return;
        }
        // create coefficient set
        auto set = CoefficientSet();
        set.name = "FACTORY";
        set.ports = 4;
        set.createEmptyCoefficients();
        struct Coeff {
            QString description;
            QString filename;
            CoefficientSet::Coefficient *coeff;
        };
        std::array<Coeff, 18> coeffs = {{
                                         {.description = "Port 1 open", .filename = "P1_OPEN.s1p", .coeff = set.opens[1]},
                                         {.description = "Port 1 short", .filename = "P1_SHORT.s1p", .coeff = set.shorts[1]},
                                         {.description = "Port 1 load", .filename = "P1_LOAD.s1p", .coeff = set.loads[1]},
                                         {.description = "Port 2 open", .filename = "P2_OPEN.s1p", .coeff = set.opens[2]},
                                         {.description = "Port 2 short", .filename = "P2_SHORT.s1p", .coeff = set.shorts[2]},
                                         {.description = "Port 2 load", .filename = "P2_LOAD.s1p", .coeff = set.loads[2]},
                                         {.description = "Port 3 open", .filename = "P3_OPEN.s1p", .coeff = set.opens[3]},
                                         {.description = "Port 3 short", .filename = "P3_SHORT.s1p", .coeff = set.shorts[3]},
                                         {.description = "Port 3 load", .filename = "P3_LOAD.s1p", .coeff = set.loads[3]},
                                         {.description = "Port 4 open", .filename = "P4_OPEN.s1p", .coeff = set.opens[4]},
                                         {.description = "Port 4 short", .filename = "P4_SHORT.s1p", .coeff = set.shorts[4]},
                                         {.description = "Port 4 load", .filename = "P4_LOAD.s1p", .coeff = set.loads[4]},
                                         {.description = "Port 1 to 2 through", .filename = "P12_THROUGH.s2p", .coeff = set.throughs[set.portsToThroughIndex(1,2)]},
                                         {.description = "Port 1 to 3 through", .filename = "P13_THROUGH.s2p", .coeff = set.throughs[set.portsToThroughIndex(1,3)]},
                                         {.description = "Port 1 to 4 through", .filename = "P14_THROUGH.s2p", .coeff = set.throughs[set.portsToThroughIndex(1,4)]},
                                         {.description = "Port 2 to 3 through", .filename = "P23_THROUGH.s2p", .coeff = set.throughs[set.portsToThroughIndex(2,3)]},
                                         {.description = "Port 2 to 4 through", .filename = "P24_THROUGH.s2p", .coeff = set.throughs[set.portsToThroughIndex(2,4)]},
                                         {.description = "Port 3 to 4 through", .filename = "P34_THROUGH.s2p", .coeff = set.throughs[set.portsToThroughIndex(3,4)]},
                                         }};
        for(const Coeff &c : coeffs) {
            addStatus("Loading coefficient ("+c.description+")...");
            try {
                auto t = Touchstone::fromFile(c.filename.toStdString());
                c.coeff->t = t;
                c.coeff->modified = true;
                QFile rmfile(c.filename);
                rmfile.remove();
            } catch (const std::exception &e) {
                abortWithError("Failed: "+QString::fromStdString(e.what()));
                return;
            }
        }
        // delete previous factory set if available
        for(unsigned int i=0;i<coeffSets.size();i++) {
            if(coeffSets[i].name == "FACTORY") {
                coeffSets.erase(coeffSets.begin() + i);
            }
        }
        // add set to device
        coeffSets.push_back(set);
        // enable factory writing on device
        addStatus("Enable factory coefficient writes...");
        bool success = usb->Cmd(":FACT:ENABLEWRITE I_AM_SURE");
        // delete all factory coefficients (this formats the factory partition)
        success &= usb->Cmd(":FACT:DEL", 5000);

        if(success) {
            // start the transfer
            addStatus("Transferring new coefficients to LibreCAL...");
            // potential communication failures should not be passed on during this
            disconnect(usb, &USBDevice::communicationFailure, this, &CalDevice::disconnected);
            saveCoefficientSets();
        } else {
            abortWithError("Failed to erase previous factory calibration");
        }
    };

    auto netw = new QNetworkAccessManager();
    connect(netw, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if(reply->error() == QNetworkReply::NoError) {
            // success
            addStatus("Factory coefficients downloaded...");
            QFile file(serial()+".zip");
            if(!file.open(QIODevice::WriteOnly)) {
                abortWithError("Failed to create folder for zipped data");
                return;
            }
            file.write(reply->readAll());
            file.flush();
            file.close();
            updateFromFile(serial()+".zip");
            // remove zip file
            file.remove();
        } else {
            abortWithError("No factory coefficients found. Check internet access and serial number");
        }
    });

    connect(ui->startUpdate, &QPushButton::clicked, this, [=](){
        ui->msg->clear();
        ui->startUpdate->setEnabled(false);
        ui->updateFile->setEnabled(false);
        ui->updateServer->setEnabled(false);
        ui->file->setEnabled(false);
        ui->browse->setEnabled(false);
        if(ui->updateServer->isChecked()) {
            addStatus("Looking up factory coefficient data...");
            QUrl url("https://librecal.kaeberich.com/calibrationdata/"+serial()+".zip");
            netw->get(QNetworkRequest(url));
        } else {
            // update from file
            updateFromFile(ui->file->text());
        }
    });

    updateEnableState();

    d->exec();
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
