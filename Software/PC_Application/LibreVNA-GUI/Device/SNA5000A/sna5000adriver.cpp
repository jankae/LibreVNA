#include "sna5000adriver.h"

#include "CustomWidgets/informationbox.h"
#include "Util/util.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QApplication>

SNA5000ADriver::SNA5000ADriver()
    : DeviceTCPDriver("SNA5000A")
{
    diffGen = new TraceDifferenceGenerator<VNAPoint>([=](const VNAPoint &p){
        VNAMeasurement m;
        m.Z0 = 50.0;
        m.pointNum = p.index;
        m.frequency = p.frequency;
        m.dBm = excitationPower;
        m.measurements = p.data;
        emit VNAmeasurementReceived(m);
    });

    traceReader.waitingForResponse = false;
//    connect(&traceTimer, &QTimer::timeout, this, &SNA5000ADriver::extractTracePoints);
//    traceTimer.setSingleShot(true);
}

SNA5000ADriver::~SNA5000ADriver()
{
    delete diffGen;
}

std::set<QString> SNA5000ADriver::GetAvailableDevices()
{
    std::set<QString> ret;

    // attempt to establish a connection to check if the device is available and extract the serial number
    detectedDevices.clear();
    auto sock = QTcpSocket();
    for(auto address : getSearchAddresses()) {
        sock.connectToHost(address, 5024);
        if(sock.waitForConnected(50)) {
           // connection successful
            sock.waitForReadyRead(100);
            auto line = QString(sock.readLine());
            if(line.startsWith("Welcome to the SCPI instrument 'Siglent SNA5")) {
                // throw away command prompt ">>"
                sock.readLine();
                // looks like we are connected to the correct instrument, request serial number
                sock.write("*IDN?\r\n");
                sock.waitForReadyRead(100);
                sock.read(1);
                sock.waitForReadyRead(100);
                line = QString(sock.readLine());
                auto fields = line.split(",");
                if(fields.size() == 4) {
                    detectedDevices[fields[2]] = address;
                    ret.insert(fields[2]);
                }
            }
            sock.disconnect();
        }
    }
    return ret;
}

bool SNA5000ADriver::connectTo(QString serial)
{
    if(connected) {
        disconnect();
    }

    // check if this device is actually available
    QHostAddress address;
    bool available = false;
    for(auto d : detectedDevices) {
        if(d.first == serial) {
            address = d.second;
            available = true;
            break;
        }
    }
    if(!available) {
        // no location information about this device available
        return false;
    }

    dataSocket.connectToHost(address, 5025);

    // check if connection succeeds
    if(!dataSocket.waitForConnected(1000)) {
        // socket failed
        dataSocket.close();
        InformationBox::ShowError("Error", "TCP connection timed out");
        return false;
    }

    connect(&dataSocket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred), this, [this](QAbstractSocket::SocketError err) {
        if(err == QAbstractSocket::SocketTimeoutError) {
            // ignore, these are triggered by the query function
        } else {
            emit ConnectionLost();
        }
    }, Qt::QueuedConnection);

    // grab model information
    dataSocket.write("*IDN?\r\n");
    dataSocket.waitForReadyRead(100);
    auto line = QString(dataSocket.readLine());
    auto fields = line.split(",");
    if(fields.size() != 4) {
        dataSocket.close();
        InformationBox::ShowError("Error", "Invalid *IDN? response");
        return false;
    }

    this->serial = fields[2];

    info = Info();
    info.hardware_version = fields[1];
    info.firmware_version = fields[3].trimmed();

    const QStringList supportedDevices = {"SNA5002A", "SNA5004A", "SNA5012A", "SNA5014A"};

    if(!supportedDevices.contains(info.hardware_version)) {
        dataSocket.close();
        InformationBox::ShowError("Error", "Invalid hardware version: " + info.hardware_version);
        return false;
    }

    info.supportedFeatures.insert(DeviceDriver::Feature::VNA);
    info.supportedFeatures.insert(DeviceDriver::Feature::VNAFrequencySweep);
    info.supportedFeatures.insert(DeviceDriver::Feature::Generator);

    // Extract limits
    info.Limits.VNA.ports = queryInt(":SERVICE:PORT:COUNT?");
    info.Limits.VNA.minFreq = queryInt(":SERVICE:SWEEP:FREQENCY:MINIMUM?");
    info.Limits.VNA.maxFreq = queryInt(":SERVICE:SWEEP:FREQENCY:MAXIMUM?");
    info.Limits.VNA.maxPoints = queryInt(":SERVICE:SWEEP:POINTS?");
    info.Limits.VNA.minIFBW = 10;
    info.Limits.VNA.maxIFBW = 3000000;
    info.Limits.VNA.mindBm = -55;
    info.Limits.VNA.maxdBm = 10;

    info.Limits.Generator.ports = info.Limits.VNA.ports;
    info.Limits.Generator.minFreq = info.Limits.VNA.minFreq;
    info.Limits.Generator.maxFreq = info.Limits.VNA.maxFreq;
    info.Limits.Generator.mindBm = info.Limits.VNA.mindBm;
    info.Limits.Generator.maxdBm = info.Limits.VNA.maxdBm;

    connected = true;

    // reset to default configuration
    dataSocket.write("*RST\r\n");

    emit InfoUpdated();

    return true;
}

void SNA5000ADriver::disconnect()
{
    traceReaderStop();
    connected = false;
    dataSocket.close();
}

DeviceDriver::Info SNA5000ADriver::getInfo()
{
    return info;
}

std::set<DeviceDriver::Flag> SNA5000ADriver::getFlags()
{
    return std::set<DeviceDriver::Flag>();
}

QString SNA5000ADriver::getStatus()
{
    return "";
}

QStringList SNA5000ADriver::availableVNAMeasurements()
{
    switch(info.Limits.VNA.ports) {
    case 2:
        return {"S11", "S12", "S21", "S22"};
    case 4:
        return {"S11", "S12", "S13", "S14", "S21", "S22", "S23", "S24", "S31", "S32", "S33", "S34", "S41", "S42", "S43", "S44"};
    default:
        return {""};
    }
}

bool SNA5000ADriver::setVNA(const VNASettings &s, std::function<void (bool)> cb)
{
    excitationPower = s.dBmStart;
    excitedPorts = s.excitedPorts;

    if(!traceReaderStop()) {
        emit ConnectionLost();
        return false;
    }

    // disable unused traces
    for(unsigned int i=1;i<=info.Limits.VNA.ports;i++) {
        write(":DISP:WIND:TRAC"+QString::number(i)+" 0");
    }
    // enable a trace for every active port
    for(auto p : s.excitedPorts) {
        write(":DISP:WIND:TRAC"+QString::number(p)+" 1");
        // set the parameter to force the stimulus active at the port
        write(":CALC:PAR"+QString::number(p)+":DEF S"+QString::number(p)+QString::number(p));
    }

    // configure the sweep
    write(":SENS:FREQ:STAR "+QString::number(s.freqStart));
    write(":SENS:FREQ:STOP "+QString::number(s.freqStop));
    write(":SENS:BWID "+QString::number(s.IFBW));
    write(":SOUR:POW "+QString::number(s.dBmStart));
    write(":SENS:SWEEP:POINTS "+QString::number(s.points));

//    traceTimer.start(100);
    if(cb) {
        cb(true);
    }
    traceReaderRestart();
    return true;
}

QStringList SNA5000ADriver::availableSGPorts()
{
    switch(info.Limits.Generator.ports) {
    case 2:
        return {"PORT1", "PORT2"};
    case 4:
        return {"PORT1", "PORT2", "PORT3", "PORT4"};
    default:
        return {""};
    }
}

bool SNA5000ADriver::setSG(const SGSettings &s)
{
    // enable SA mode (generator control is only available there)
    write(":CALC:CUST:DEF \"SA\"");

    if(s.port == 0) {
        // turn off all ports
        for(unsigned int i=0;i<info.Limits.Generator.ports;i++) {
            write(":SENS:SA:SOUR"+QString::number(i)+":STAT OFF");
        }
    } else {
        // set the frequency
        write(":SENS:SA:SOUR"+QString::number(s.port)+":FREQ:CW "+QString::number(s.freq));
        // set the power
        write(":SENS:SA:SOUR"+QString::number(s.port)+":POW:VAL "+QString::number(s.dBm));
        // enable the port
        write(":SENS:SA:SOUR"+QString::number(s.port)+":STAT ON");
    }
    return true;
}

bool SNA5000ADriver::setIdle(std::function<void (bool)> cb)
{
    if(!connected) {
        return false;
    }
    if(!traceReaderStop()) {
        emit ConnectionLost();
        return false;
    }
//    traceTimer.stop();

    write("*RST\r\n");
    if(cb) {
        cb(true);
    }
    return true;
}

QStringList SNA5000ADriver::availableExtRefInSettings()
{
    return {""};
}

QStringList SNA5000ADriver::availableExtRefOutSettings()
{
    return {""};
}

bool SNA5000ADriver::setExtRef(QString option_in, QString option_out)
{
    Q_UNUSED(option_in)
    Q_UNUSED(option_out)
    return false;
}

void SNA5000ADriver::write(QString s)
{
    dataSocket.write(QString(s + "\r\n").toLocal8Bit());
    dataSocket.readAll();
}

QString SNA5000ADriver::query(QString s, unsigned int timeout)
{
    dataSocket.write(QString(s + "\r\n").toLocal8Bit());
    if(!waitForLine(timeout)) {
        return QString();
    } else {
        return QString(dataSocket.readLine());
    }
}

long long SNA5000ADriver::queryInt(QString s)
{
    auto resp = query(s);
    if(resp.isEmpty()) {
        return 0;
    } else {
        return resp.toLongLong();
    }
}

std::vector<double> SNA5000ADriver::queryDoubleList(QString s)
{
    std::vector<double> ret;
    auto resp = query(s, 1000);
    if(!resp.isEmpty()) {
        QStringList values = resp.split(",");
        for(auto v : values) {
            ret.push_back(v.toDouble());
        }
    }
    return ret;
}

void SNA5000ADriver::extractTracePoints()
{
//    while(connected) {
//        qDebug() << "SNA5000 Thread";
//        std::vector<double> xcoord = queryDoubleList(":CALC:DATA:XAXIS?");
//        std::map<QString, std::vector<double>> data;
//        for(auto i : excitedPorts) {
//            for(auto j : excitedPorts) {
//                QString name = "S"+QString::number(i)+QString::number(j);
//                std::vector<double> Sij = queryDoubleList(":SENS:DATA:RAWD? "+name);
//                if(Sij.size() != xcoord.size() * 2) {
//                    // invalid size, abort
//                    return;
//                }
//                data[name] = Sij;
//            }
//        }

//        // Compile VNApoints
//        std::vector<VNAPoint> trace;
//        trace.resize(xcoord.size());
//        for(unsigned int i=0;i<xcoord.size();i++) {
//            trace[i].index = i;
//            trace[i].frequency = xcoord[i];
//            std::map<QString, std::complex<double>> tracedata;
//            for(auto d : data) {
//                tracedata[d.first] = std::complex(d.second[i*2], d.second[i*2+1]);
//            }
//            trace[i].data = tracedata;
//        }

//        diffGen->newTrace(trace);
//        QThread::msleep(100);
//    }
    //    traceTimer.start(500);
}

void SNA5000ADriver::handleIncomingData()
{
    if(!dataSocket.canReadLine()) {
        // no complete response yet, ignore
        return;
    }
    traceReader.waitingForResponse = false;
    std::vector<double> data;
    QStringList values = QString(dataSocket.readLine()).split(",");
    for(auto v : values) {
        data.push_back(v.toDouble());
    }
    if(traceReader.state == 0) {
        traceReader.xaxis = data;
    } else {
        unsigned int comp = 0;
        bool handled = false;
        for(auto i : excitedPorts) {
            for(auto j : excitedPorts) {
                comp++;
                if(traceReader.state == comp) {
                    QString name = "S"+QString::number(i)+QString::number(j);
                    traceReader.data[name] = data;
                    handled = true;
                    break;
                }
            }
            if(handled) {
                break;
            }
        }
    }
    if(traceReader.state >= excitedPorts.size()*excitedPorts.size()) {
        // Check size, abort if wrong
        bool sizeOkay = true;
        for(auto d : traceReader.data) {
            if(d.second.size() != traceReader.xaxis.size() * 2) {
                sizeOkay = false;
                break;
            }
        }

        if(sizeOkay) {
            /*
             * The SNA5000A performs the measurements in multiple sweeps (with the stimulus active at one port per sweep).
             * E.g. measuring S11/S21 in the first sweep and then S12/S22. The LibreVNA-GUI expects each point of each sweep
             * to containg all measured parameters (S11/S12/S21/S22).
             *
             * Values that are not measured yet are reported as very small values by the SNA5000A. Ignore all datapoints
             * where at least one parameter is too small (=not measured yet). This will cause a delay in the displaying of
             * measurements but at least we can process complete datapoints afterwards.
             */

            // threshold equals -196dB, we can safely assume that no real measurement will ever below that
            constexpr double threshold = 1e-10;

            int lastIndex = -1;
            for(unsigned int i=0;i<traceReader.xaxis.size();i++) {
                for(auto d : traceReader.data) {
                    if(abs(d.second[i*2]) < threshold && abs(d.second[i*2+1]) < threshold) {
                        lastIndex = i;
                        break;
                    }
                }
                if(lastIndex >= 0) {
                    // deteceted incomplete measurements
                    break;
                }
            }
            if(lastIndex == -1) {
                // all measurements complete
                lastIndex = traceReader.xaxis.size();
            }

            if(lastIndex > 0) {
                // Compile VNApoints
                std::vector<VNAPoint> trace;
                trace.resize(lastIndex);
                for(int i=0;i<lastIndex;i++) {
                    trace[i].index = i;
                    trace[i].frequency = traceReader.xaxis[i];
                    std::map<QString, std::complex<double>> tracedata;
                    for(auto d : traceReader.data) {
                        tracedata[d.first] = std::complex(d.second[i*2], d.second[i*2+1]);
                    }
                    trace[i].data = tracedata;
                }

                diffGen->newTrace(trace);
            }
        }
        traceReader.state = 0;
    } else {
        // move on to next trace
        traceReader.state++;
    }
    traceReaderStatemachine();
}

bool SNA5000ADriver::traceReaderStop(unsigned int timeout)
{
    traceReader.enabled = false;
    if(traceReader.waitingForResponse) {
        // already issued a command, needs to wait for the response parsing
        auto start = QDateTime::currentDateTimeUtc();
        while(traceReader.waitingForResponse) {
            if(start.msecsTo(QDateTime::currentDateTimeUtc()) >= timeout) {
                // timed out
                qWarning() << "Timed out waiting for trace reader to stop";
                return false;
            }
            QApplication::processEvents();
        }
        QObject::disconnect(&dataSocket, &QTcpSocket::readyRead, this, &SNA5000ADriver::handleIncomingData);
        return true;
    } else {
        // already stopped
        QObject::disconnect(&dataSocket, &QTcpSocket::readyRead, this, &SNA5000ADriver::handleIncomingData);
        return true;
    }
}

void SNA5000ADriver::traceReaderRestart()
{
    traceReader.enabled = true;
    traceReader.data.clear();
    traceReader.xaxis.clear();
    traceReader.state = 0;
    dataSocket.readAll();
    traceReaderStatemachine();
}

void SNA5000ADriver::traceReaderStatemachine()
{
    if(!traceReader.enabled) {
        return;
    }
    if(traceReader.state == 0) {
        write(":CALC:DATA:XAXIS?");
        traceReader.waitingForResponse = true;
        QObject::connect(&dataSocket, &QTcpSocket::readyRead, this, &SNA5000ADriver::handleIncomingData, Qt::UniqueConnection);
    } else {
        unsigned int comp = 0;
        for(auto i : excitedPorts) {
            for(auto j : excitedPorts) {
                comp++;
                if(traceReader.state == comp) {
                    QString name = "S"+QString::number(i)+QString::number(j);
                    write(":SENS:DATA:RAWD? "+name);
                    traceReader.waitingForResponse = true;
                    QObject::connect(&dataSocket, &QTcpSocket::readyRead, this, &SNA5000ADriver::handleIncomingData, Qt::UniqueConnection);
                    return;
                }
            }
        }
    }
}

bool SNA5000ADriver::waitForLine(unsigned int timeout)
{
    auto start = QDateTime::currentDateTimeUtc();
    while(!dataSocket.canReadLine()) {
        if(start.msecsTo(QDateTime::currentDateTimeUtc()) >= timeout) {
            // timed out
            qWarning() << "Timed out waiting for response";
            return false;
        }
        dataSocket.waitForReadyRead(10);
    }
    return true;
}

