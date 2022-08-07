#ifndef DEVICE_H
#define DEVICE_H

#include "../VNA_embedded/Application/Communication/Protocol.hpp"

#include <functional>
#include <libusb-1.0/libusb.h>
#include <thread>
#include <QObject>
#include <condition_variable>
#include <set>
#include <QQueue>
#include <QTimer>
#include <mutex>

Q_DECLARE_METATYPE(Protocol::Datapoint)
Q_DECLARE_METATYPE(Protocol::ManualStatusV1)
Q_DECLARE_METATYPE(Protocol::SpectrumAnalyzerResult)
Q_DECLARE_METATYPE(Protocol::AmplitudeCorrectionPoint)

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
    std::condition_variable cv;
};


class Device : public QObject
{
    Q_OBJECT
public:
    enum class TransmissionResult {
        Ack,
        Nack,
        Timeout,
        InternalError,
    };
    Q_ENUM(TransmissionResult)

    // connect to a VNA device. If serial is specified only connecting to this device, otherwise to the first one found
    Device(QString serial = QString());
    ~Device();

    static void RegisterTypes();
    bool SendPacket(const Protocol::PacketInfo& packet, std::function<void(TransmissionResult)> cb = nullptr, unsigned int timeout = 500);
    bool Configure(Protocol::SweepSettings settings, std::function<void(TransmissionResult)> cb = nullptr);
    bool Configure(Protocol::SpectrumAnalyzerSettings settings, std::function<void(TransmissionResult)> cb = nullptr);
    bool SetManual(Protocol::ManualControlV1 manual);
    bool SetIdle(std::function<void(TransmissionResult)> cb = nullptr);
    bool SendFirmwareChunk(Protocol::FirmwarePacket &fw);
    bool SendCommandWithoutPayload(Protocol::PacketType type, std::function<void(TransmissionResult)> cb = nullptr);
    QString serial() const;
    const Protocol::DeviceInfo& Info();
    static const Protocol::DeviceInfo& Info(Device *dev);
    Protocol::DeviceStatusV1& StatusV1();
    static const Protocol::DeviceStatusV1& StatusV1(Device *dev);
    QString getLastDeviceInfoString();

    // Returns serial numbers of all connected devices
    static std::set<QString> GetDevices();
signals:
    void DatapointReceived(Protocol::VNADatapoint<32>*);
    void ManualStatusReceived(Protocol::ManualStatusV1);
    void SpectrumResultReceived(Protocol::SpectrumAnalyzerResult);
    void AmplitudeCorrectionPointReceived(Protocol::AmplitudeCorrectionPoint);
    void FrequencyCorrectionReceived(float ppm);
    void DeviceInfoUpdated();
    void DeviceStatusUpdated();
    void ConnectionLost();
    void AckReceived();
    void NackReceived();
    void TriggerReceived(bool set);
    void LogLineReceived(QString line);
    void NeedsFirmwareUpdate(int usedProtocol, int requiredProtocol);
public slots:
    void SetTrigger(bool set);
private slots:
    void ReceivedData();
    void ReceivedLog();
    void transmissionTimeout() {
        transmissionFinished(TransmissionResult::Timeout);
    }
    void transmissionFinished(TransmissionResult result);
signals:
    void receivedAnswer(TransmissionResult result);

private:
    static constexpr int EP_Data_Out_Addr = 0x01;
    static constexpr int EP_Data_In_Addr = 0x81;
    static constexpr int EP_Log_In_Addr = 0x82;

    void USBHandleThread();
    // foundCallback is called for every device that is found. If it returns true the search continues, otherwise it is aborted.
    // When the search is aborted the last found device is still opened
    static void SearchDevices(std::function<bool(libusb_device_handle *handle, QString serial)> foundCallback, libusb_context *context, bool ignoreOpenError);

    libusb_device_handle *m_handle;
    libusb_context *m_context;
    USBInBuffer *dataBuffer;
    USBInBuffer *logBuffer;

    using Transmission = struct {
        Protocol::PacketInfo packet;
        unsigned int timeout;
        std::function<void(TransmissionResult)> callback;
    };

    std::mutex transmissionMutex;
    QQueue<Transmission> transmissionQueue;
    bool startNextTransmission();
    QTimer transmissionTimer;
    bool transmissionActive;

    QString m_serial;
    bool m_connected;
    std::thread *m_receiveThread;
    Protocol::DeviceInfo info;
    bool infoValid;
    union {
        Protocol::DeviceStatusV1 v1;
    } status;
};

#endif // DEVICE_H
