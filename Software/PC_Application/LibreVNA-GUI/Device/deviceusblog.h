#ifndef DEVICEUSBLOG_H
#define DEVICEUSBLOG_H

#include "../../VNA_embedded/Application/Communication/Protocol.hpp"

#include "savable.h"

#include <deque>
#include <cstdint>
#include <QDateTime>
#include <QObject>
#include <mutex>

class DeviceUSBLog : public QObject, public Savable
{
    Q_OBJECT
public:
    static DeviceUSBLog& getInstance() {
        static DeviceUSBLog instance;
        return instance;
    }
    DeviceUSBLog(const DeviceUSBLog&) = delete;
    virtual ~DeviceUSBLog();

    void reset();

    void addPacket(Protocol::PacketInfo &p, QString serial = "");
    void addInvalidBytes(const uint8_t *bytes, uint16_t len, QString serial = "");

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;


    class LogEntry : public Savable {
    public:
        LogEntry()
            : type(Type::InvalidBytes), timestamp(QDateTime()), serial(""), p(nullptr) {}
        ~LogEntry() {
            delete p;
        }

        LogEntry(const LogEntry &e);

        enum class Type {
            Packet,
            InvalidBytes,
        };
        Type type;
        QDateTime timestamp;
        QString serial;
        std::vector<uint8_t> bytes;
        Protocol::PacketInfo *p;
        Protocol::VNADatapoint<32> *datapoint;
        unsigned int storageSize() const {
            unsigned long size = sizeof(type) + sizeof(timestamp) + serial.size();
            switch(type) {
            case Type::InvalidBytes: size += bytes.size(); break;
            case Type::Packet: size += sizeof(Protocol::PacketInfo); break;
            }
            return size;
        }

        virtual nlohmann::json toJSON() override;
        virtual void fromJSON(nlohmann::json j) override;
    };

    LogEntry getEntry(unsigned int index);

    unsigned long getUsedStorageSize() const;
    unsigned long getMaxStorageSize() const;

signals:
    void entryAdded(const LogEntry &e);

private:
    DeviceUSBLog();

    void addEntry(const LogEntry &e);

    unsigned long maxStorageSize;
    unsigned long usedStorageSize;
    std::deque<LogEntry> entries;

    std::mutex access;
};

#endif // DEVICEUSBLOG_H
