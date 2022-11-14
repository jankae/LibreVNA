#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <libusb-1.0/libusb.h>
#include <QString>
#include <set>
#include <functional>

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

signals:
    void communicationFailure();

private:
    static void SearchDevices(std::function<bool (libusb_device_handle *, QString)> foundCallback, libusb_context *context, bool ignoreOpenError);
    bool send(const QString &s);
    bool receive(QString *s);
    bool flushRX();
    libusb_device_handle *m_handle;
    libusb_context *m_context;

    QString m_serial;
};

#endif // DEVICE_H
