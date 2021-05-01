#include "device.h"

#include <signal.h>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <mutex>

using namespace std;

using USBID = struct {
    int VID;
    int PID;
};
static constexpr USBID IDs[] = {
    {0x0483, 0x564e},
    {0x0483, 0x4121},
};

USBInBuffer::USBInBuffer(libusb_device_handle *handle, unsigned char endpoint, int buffer_size) :
    buffer_size(buffer_size),
    received_size(0),
    inCallback(false)
{
    buffer = new unsigned char[buffer_size];
    transfer = libusb_alloc_transfer(0);
    libusb_fill_bulk_transfer(transfer, handle, endpoint, buffer, buffer_size, CallbackTrampoline, this, 100);
    libusb_submit_transfer(transfer);
}

USBInBuffer::~USBInBuffer()
{
    if(transfer) {
        libusb_cancel_transfer(transfer);
        // wait for cancellation to complete
        mutex mtx;
        unique_lock<mutex> lck(mtx);
        using namespace std::chrono_literals;
        if(cv.wait_for(lck, 100ms) == cv_status::timeout) {
            qWarning() << "Timed out waiting for mutex acquisition during disconnect";
        }
    }
    delete[] buffer;
}

void USBInBuffer::removeBytes(int handled_bytes)
{
    if(!inCallback) {
        throw runtime_error("Removing of bytes is only allowed from within receive callback");
    }
    if(handled_bytes >= received_size) {
        received_size = 0;
    } else {
        // not removing all bytes, have to move remaining data to the beginning of the buffer
        memmove(buffer, &buffer[handled_bytes], received_size - handled_bytes);
        received_size -= handled_bytes;
    }
}

int USBInBuffer::getReceived() const
{
    return received_size;
}

void USBInBuffer::Callback(libusb_transfer *transfer)
{
    switch(transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
        received_size += transfer->actual_length;
        inCallback = true;
        emit DataReceived();
        inCallback = false;
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        qCritical() << "LIBUSB_TRANSFER_NO_DEVICE";
        libusb_free_transfer(transfer);
        return;
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_OVERFLOW:
    case LIBUSB_TRANSFER_STALL:
        qCritical() << "LIBUSB_ERROR" << transfer->status;
        libusb_free_transfer(transfer);
        this->transfer = nullptr;
        emit TransferError();
        return;
        break;
    case LIBUSB_TRANSFER_TIMED_OUT:
        // nothing to do
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        // destructor called, do not resubmit
        libusb_free_transfer(transfer);
        this->transfer = nullptr;
        cv.notify_all();
        return;
        break;
    }
    // Resubmit the transfer
    transfer->buffer = &buffer[received_size];
    transfer->length = buffer_size - received_size;
    libusb_submit_transfer(transfer);
}

void USBInBuffer::CallbackTrampoline(libusb_transfer *transfer)
{
    auto usb = (USBInBuffer*) transfer->user_data;
    usb->Callback(transfer);
}

uint8_t *USBInBuffer::getBuffer() const
{
    return buffer;
}

static constexpr Protocol::DeviceInfo defaultInfo = {
    .ProtocolVersion = Protocol::Version,
    .FW_major = 0,
    .FW_minor = 0,
    .FW_patch = 0,
    .HW_Revision = '0',
    .extRefAvailable = 0,
    .extRefInUse = 0,
    .FPGA_configured = 0,
    .source_locked = 0,
    .LO1_locked = 0,
    .ADC_overload = 0,
    .unlevel = 0,
    .temp_source = 0,
    .temp_LO1 = 0,
    .temp_MCU = 0,
    .limits_minFreq = 0,
    .limits_maxFreq = 6000000000,
    .limits_minIFBW = 10,
    .limits_maxIFBW = 1000000,
    .limits_maxPoints = 10000,
    .limits_cdbm_min = -10000,
    .limits_cdbm_max = 1000,
    .limits_minRBW = 1,
    .limits_maxRBW = 1000000,
    .limits_maxAmplitudePoints = 255,
    .limits_maxFreqHarmonic = 18000000000,
};

Protocol::DeviceInfo Device::lastInfo = defaultInfo;

Device::Device(QString serial)
{
    lastInfo = defaultInfo;

    m_handle = nullptr;
    lastInfoValid = false;
    libusb_init(&m_context);
#if LIBUSB_API_VERSION >= 0x01000106
    libusb_set_option(m_context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
#endif

    SearchDevices([=](libusb_device_handle *handle, QString found_serial) -> bool {
        if(serial.isEmpty() || serial == found_serial) {
            // accept connection to this device
            m_serial = found_serial;
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
        auto msg = new QMessageBox(QMessageBox::Icon::Warning, "Error opening device", message);
        msg->show();
        libusb_exit(m_context);
        throw std::runtime_error(message.toStdString());
        return;
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
        auto msg = new QMessageBox(QMessageBox::Icon::Warning, "Error opening device", message);
        msg->show();
        libusb_exit(m_context);
        throw std::runtime_error(message.toStdString());
    }
    qInfo() << "USB connection established" << flush;
    m_connected = true;
    m_receiveThread = new std::thread(&Device::USBHandleThread, this);
    dataBuffer = new USBInBuffer(m_handle, EP_Data_In_Addr, 65536);
    logBuffer = new USBInBuffer(m_handle, EP_Log_In_Addr, 65536);
    connect(dataBuffer, &USBInBuffer::DataReceived, this, &Device::ReceivedData, Qt::DirectConnection);
    connect(dataBuffer, &USBInBuffer::TransferError, this, &Device::ConnectionLost);
    connect(logBuffer, &USBInBuffer::DataReceived, this, &Device::ReceivedLog, Qt::DirectConnection);
    connect(&transmissionTimer, &QTimer::timeout, this, &Device::transmissionTimeout);
    connect(this, &Device::receivedAnswer, this, &Device::transmissionFinished, Qt::QueuedConnection);
    transmissionTimer.setSingleShot(true);
    transmissionActive = false;
    // got a new connection, request info
    SendCommandWithoutPayload(Protocol::PacketType::RequestDeviceInfo);
}

Device::~Device()
{
    if(m_connected) {
        SetIdle();
        delete dataBuffer;
        delete logBuffer;
        m_connected = false;
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
    }
}

bool Device::SendPacket(const Protocol::PacketInfo& packet, std::function<void(TransmissionResult)> cb, unsigned int timeout)
{
    Transmission t;
    t.packet = packet;
    t.timeout = timeout;
    t.callback = cb;
    transmissionQueue.enqueue(t);
//    qDebug() << "Enqueued packet, queue at " << transmissionQueue.size();
    if(!transmissionActive) {
        startNextTransmission();
    }
    return true;
}

bool Device::Configure(Protocol::SweepSettings settings, std::function<void(TransmissionResult)> cb)
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::SweepSettings;
    p.settings = settings;
    return SendPacket(p, cb);
}

bool Device::Configure(Protocol::SpectrumAnalyzerSettings settings)
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::SpectrumAnalyzerSettings;
    p.spectrumSettings = settings;
    return SendPacket(p);
}

bool Device::SetManual(Protocol::ManualControl manual)
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    p.manual = manual;
    return SendPacket(p);
}

bool Device::SetIdle()
{
    return SendCommandWithoutPayload(Protocol::PacketType::SetIdle);
}

bool Device::SendFirmwareChunk(Protocol::FirmwarePacket &fw)
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::FirmwarePacket;
    p.firmware = fw;
    return SendPacket(p);
}

bool Device::SendCommandWithoutPayload(Protocol::PacketType type)
{
    Protocol::PacketInfo p;
    p.type = type;
    return SendPacket(p);
}

std::set<QString> Device::GetDevices()
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

void Device::USBHandleThread()
{
    qDebug() << "Receive thread started";
    while (m_connected) {
        libusb_handle_events(m_context);
    }
    qDebug() << "Disconnected, receive thread exiting";
}

void Device::SearchDevices(std::function<bool (libusb_device_handle *, QString)> foundCallback, libusb_context *context, bool ignoreOpenError)
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
                               "this device (is another instance of the application already runnning? "
                               "If that is not the case, you can try installing the WinUSB driver using Zadig (https://zadig.akeo.ie/)");
                qWarning() << message;
                auto msg = new QMessageBox(QMessageBox::Icon::Warning, "Error opening device", message);
                msg->show();
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

const Protocol::DeviceInfo &Device::Info()
{
    return lastInfo;
}

QString Device::getLastDeviceInfoString()
{
    QString ret;
    if(!lastInfoValid) {
        ret.append("No device information available yet");
    } else {
        ret.append("HW Rev.");
        ret.append(lastInfo.HW_Revision);
        ret.append(" FW "+QString::number(lastInfo.FW_major)+"."+QString::number(lastInfo.FW_minor)+"."+QString::number(lastInfo.FW_patch));
        ret.append(" Temps: "+QString::number(lastInfo.temp_source)+"°C/"+QString::number(lastInfo.temp_LO1)+"°C/"+QString::number(lastInfo.temp_MCU)+"°C");
        ret.append(" Reference:");
        if(lastInfo.extRefInUse) {
            ret.append("External");
        } else {
            ret.append("Internal");
            if(lastInfo.extRefAvailable) {
                ret.append(" (External available)");
            }
        }
    }
    return ret;
}

void Device::ReceivedData()
{
    Protocol::PacketInfo packet;
    uint16_t handled_len;
//    qDebug() << "Received data";
    do {
        handled_len = Protocol::DecodeBuffer(dataBuffer->getBuffer(), dataBuffer->getReceived(), &packet);
        dataBuffer->removeBytes(handled_len);
        switch(packet.type) {
        case Protocol::PacketType::Datapoint:
            emit DatapointReceived(packet.datapoint);
            break;
        case Protocol::PacketType::Status:
            emit ManualStatusReceived(packet.status);
            break;
        case Protocol::PacketType::SpectrumAnalyzerResult:
            emit SpectrumResultReceived(packet.spectrumResult);
            break;
        case Protocol::PacketType::SourceCalPoint:
        case Protocol::PacketType::ReceiverCalPoint:
            emit AmplitudeCorrectionPointReceived(packet.amplitudePoint);
            break;
        case Protocol::PacketType::DeviceInfo:
            if(packet.info.ProtocolVersion != Protocol::Version) {
                if(!lastInfoValid) {
                emit NeedsFirmwareUpdate(packet.info.ProtocolVersion, Protocol::Version);
                }
            } else {
                lastInfo = packet.info;
            }
            lastInfoValid = true;
            emit DeviceInfoUpdated();
            break;
        case Protocol::PacketType::Ack:
            emit AckReceived();
            emit receivedAnswer(TransmissionResult::Ack);
            break;
        case Protocol::PacketType::Nack:
            emit NackReceived();
            emit receivedAnswer(TransmissionResult::Nack);
            break;
        case Protocol::PacketType::FrequencyCorrection:
            emit FrequencyCorrectionReceived(packet.frequencyCorrection.ppm);
            break;
       default:
            break;
        }
    } while (handled_len > 0);
}

void Device::ReceivedLog()
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

QString Device::serial() const
{
    return m_serial;
}

bool Device::startNextTransmission()
{
    if(transmissionQueue.isEmpty() || !m_connected) {
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
    auto ret = libusb_bulk_transfer(m_handle, EP_Data_Out_Addr, buffer, length, &actual_length, 0);
    if(ret < 0) {
        qCritical() << "Error sending data: "
                                << libusb_strerror((libusb_error) ret);
        return false;
    }
    transmissionTimer.start(t.timeout);
//    qDebug() << "Transmission started, queue at " << transmissionQueue.size();
    return true;
}

void Device::transmissionFinished(TransmissionResult result)
{
    // remove transmitted packet
//    qDebug() << "Transmission finsished (" << result << "), queue at " << transmissionQueue.size();
    if(transmissionQueue.empty()) {
        qWarning() << "transmissionFinished with empty transmission queue, stray Ack?";
        return;
    }
    if(result == TransmissionResult::Timeout) {
        qWarning() << "transmissionFinished with timeout";
    }
    if(result == TransmissionResult::Nack) {
        qWarning() << "transmissionFinished with NACK";
    }
    auto t = transmissionQueue.dequeue();
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
