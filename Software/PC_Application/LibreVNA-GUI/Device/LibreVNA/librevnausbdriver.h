#ifndef LIBREVNAUSBDRIVER_H
#define LIBREVNAUSBDRIVER_H

#include "librevnadriver.h"
#include "Util/usbinbuffer.h"

#include <thread>

#include <QQueue>
#include <QTimer>
#include <QDateTime>

class LibreVNAUSBDriver : public LibreVNADriver
{
    Q_OBJECT
public:
    LibreVNAUSBDriver();

    /**
     * @brief Returns the driver name. It must be unique across all implemented drivers and is used to identify the driver
     * @return driver name
     */
    virtual QString getDriverName() override;
    /**
     * @brief Lists all available devices by their serial numbers
     * @return Serial numbers of detected devices
     */
    virtual std::set<QString> GetAvailableDevices() override;
    /**
     * @brief Connects to a device, given by its serial number
     * @param serial Serial number of device that should be connected to
     * @return true if connection successful, otherwise false
     */
    virtual bool connectTo(QString serial) override;
    /**
     * @brief Disconnects from device. Has no effect if no device was connected
     */
    virtual void disconnect() override;

private slots:
    void ReceivedData();
    void ReceivedLog();
    void transmissionTimeout() {
        transmissionFinished(TransmissionResult::Timeout);
    }
    void transmissionFinished(TransmissionResult result);
private:
    static constexpr int EP_Data_Out_Addr = 0x01;
    static constexpr int EP_Data_In_Addr = 0x81;
    static constexpr int EP_Log_In_Addr = 0x82;

    virtual bool SendPacket(const Protocol::PacketInfo& packet, std::function<void(TransmissionResult)> cb = nullptr, unsigned int timeout = 500) override;

    void USBHandleThread();
    // foundCallback is called for every device that is found. If it returns true the search continues, otherwise it is aborted.
    // When the search is aborted the last found device is still opened
    static void SearchDevices(std::function<bool(libusb_device_handle *handle, QString getSerial)> foundCallback, libusb_context *context, bool ignoreOpenError);

    libusb_device_handle *m_handle;
    libusb_context *m_context;
    USBInBuffer *dataBuffer;
    USBInBuffer *logBuffer;

    class Transmission {
    public:
        Protocol::PacketInfo packet;
        unsigned int timeout;
        std::function<void(TransmissionResult)> callback;
    };

    std::mutex transmissionMutex;
    QQueue<Transmission> transmissionQueue;
    bool startNextTransmission();
    QTimer transmissionTimer;
    bool transmissionActive;

    std::thread *m_receiveThread;

    std::mutex accessMutex;

    // USB throughput indicator
    QDateTime lastTimestamp;
    unsigned long byteCnt;

};

#endif // LIBREVNAUSBDRIVER_H
