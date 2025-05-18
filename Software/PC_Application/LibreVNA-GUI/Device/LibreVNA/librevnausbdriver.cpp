#include "librevnausbdriver.h"

#include "CustomWidgets/informationbox.h"
#include "devicepacketlog.h"

#include <QTimer>
#include <QThread>

using namespace std;

using USBID = struct {
    int VID;
    int PID;
};
static constexpr USBID IDs[] = {
    {0x0483, 0x564e},
    {0x0483, 0x4121},
    {0x1209, 0x4121},
};

LibreVNAUSBDriver::LibreVNAUSBDriver()
    : LibreVNADriver()
{
    connected = false;
    m_handle = nullptr;
    m_context = nullptr;
    dataBuffer = nullptr;
    logBuffer = nullptr;
    m_receiveThread = nullptr;
    lastTimestamp = QDateTime::currentDateTime();
    byteCnt = 0;

    specificSettings.push_back(Savable::SettingDescription(&captureRawReceiverValues, "LibreVNAUSBDriver.captureRawReceiverValues", false));
    specificSettings.push_back(Savable::SettingDescription(&harmonicMixing, "LibreVNAUSBDriver.harmonicMixing", false));
    specificSettings.push_back(Savable::SettingDescription(&SASignalID, "LibreVNAUSBDriver.signalID", true));
    specificSettings.push_back(Savable::SettingDescription(&VNASuppressInvalidPeaks, "LibreVNAUSBDriver.suppressInvalidPeaks", true));
    specificSettings.push_back(Savable::SettingDescription(&VNAAdjustPowerLevel, "LibreVNAUSBDriver.adjustPowerLevel", false));
    specificSettings.push_back(Savable::SettingDescription(&SAUseDFT, "LibreVNAUSBDriver.useDFT", true));
    specificSettings.push_back(Savable::SettingDescription(&SARBWLimitForDFT, "LibreVNAUSBDriver.RBWlimitDFT", 3000));
}

QString LibreVNAUSBDriver::getDriverName()
{
    return "LibreVNA/USB";
}

std::set<QString> LibreVNAUSBDriver::GetAvailableDevices()
{
    std::set<QString> serials;

    libusb_context *ctx;
    libusb_init(&ctx);
#if LIBUSB_API_VERSION >= 0x01000106
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
#endif

    SearchDevices([&serials](libusb_device_handle *, QString serial) -> bool {
        serials.insert(serial);
        return true;
    }, ctx, true);

    libusb_exit(ctx);

    return serials;
}

bool LibreVNAUSBDriver::connectTo(QString serial)
{
    if(connected) {
        disconnect();
    }

//    info = defaultInfo;
//    status = {};

    m_handle = nullptr;
//    infoValid = false;
    libusb_init(&m_context);
#if LIBUSB_API_VERSION >= 0x01000106
    libusb_set_option(m_context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
#endif

    SearchDevices([=](libusb_device_handle *handle, QString found_serial) -> bool {
        if(serial.isEmpty() || serial == found_serial) {
            // accept connection to this device
            this->serial = found_serial;
            m_handle = handle;
            // abort device search
            return false;
        } else {
            // not the requested device, continue search
            return true;
        }
    }, m_context, false);

    if(!m_handle) {
        QString message =  "No device found";
        InformationBox::ShowError("Error opening device", message);
        libusb_exit(m_context);
        throw std::runtime_error(message.toStdString());
    }

    // Found the correct device, now connect
    /* claim the interface */
    int ret = libusb_claim_interface(m_handle, 0);
    if (ret < 0) {
        libusb_close(m_handle);
        /* Failed to open */
        QString message =  "Failed to claim interface: \"";
        message.append(libusb_strerror((libusb_error) ret));
        message.append("\" Maybe you are already connected to this device?");
        qWarning() << message;
        InformationBox::ShowError("Error opening device", message);
        libusb_exit(m_context);
        throw std::runtime_error(message.toStdString());
    }
    qInfo() << "USB connection established" << Qt::flush;
    connected = true;
    m_receiveThread = new std::thread(&LibreVNAUSBDriver::USBHandleThread, this);
    dataBuffer = new USBInBuffer(m_handle, EP_Data_In_Addr, 65536);
    logBuffer = new USBInBuffer(m_handle, EP_Log_In_Addr, 65536);
    connect(dataBuffer, &USBInBuffer::DataReceived, this, &LibreVNAUSBDriver::ReceivedData, Qt::DirectConnection);
    connect(dataBuffer, &USBInBuffer::TransferError, this, &LibreVNAUSBDriver::ConnectionLost);
    connect(logBuffer, &USBInBuffer::DataReceived, this, &LibreVNAUSBDriver::ReceivedLog, Qt::DirectConnection);
    connect(&transmissionTimer, &QTimer::timeout, this, &LibreVNAUSBDriver::transmissionTimeout, Qt::UniqueConnection);
    connect(this, &LibreVNAUSBDriver::receivedAnswer, this, &LibreVNAUSBDriver::transmissionFinished, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(this, &LibreVNAUSBDriver::receivedPacket, this, &LibreVNAUSBDriver::handleReceivedPacket, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    transmissionTimer.setSingleShot(true);
    transmissionActive = false;

    sendWithoutPayload(Protocol::PacketType::RequestDeviceInfo);
    sendWithoutPayload(Protocol::PacketType::RequestDeviceStatus);
//    updateIFFrequencies();
    return true;
}

void LibreVNAUSBDriver::disconnect()
{
    if(connected) {
        setIdle();
        delete dataBuffer;
        delete logBuffer;
        connected = false;
        serial = "";
        for (int if_num = 0; if_num < 1; if_num++) {
            int ret = libusb_release_interface(m_handle, if_num);
            if (ret < 0) {
                qCritical() << "Error releasing interface" << libusb_error_name(ret);
            }
        }
        libusb_release_interface(m_handle, 0);
        libusb_close(m_handle);
        m_receiveThread->join();
        libusb_exit(m_context);
        delete m_receiveThread;
        m_handle = nullptr;
        m_context = nullptr;
        m_receiveThread = nullptr;
        dataBuffer = nullptr;
        logBuffer = nullptr;
    }
}

void LibreVNAUSBDriver::ReceivedData()
{
    Protocol::PacketInfo packet;
    uint16_t handled_len;
//    qDebug() << "Received data";
    do {
        // qDebug() << "Decoding" << dataBuffer->getReceived() << "Bytes";
        handled_len = Protocol::DecodeBuffer(dataBuffer->getBuffer(), dataBuffer->getReceived(), &packet);
        // qDebug() << "Handled" << handled_len << "Bytes, type:" << (int) packet.type;
        if(handled_len > 0) {
            auto &log = DevicePacketLog::getInstance();
            if(packet.type != Protocol::PacketType::None) {
                log.addPacket(packet, serial);
            } else {
                log.addInvalidBytes(dataBuffer->getBuffer(), handled_len, serial);
            }
        }
        dataBuffer->removeBytes(handled_len);
        if(packet.type == Protocol::PacketType::SetTrigger) {
            qDebug() << "Incoming set trigger from " << serial;
        }
        if(packet.type == Protocol::PacketType::ClearTrigger) {
            qDebug() << "Incoming clear trigger from " << serial;
        }
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
        case Protocol::PacketType::None:
            break;
        default:
            // pass on to LibreVNADriver class
            emit receivedPacket(packet);
            break;
        }
        // byteCnt += handled_len;
        // auto now = QDateTime::currentDateTime();
        // if(lastTimestamp.time().msecsTo(now.time()) > 1000) {
        //     lastTimestamp = now;
        //     constexpr unsigned int maxThroughput = 12000000 / 8;
        //     qDebug() << "USB throughput: " << byteCnt << "(" << (double) byteCnt * 100.0 / maxThroughput << "%)";
        //     byteCnt = 0;
        // }
    } while (handled_len > 0);
}

void LibreVNAUSBDriver::ReceivedLog()
{
    uint16_t handled_len;
    do {
        handled_len = 0;
        auto firstLinebreak = (uint8_t*) memchr(logBuffer->getBuffer(), '\n', logBuffer->getReceived());
        if(firstLinebreak) {
            handled_len = firstLinebreak - logBuffer->getBuffer();
            auto line = QString::fromLatin1((const char*) logBuffer->getBuffer(), handled_len - 1);
            emit LogLineReceived(line);
            logBuffer->removeBytes(handled_len + 1);
        }
    } while(handled_len > 0);
}

void LibreVNAUSBDriver::transmissionFinished(LibreVNADriver::TransmissionResult result)
{
    lock_guard<mutex> lock(transmissionMutex);
    // remove transmitted packet
    if(transmissionQueue.empty()) {
        qWarning() << "transmissionFinished with empty transmission queue, stray Ack? Result:" << result;
        return;
    }
    auto t = transmissionQueue.dequeue();
    // qDebug() << "Transmission finsished (packet type" << (int) t.packet.type <<",result" << result << "), queue at " << transmissionQueue.size();
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

bool LibreVNAUSBDriver::SendPacket(const Protocol::PacketInfo &packet, std::function<void (LibreVNADriver::TransmissionResult)> cb, unsigned int timeout)
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

void LibreVNAUSBDriver::USBHandleThread()
{
    qDebug() << "Receive thread started";
    while (connected) {
        libusb_handle_events(m_context);
    }
    qDebug() << "Disconnected, receive thread exiting";
}

void LibreVNAUSBDriver::SearchDevices(std::function<bool (libusb_device_handle *, QString)> foundCallback, libusb_context *context, bool ignoreOpenError)
{
    libusb_device **devList;
    auto ndevices = libusb_get_device_list(context, &devList);

    for (ssize_t idx = 0; idx < ndevices; idx++) {
        int ret;
        libusb_device *device = devList[idx];
        libusb_device_descriptor desc = {};

        ret = libusb_get_device_descriptor(device, &desc);
        if (ret) {
            /* some error occured */
            qCritical() << "Failed to get device descriptor: "
                    << libusb_strerror((libusb_error) ret);
            continue;
        }

        bool correctID = false;
        int numIDs = sizeof(IDs)/sizeof(IDs[0]);
        for(int i=0;i<numIDs;i++) {
            if(desc.idVendor == IDs[i].VID && desc.idProduct == IDs[i].PID) {
                correctID = true;
                break;
            }
        }
        if(!correctID) {
            continue;
        }

        /* Try to open the device */
        libusb_device_handle *handle = nullptr;
        ret = libusb_open(device, &handle);
        if (ret) {
            qDebug() << libusb_strerror((enum libusb_error) ret);
            /* Failed to open */
            if(!ignoreOpenError) {
                QString message =  "Found potential device but failed to open usb connection: \"";
                message.append(libusb_strerror((libusb_error) ret));
                message.append("\" On Linux this is most likely caused by a missing udev rule. "
                               "On Windows this most likely means that you are already connected to "
                               "this device (is another instance of the application already runnning?)");
                qWarning() << message;
                InformationBox::ShowMessage("Error opening device", message);
            }
            continue;
        }

        char c_product[256];
        char c_serial[256];
        libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber,
                (unsigned char*) c_serial, sizeof(c_serial));
        ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct,
                (unsigned char*) c_product, sizeof(c_product));
        if (ret > 0) {
            /* managed to read the product string */
            QString product(c_product);
            if (product == "VNA") {
                // this is a match
                if(!foundCallback(handle, QString(c_serial))) {
                    // abort search
                    break;
                }
            }
        } else {
            qWarning() << "Failed to get product descriptor: "
                    << libusb_strerror((libusb_error) ret);
        }
        libusb_close(handle);
    }
    libusb_free_device_list(devList, 1);
}

bool LibreVNAUSBDriver::startNextTransmission()
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
    int actual_length;
    auto &log = DevicePacketLog::getInstance();
    log.addPacket(t.packet);
    auto ret = libusb_bulk_transfer(m_handle, EP_Data_Out_Addr, buffer, length, &actual_length, 0);
    if(ret < 0) {
        qCritical() << "Error sending data: "
                                << libusb_strerror((libusb_error) ret);
        return false;
    }
    transmissionTimer.start(t.timeout);
    // qDebug() << "Transmission started (packet type" << (int) t.packet.type << "), queue at " << transmissionQueue.size();
    return true;
}
