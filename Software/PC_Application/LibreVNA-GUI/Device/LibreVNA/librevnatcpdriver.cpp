#include "librevnatcpdriver.h"

#include "CustomWidgets/informationbox.h"
#include "devicepacketlog.h"
#include "Util/util.h"

#include <QTimer>
#include <QNetworkInterface>

using namespace std;

static const QString service_name = "urn:schemas-upnp-org:device:LibreVNA:1";
static constexpr int DataPort = 19544;
static constexpr int LogPort = 19545;
static auto SSDPaddress = QHostAddress("239.255.255.250");
static constexpr int SSDPport = 1900;

LibreVNATCPDriver::LibreVNATCPDriver()
    : LibreVNADriver()
{
    connected = false;
    m_receiveThread = nullptr;


    auto interfaces = QNetworkInterface::allInterfaces();
    for(auto i : interfaces) {
        switch(i.type()) {
        case QNetworkInterface::Ethernet:
        case QNetworkInterface::Wifi:
        case QNetworkInterface::Virtual:
        case QNetworkInterface::Unknown:
            break;
        default:
            // skip all other interface types
            continue;
        }
        auto socket = new QUdpSocket();
        socket->bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress);
        socket->setMulticastInterface(i);
        socket->joinMulticastGroup(SSDPaddress, i);
        connect(socket, &QUdpSocket::readyRead, this, [=](){
            SSDPreceived(socket);
        });
        ssdpSockets.push_back(socket);
    }

    connect(&ssdpTimer, &QTimer::timeout,this, &LibreVNATCPDriver::SSDRequest);
    ssdpTimer.start(1000);

    specificSettings.push_back(Savable::SettingDescription(&captureRawReceiverValues, "LibreVNATCPDriver.captureRawReceiverValues", false));
    specificSettings.push_back(Savable::SettingDescription(&harmonicMixing, "LibreVNATCPDriver.harmonicMixing", false));
    specificSettings.push_back(Savable::SettingDescription(&SASignalID, "LibreVNATCPDriver.signalID", true));
    specificSettings.push_back(Savable::SettingDescription(&VNASuppressInvalidPeaks, "LibreVNATCPDriver.suppressInvalidPeaks", true));
    specificSettings.push_back(Savable::SettingDescription(&VNAAdjustPowerLevel, "LibreVNATCPDriver.adjustPowerLevel", false));
    specificSettings.push_back(Savable::SettingDescription(&SAUseDFT, "LibreVNATCPDriver.useDFT", true));
    specificSettings.push_back(Savable::SettingDescription(&SARBWLimitForDFT, "LibreVNATCPDriver.RBWlimitDFT", 3000));
}

QString LibreVNATCPDriver::getDriverName()
{
    return "LibreVNA/TCP";
}

std::set<QString> LibreVNATCPDriver::GetAvailableDevices()
{
    std::set<QString> serials;
    for(auto d : detectedDevices) {
        serials.insert(d.serial);
    }

    return serials;
}

bool LibreVNATCPDriver::connectTo(QString serial)
{
    if(connected) {
        disconnect();
    }

    // check if this device is actually available
    DetectedDevice devInfo;
    bool available = false;
    for(auto d : detectedDevices) {
        if(d.serial == serial) {
            devInfo = d;
            available = true;
            break;
        }
    }
    if(!available) {
        // no location information about this device available
        return false;
    }

    // attempt to connect to the device
    dataSocket.connectToHost(devInfo.address, DataPort);
    logSocket.connectToHost(devInfo.address, LogPort);

    // check if connection succeeds
    if(!dataSocket.waitForConnected(1000) || !logSocket.waitForConnected(1000)) {
        // at least one socket failed
        dataSocket.close();
        logSocket.close();
        InformationBox::ShowError("Error", "TCP connection timed out");
        return false;
    }

    // sockets are connected now
    dataBuffer.clear();
    logBuffer.clear();
    connect(&dataSocket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred), this, &LibreVNATCPDriver::ConnectionLost, Qt::QueuedConnection);
    connect(&logSocket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred), this, &LibreVNATCPDriver::ConnectionLost, Qt::QueuedConnection);

    qInfo() << "TCP connection established" << Qt::flush;
    this->serial = serial;
    connected = true;

    connect(&dataSocket, &QTcpSocket::readyRead, this, &LibreVNATCPDriver::ReceivedData, Qt::UniqueConnection);
    connect(&logSocket, &QTcpSocket::readyRead, this, &LibreVNATCPDriver::ReceivedLog, Qt::UniqueConnection);
    connect(&transmissionTimer, &QTimer::timeout, this, &LibreVNATCPDriver::transmissionTimeout, Qt::UniqueConnection);
    connect(this, &LibreVNATCPDriver::receivedAnswer, this, &LibreVNATCPDriver::transmissionFinished, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(this, &LibreVNATCPDriver::receivedPacket, this, &LibreVNATCPDriver::handleReceivedPacket, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    transmissionTimer.setSingleShot(true);
    transmissionActive = false;

    sendWithoutPayload(Protocol::PacketType::RequestDeviceInfo);
    sendWithoutPayload(Protocol::PacketType::RequestDeviceStatus);
//    updateIFFrequencies();
    return true;
}

void LibreVNATCPDriver::disconnect()
{
    if(connected) {
        setIdle();
        transmissionTimer.stop();
        transmissionQueue.clear();
        transmissionActive = false;
        dataSocket.flush();
        dataSocket.close();
        logSocket.close();
//        delete dataSocket;
//        delete logSocket;
//        dataSocket = nullptr;
//        logSocket = nullptr;
        connected = false;
        serial = "";
    }
}

void LibreVNATCPDriver::registerTypes()
{
    qDebug() << "Registering meta type: " << qRegisterMetaType<Protocol::PacketInfo>();
    qDebug() << "Registering meta type: " << qRegisterMetaType<TransmissionResult>();
}

void LibreVNATCPDriver::SSDRequest()
{
    QByteArray data;
    data.append("M-SEARCH * HTTP/1.1\r\n"
                "HOST: 239.255.255.250:1900\r\n"
                "MAN: \"ssdp:discover\"\r\n"
                "MX: 1\r\n"
                "ST: ");
    data.append(service_name.toUtf8());
    data.append("\r\n"
                "\r\n");

    pruneDetectedDevices();

    for(auto s : ssdpSockets) {
        s->writeDatagram(data.data(), SSDPaddress, SSDPport);
    }
}

void LibreVNATCPDriver::SSDPreceived(QUdpSocket *sock)
{
    while(sock->hasPendingDatagrams()) {
        QHostAddress sender;
        quint16 senderPort;
        QByteArray buf(sock->pendingDatagramSize(), Qt::Uninitialized);
        QDataStream str(&buf, QIODevice::ReadOnly);
        sock->readDatagram(buf.data(), buf.size(), &sender, &senderPort);

        QString ssdp_string = QString(buf);
        auto lines = ssdp_string.split("\r\n");

        QString location, st, serial, max_age = "2";

        if(lines[0] != "HTTP/1.1 200 OK") {
            continue;
        }
        for(QString l : lines) {
            if(l.startsWith("LOCATION:")) {
                location = l.split(" ")[1];
            } else if(l.startsWith("ST:")) {
                st = l.split(" ")[1];
            } else if(l.startsWith("LibreVNA-serial:")) {
                serial = l.split(" ")[1];
            } else if(l.startsWith("CACHE-CONTROL:")) {
                max_age = l.split("=")[1];
            }
        }
        if(location.isEmpty() || st.isEmpty() || serial.isEmpty() || max_age.isEmpty()) {
            // some required field is missing
            continue;
        }

        // new device
        DetectedDevice d;
        d.address = QHostAddress(location);
        d.maxAgeSeconds = max_age.toUInt();
        d.responseTime = QDateTime::currentDateTime();
        d.serial = serial;
        addDetectedDevice(d);
    }
}

void LibreVNATCPDriver::ReceivedData()
{
    dataBuffer.append(dataSocket.readAll());
    Protocol::PacketInfo packet;
    uint16_t handled_len;
//    qDebug() << "Received data";
    do {
//        qDebug() << "Decoding" << dataBuffer->getReceived() << "Bytes";
        handled_len = Protocol::DecodeBuffer((uint8_t*) dataBuffer.data(), dataBuffer.size(), &packet);
//        qDebug() << "Handled" << handled_len << "Bytes, type:" << (int) packet.type;
        if(handled_len > 0) {
            auto &log = DevicePacketLog::getInstance();
            if(packet.type != Protocol::PacketType::None) {
                log.addPacket(packet, serial);
            } else {
                log.addInvalidBytes((uint8_t*) dataBuffer.data(), handled_len, serial);
            }
        }
        dataBuffer.remove(0, handled_len);
        switch(packet.type) {
        case Protocol::PacketType::Ack:
            emit receivedAnswer(TransmissionResult::Ack);
            break;
        case Protocol::PacketType::Nack:
            emit receivedAnswer(TransmissionResult::Nack);
            break;
        case Protocol::PacketType::SetTrigger:
            emit receivedTrigger(this, true);
            break;
        case Protocol::PacketType::ClearTrigger:
            emit receivedTrigger(this, false);
            break;
       default:
            // pass on to LibreVNADriver class
            emit receivedPacket(packet);
            break;
        }
    } while (handled_len > 0);
}

void LibreVNATCPDriver::ReceivedLog()
{
    logBuffer.append(logSocket.readAll());
    uint16_t handled_len;
    do {
        handled_len = 0;
        auto firstLinebreak = (uint8_t*) memchr((uint8_t*) logBuffer.data(), '\n', logBuffer.size());
        if(firstLinebreak) {
            handled_len = firstLinebreak - (uint8_t*) logBuffer.data();
            auto line = QString::fromLatin1(logBuffer.data(), handled_len - 1);
            emit LogLineReceived(line);
            logBuffer.remove(0, handled_len + 1);
        }
    } while(handled_len > 0);
}

void LibreVNATCPDriver::transmissionFinished(LibreVNADriver::TransmissionResult result)
{
    lock_guard<mutex> lock(transmissionMutex);
    // remove transmitted packet
//    qDebug() << "Transmission finsished (" << result << "), queue at " << transmissionQueue.size() << " Outstanding ACKs:"<<outstandingAckCount;
    if(transmissionQueue.empty()) {
        qWarning() << "transmissionFinished with empty transmission queue, stray Ack? Result:" << result;
        return;
    }
    auto t = transmissionQueue.dequeue();
    if(result == TransmissionResult::Timeout) {
        qWarning() << "transmissionFinished with timeout, packettype:" << (int) t.packet.type << "Device:" << serial;
    }
    if(result == TransmissionResult::Nack) {
        qWarning() << "transmissionFinished with NACK";
    }
    if(t.callback) {
        t.callback(result);
    }
    transmissionTimer.stop();
    bool success = false;
    while(!transmissionQueue.isEmpty() && !success) {
        success = startNextTransmission();
        if(!success) {
            // failed to send this packet
            auto t = transmissionQueue.dequeue();
            if(t.callback) {
                t.callback(TransmissionResult::InternalError);
            }
        }
    }
    if(transmissionQueue.isEmpty()) {
        transmissionActive = false;
    }
}

bool LibreVNATCPDriver::SendPacket(const Protocol::PacketInfo &packet, std::function<void (LibreVNADriver::TransmissionResult)> cb, unsigned int timeout)
{
    Transmission t;
    t.packet = packet;
    t.timeout = timeout;
    t.callback = cb;
    lock_guard<mutex> lock(transmissionMutex);
    transmissionQueue.enqueue(t);
//    qDebug() << "Enqueued packet, queue at " << transmissionQueue.size();
    if(!transmissionActive) {
        startNextTransmission();
    }
    return true;
}

void LibreVNATCPDriver::addDetectedDevice(const LibreVNATCPDriver::DetectedDevice &d)
{
    for(auto &e : detectedDevices) {
        if(e.serial == d.serial) {
            // replace entry
            e = d;
            return;
        }
    }
    // serial not already present, add
    detectedDevices.push_back(d);
}

void LibreVNATCPDriver::pruneDetectedDevices()
{
    for(unsigned int i=0;i<detectedDevices.size();i++) {
        auto d = detectedDevices[i];
        if(d.responseTime.secsTo(QDateTime::currentDateTime()) > d.maxAgeSeconds) {
            // too old, remove
            detectedDevices.erase(detectedDevices.begin()+i);
            i--;
        }
    }
}

bool LibreVNATCPDriver::startNextTransmission()
{
    if(transmissionQueue.isEmpty() || !connected) {
        // nothing more to transmit
        transmissionActive = false;
        return false;
    }
    transmissionActive = true;
    auto t = transmissionQueue.head();
    unsigned char buffer[1024];
    unsigned int length = Protocol::EncodePacket(t.packet, buffer, sizeof(buffer));
    if(!length) {
        qCritical() << "Failed to encode packet";
        return false;
    }
    auto &log = DevicePacketLog::getInstance();
    log.addPacket(t.packet);
    auto ret = dataSocket.write((char*) buffer, length);
    if(ret < 0) {
        qCritical() << "Error sending TCP data";
        return false;
    }
    transmissionTimer.start(t.timeout);
//    qDebug() << "Transmission started, queue at " << transmissionQueue.size();
    return true;
}

