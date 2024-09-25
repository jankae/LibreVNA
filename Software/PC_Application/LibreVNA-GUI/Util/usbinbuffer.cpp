#include "usbinbuffer.h"

#include <mutex>

#include <QDebug>

using namespace std;

USBInBuffer::USBInBuffer(libusb_device_handle *handle, unsigned char endpoint, int buffer_size) :
    buffer_size(buffer_size),
    received_size(0),
    inCallback(false),
    cancelling(false)
{
    buffer = new unsigned char[buffer_size];
    memset(buffer, 0, buffer_size);
    transfer = libusb_alloc_transfer(0);
    libusb_fill_bulk_transfer(transfer, handle, endpoint, buffer, buffer_size, CallbackTrampoline, this, 0);
    libusb_submit_transfer(transfer);
}

USBInBuffer::~USBInBuffer()
{
    if(transfer) {
        cancelling = true;
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
    if(cancelling || (transfer->status == LIBUSB_TRANSFER_CANCELLED)) {
        // destructor called, do not resubmit
        libusb_free_transfer(transfer);
        this->transfer = nullptr;
        cv.notify_all();
        return;
    }
    switch(transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
    case LIBUSB_TRANSFER_TIMED_OUT:
        if(transfer->actual_length > 0) {
            received_size += transfer->actual_length;
            inCallback = true;
            emit DataReceived();
            inCallback = false;
        }
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
    case LIBUSB_TRANSFER_CANCELLED:
        // already handled before switch-case
        break;
    }
    // Resubmit the transfer
    transfer->buffer = &buffer[received_size];
    transfer->length = buffer_size - received_size;
    transfer->length = (transfer->length / 512) * 512;
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
