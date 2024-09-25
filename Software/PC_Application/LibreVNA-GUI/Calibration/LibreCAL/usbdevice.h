#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "Util/usbinbuffer.h"

#include <libusb-1.0/libusb.h>
#include <QString>
#include <set>
#include <functional>
#include <mutex>
#include <thread>

#include <QObject>

class USBDevice : public QObject
{
    Q_OBJECT
public:
    // connect to a CAL device. If serial is specified only connecting to this device, otherwise to the first one found
    USBDevice(QString serial = QString());
    ~USBDevice();

    bool Cmd(QString cmd);
    QString Query(QString query);
    QString serial() const;

    // Returns serial numbers of all connected devices
    static std::set<QString> GetDevices();

    bool send(const QString &s);
    bool receive(QString *s, unsigned int timeout = 2000);
    void flushReceived();
signals:
    void communicationFailure();

private slots:
    void ReceivedData();
private:
    void USBHandleThread();
    bool connected;
    std::thread *m_receiveThread;

    static void SearchDevices(std::function<bool (libusb_device_handle *, QString)> foundCallback, libusb_context *context, bool ignoreOpenError);
    libusb_device_handle *m_handle;
    libusb_context *m_context;

    USBInBuffer *usbBuffer;
    std::mutex mtx;
    std::condition_variable cv;
    QStringList lineBuffer;

    QString m_serial;
};

#endif // DEVICE_H
