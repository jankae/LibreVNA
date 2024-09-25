#ifndef USBINBUFFER_H
#define USBINBUFFER_H

#include <libusb-1.0/libusb.h>
#include <condition_variable>

#include <QObject>

class USBInBuffer : public QObject {
    Q_OBJECT
public:
    USBInBuffer(libusb_device_handle *handle, unsigned char endpoint, int buffer_size);
    ~USBInBuffer();

    void removeBytes(int handled_bytes);
    int getReceived() const;
    uint8_t *getBuffer() const;

signals:
    void DataReceived();
    void TransferError();

private:
    void Callback(libusb_transfer *transfer);
    static void LIBUSB_CALL CallbackTrampoline(libusb_transfer *transfer);
    libusb_transfer *transfer;
    unsigned char *buffer;
    int buffer_size;
    int received_size;
    bool inCallback;
    bool cancelling;
    std::condition_variable cv;
};

#endif // USBINBUFFER_H
