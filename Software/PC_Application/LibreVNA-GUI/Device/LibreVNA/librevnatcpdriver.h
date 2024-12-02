#ifndef LIBREVNATCPDRIVER_H
#define LIBREVNATCPDRIVER_H

#include "librevnadriver.h"

#include <condition_variable>
#include <thread>
#include <deque>

#include <QQueue>
#include <QTimer>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QDateTime>

class LibreVNATCPDriver : public LibreVNADriver
{
    Q_OBJECT
public:
    LibreVNATCPDriver();

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

    /**
     * @brief Registers metatypes within the Qt Framework.
     *
     * If the device driver uses a queued signal/slot connection with custom data types, these types must be registered before emitting the signal.
     * Register them within this function with qRegisterMetaType<Type>("Name");
     */
    virtual void registerTypes() override;

    void copyDetectedDevices(const LibreVNATCPDriver &other) {
        detectedDevices = other.detectedDevices;
    }

private slots:
    void SSDRequest();
    void SSDPreceived(QUdpSocket *sock);
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

    // Sockets for SSDP protocol
    std::vector<QUdpSocket*> ssdpSockets;
    class DetectedDevice {
    public:
        QString serial;
        QHostAddress address;
        QDateTime responseTime;
        int maxAgeSeconds;
    };

    void addDetectedDevice(const DetectedDevice &d);
    // removes entries that are too old
    void pruneDetectedDevices();
    std::vector<DetectedDevice> detectedDevices;

    // Connection sockets
    QTcpSocket dataSocket;
    QTcpSocket logSocket;

    QByteArray dataBuffer;
    QByteArray logBuffer;

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
    QTimer ssdpTimer;
    bool transmissionActive;

    std::thread *m_receiveThread;

    std::mutex accessMutex;
};

#endif // LibreVNATCPDriver_H
