#include "usbdevice.h"

#include "CustomWidgets/informationbox.h"

#include <signal.h>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QDateTime>
#include <mutex>

using namespace std;

using USBID = struct {
    int VID;
    int PID;
};
static constexpr USBID IDs[] = {
    {0x0483, 0x4122},
    {0x1209, 0x4122},
};

USBDevice::USBDevice(QString serial)
{
    m_handle = nullptr;
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
        if(!serial.isEmpty()) {
            // only show error message if specific device was requested
            InformationBox::ShowError("Error opening device", message);
        }
        libusb_exit(m_context);
        throw std::runtime_error(message.toStdString());
        return;
    }

    // Found the correct device, now connect
    /* claim the interface */
    int ret = libusb_claim_interface(m_handle, 2);
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
    m_receiveThread = new std::thread(&USBDevice::USBHandleThread, this);
    usbBuffer = new USBInBuffer(m_handle, LIBUSB_ENDPOINT_IN | 0x03, 65536);
    connect(usbBuffer, &USBInBuffer::DataReceived, this, &USBDevice::ReceivedData, Qt::DirectConnection);
}

USBDevice::~USBDevice()
{
    delete usbBuffer;
    connected = false;
    libusb_release_interface(m_handle, 2);
    libusb_close(m_handle);
    m_receiveThread->join();
    libusb_exit(m_context);
    delete m_receiveThread;
}

bool USBDevice::Cmd(QString cmd)
{
    QString rcv;
    flushReceived();
    bool success = send(cmd) && receive(&rcv);
    if(success && rcv == "") {
        // empty response expected by commad
        return true;
    } else {
        // failed to send/receive
        emit communicationFailure();
        return false;
    }
}

QString USBDevice::Query(QString query)
{
    flushReceived();
    if(send(query)) {
        QString rcv;
        if(receive(&rcv)) {
            return rcv;
        } else {
            emit communicationFailure();
        }
    } else {
        emit communicationFailure();
    }
    return QString();
}


std::set<QString> USBDevice::GetDevices()
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

void USBDevice::SearchDevices(std::function<bool (libusb_device_handle *, QString)> foundCallback, libusb_context *context, bool ignoreOpenError)
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
                InformationBox::ShowError("Error opening device", message);
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
            if (product == "LibreCAL") {
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

bool USBDevice::send(const QString &s)
{
    unsigned char data[s.size()+2];
    memcpy(data, s.toLatin1().data(), s.size());
    memcpy(&data[s.size()], "\r\n", 2);
    int actual;
    auto r = libusb_bulk_transfer(m_handle, LIBUSB_ENDPOINT_OUT | 0x03, data, s.size() + 2, &actual, 0);
    if(r == 0 && actual == s.size() + 2) {
        return true;
    } else {
        return false;
    }
}

bool USBDevice::receive(QString *s, unsigned int timeout)
{
    // check if we already have a line queued
    unique_lock<mutex> lck(mtx);
    while(lineBuffer.size() == 0) {
        // needs to wait for an incoming line
        using namespace std::chrono_literals;
        if(cv.wait_for(lck, std::chrono::milliseconds(timeout)) == cv_status::timeout) {
            qWarning() << "Timed out while waiting for received line";
            return false;
        }
    }
    *s = lineBuffer.takeFirst();
    return true;
}

void USBDevice::ReceivedData()
{
    uint16_t handled_len;
    unique_lock<mutex> lck(mtx);
    do {
        handled_len = 0;
        auto firstLinebreak = (uint8_t*) memchr(usbBuffer->getBuffer(), '\n', usbBuffer->getReceived());
        if(firstLinebreak) {
            handled_len = firstLinebreak - usbBuffer->getBuffer();
            auto line = QString::fromLatin1((const char*) usbBuffer->getBuffer(), handled_len - 1);

            // add received line to buffer
            lineBuffer.append(line);

            usbBuffer->removeBytes(handled_len + 1);
        }
    } while(handled_len > 0);
    if(lineBuffer.size() > 0) {
        cv.notify_one();
    }
}

void USBDevice::flushReceived()
{
    unique_lock<mutex> lck(mtx);
    lineBuffer.clear();
}

void USBDevice::USBHandleThread()
{
    qDebug() << "Receive thread started";
    while (connected) {
        libusb_handle_events(m_context);
    }
    qDebug() << "Disconnected, receive thread exiting";
}

QString USBDevice::serial() const
{
    return m_serial;
}
