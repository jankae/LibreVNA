#ifndef LIBREVNADRIVER_H
#define LIBREVNADRIVER_H

#include "devicedriver.h"

#include "../../VNA_embedded/Application/Communication/Protocol.hpp"

class LibreVNADriver : public DeviceDriver
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

    LibreVNADriver();

protected:
    virtual bool SendPacket(const Protocol::PacketInfo& packet, std::function<void(TransmissionResult)> cb = nullptr, unsigned int timeout = 500) = 0;
    virtual void ReceivedData(const uint8_t data, unsigned int len) = 0;

    bool connected;
};

#endif // LIBREVNADRIVER_H
