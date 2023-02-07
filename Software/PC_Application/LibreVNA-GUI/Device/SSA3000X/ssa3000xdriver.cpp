#include "ssa3000xdriver.h"

#include "CustomWidgets/informationbox.h"

#include <QTcpSocket>

SSA3000XDriver::SSA3000XDriver()
{
    searchAddresses.push_back(QHostAddress("192.168.22.2"));
}

SSA3000XDriver::~SSA3000XDriver()
{

}

std::set<QString> SSA3000XDriver::GetAvailableDevices()
{
    std::set<QString> ret;

    // attempt to establish a connection to check if the device is available and extract the serial number
    detectedDevices.clear();
    auto sock = QTcpSocket();
    for(auto address : searchAddresses) {
        sock.connectToHost(address, 5024);
        if(sock.waitForConnected(50)) {
           // connection successful
            sock.waitForReadyRead(100);
            auto line = QString(sock.readLine());
            if(line.startsWith("Welcome to the SCPI instrument 'Siglent SSA3")) {
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
        } else {
            qDebug() << "SSA3000X failed to connect";
        }
    }
    return ret;
}

bool SSA3000XDriver::connectTo(QString serial)
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
    info.supportedFeatures.insert(DeviceDriver::Feature::SA);
    info.supportedFeatures.insert(DeviceDriver::Feature::SATrackingGenerator);
    info.supportedFeatures.insert(DeviceDriver::Feature::Generator);
    info.supportedFeatures.insert(DeviceDriver::Feature::ExtRefIn);

    double maxFreq = 0;
    if(info.hardware_version == "SSA3032X") {
        maxFreq = 3200000000;
    } else if(info.hardware_version == "SSA3021X") {
        maxFreq = 2100000000;
    } else {
        dataSocket.close();
        InformationBox::ShowError("Error", "Invalid hardware version: " + info.hardware_version);
        return false;
    }

    info.Limits.SA.ports = 1;
    info.Limits.SA.minFreq = 0;
    info.Limits.SA.maxFreq = maxFreq;
    info.Limits.SA.minRBW = 1;
    info.Limits.SA.maxRBW = 1000000;
    info.Limits.SA.mindBm = -20;
    info.Limits.SA.maxdBm = 0;
    info.Limits.Generator.ports = 1;
    info.Limits.Generator.minFreq = 0;
    info.Limits.Generator.maxFreq = maxFreq;
    info.Limits.Generator.mindBm = -20;
    info.Limits.Generator.maxdBm = 0;

    connected = true;

    // reset to default configuration
    dataSocket.write("*RST\r\n");

    emit InfoUpdated();

//    // sockets are connected now
//    dataBuffer.clear();
//    logBuffer.clear();
//    connect(&dataSocket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::error), this, &LibreVNATCPDriver::ConnectionLost, Qt::QueuedConnection);
//    connect(&logSocket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::error), this, &LibreVNATCPDriver::ConnectionLost, Qt::QueuedConnection);

//    qInfo() << "TCP connection established" << flush;
//    this->serial = serial;
//    connected = true;

//    connect(&dataSocket, &QTcpSocket::readyRead, this, &LibreVNATCPDriver::ReceivedData, Qt::UniqueConnection);
//    connect(&logSocket, &QTcpSocket::readyRead, this, &LibreVNATCPDriver::ReceivedLog, Qt::UniqueConnection);
//    connect(&transmissionTimer, &QTimer::timeout, this, &LibreVNATCPDriver::transmissionTimeout, Qt::UniqueConnection);
//    connect(this, &LibreVNATCPDriver::receivedAnswer, this, &LibreVNATCPDriver::transmissionFinished, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
//    connect(this, &LibreVNATCPDriver::receivedPacket, this, &LibreVNATCPDriver::handleReceivedPacket, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
//    transmissionTimer.setSingleShot(true);
//    transmissionActive = false;

//    sendWithoutPayload(Protocol::PacketType::RequestDeviceInfo);
//    sendWithoutPayload(Protocol::PacketType::RequestDeviceStatus);
//    updateIFFrequencies();
    return true;
}

void SSA3000XDriver::disconnect()
{
    dataSocket.close();
    connected = false;
}

DeviceDriver::Info SSA3000XDriver::getInfo()
{
    return info;
}

std::set<DeviceDriver::Flag> SSA3000XDriver::getFlags()
{
    return std::set<DeviceDriver::Flag>();
}

QString SSA3000XDriver::getStatus()
{
    return "";
}

QStringList SSA3000XDriver::availableSAMeasurements()
{
    return {"PORT1"};
}

bool SSA3000XDriver::setSA(const DeviceDriver::SASettings &s, std::function<void (bool)> cb)
{
    if(!connected) {
        return false;
    }
    write(":FREQ:STAR "+QString::number(s.freqStart));
    write(":FREQ:STOP "+QString::number(s.freqStop));
    write(":BWID "+QString::number(s.RBW));
    write(":FREQ:STAR "+QString::number(s.freqStart));
    write(":FREQ:STAR "+QString::number(s.freqStart));
    write(":FREQ:STAR "+QString::number(s.freqStart));

    QString windowName = "";
    switch(s.window) {
    case SASettings::Window::FlatTop:
        windowName = "FLATtop";
        break;
    case SASettings::Window::Hann:
        windowName = "HANNing";
        break;
    case SASettings::Window::Kaiser:
        windowName = "HAMMing"; // kaiser is not available
        break;
    case SASettings::Window::None:
        windowName = "RECTangular";
        break;
    }
    write(":DDEM:FFT:WIND "+windowName);

    QString detName = "";
    switch(s.detector) {
    case SASettings::Detector::Average:
        detName = "AVERage";
        break;
    case SASettings::Detector::Normal:
        detName = "NORMAL";
        break;
    case SASettings::Detector::NPeak:
        detName = "NEGative";
        break;
    case SASettings::Detector::PPeak:
        detName = "POSitive";
        break;
    case SASettings::Detector::Sample:
        detName = "SAMPle";
        break;
    }
    write(":DET:TRAC:FUNC "+detName);

    write(":OUTP:STAT " + (s.trackingGenerator ? QString("ON") : QString("OFF")));
    write(":SOUR:POW "+QString::number((int) s.trackingPower));

    return true;
}

unsigned int SSA3000XDriver::getSApoints()
{
    return 0;
}

QStringList SSA3000XDriver::availableSGPorts()
{
    return {"PORT1"};
}

bool SSA3000XDriver::setSG(const DeviceDriver::SGSettings &s)
{
    if(!connected) {
        return false;
    }
    if(s.port == 1) {
        write(":FREQ:SPAN:ZERO");
        write(":FREQ:CENT "+QString::number(s.freq));
        write(":OUTP:STAT ON");
        write(":SOUR:POW "+QString::number((int) s.dBm));
    } else {
        write(":OUTP:STAT OFF");
    }
    return true;
}

bool SSA3000XDriver::setIdle(std::function<void (bool)> cb)
{
    if(!connected) {
        return false;
    }
    write("*RST\r\n");
    if(cb) {
        cb(true);
    }
}

QStringList SSA3000XDriver::availableExtRefInSettings()
{
    return {""};
}

QStringList SSA3000XDriver::availableExtRefOutSettings()
{
    return {""};
}

bool SSA3000XDriver::setExtRef(QString option_in, QString option_out)
{
    return false;
}

void SSA3000XDriver::write(QString s)
{
    dataSocket.write(QString(s + "\r\n").toLocal8Bit());
}
