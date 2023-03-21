#include "devicepacketlog.h"

#include "preferences.h"

#include <exception>

#include <QDateTime>

using namespace std;

DevicePacketLog::DevicePacketLog()
    : usedStorageSize(0)
{
    auto &pref = Preferences::getInstance();
    maxStorageSize = pref.Debug.USBlogSizeLimit;
}

DevicePacketLog::~DevicePacketLog()
{

}

void DevicePacketLog::reset()
{
    std::lock_guard<mutex> guard(access);
    entries.clear();
    usedStorageSize = 0;
}

void DevicePacketLog::addPacket(Protocol::PacketInfo &p, QString serial)
{
    LogEntry e;
    e.timestamp = QDateTime::currentDateTimeUtc();
    e.serial = serial;
    e.type = LogEntry::Type::Packet;
    e.p = new Protocol::PacketInfo;
    *e.p = p;
    if(p.type == Protocol::PacketType::VNADatapoint) {
        e.datapoint = new Protocol::VNADatapoint<32>(*p.VNAdatapoint);
    } else {
        e.datapoint = nullptr;
    }
    addEntry(e);
}

void DevicePacketLog::addInvalidBytes(const uint8_t *bytes, uint16_t len, QString serial)
{
    LogEntry e;
    e.timestamp = QDateTime::currentDateTimeUtc();
    e.serial = serial;
    e.type = LogEntry::Type::InvalidBytes;
    while(len--) {
        e.bytes.push_back(*bytes++);
    }
    addEntry(e);
}

nlohmann::json DevicePacketLog::toJSON()
{
    nlohmann::json j;
    for(auto &e : entries) {
        j.push_back(e.toJSON());
    }
    return j;
}

void DevicePacketLog::fromJSON(nlohmann::json j)
{
    reset();
    for(auto jd : j) {
        LogEntry e;
        e.fromJSON(jd);
        addEntry(e);
    }
}

DevicePacketLog::LogEntry DevicePacketLog::getEntry(unsigned int index)
{
    std::lock_guard<mutex> guard(access);
    if(index < entries.size()) {
        return entries[index];
    } else {
        throw std::runtime_error("Index too high");
    }
}

void DevicePacketLog::addEntry(const DevicePacketLog::LogEntry &e)
{
    std::lock_guard<mutex> guard(access);
    usedStorageSize += e.storageSize();
    while(usedStorageSize > maxStorageSize) {
        usedStorageSize -= entries.front().storageSize();
        entries.pop_front();
    }
    entries.push_back(e);
    emit entryAdded(e);
}

unsigned long DevicePacketLog::getMaxStorageSize() const
{
    return maxStorageSize;
}

unsigned long DevicePacketLog::getUsedStorageSize() const
{
    return usedStorageSize;
}

DevicePacketLog::LogEntry::LogEntry(const DevicePacketLog::LogEntry &e)
{
    timestamp = e.timestamp;
    type = e.type;
    serial = e.serial;
    bytes = e.bytes;
    if(e.p) {
        p = new Protocol::PacketInfo;
        *p = *e.p;
        if(p->type == Protocol::PacketType::VNADatapoint) {
            datapoint = new Protocol::VNADatapoint<32>(*e.datapoint);
        } else {
            datapoint = nullptr;
        }
    } else {
        datapoint = nullptr;
        p = nullptr;
    }
}

nlohmann::json DevicePacketLog::LogEntry::toJSON()
{
    nlohmann::json j;
    j["type"] = type == Type::Packet ? "Packet" : "InvalidBytes";
    j["timestamp"] = timestamp.toMSecsSinceEpoch();
    j["serial"] = serial.toStdString();
    nlohmann::json jdata;
    if(type == Type::Packet) {
        for(unsigned int i=0;i<sizeof(Protocol::PacketInfo);i++) {
            jdata.push_back(*(((uint8_t*) p) + i));
        }
        if(datapoint) {
            nlohmann::json jdatapoint;
            for(unsigned int i=0;i<sizeof(*datapoint);i++) {
                jdatapoint.push_back(*(((uint8_t*) datapoint) + i));
            }
            j["datapoint"] = jdatapoint;
        }
    } else {
        for(auto b : bytes) {
            jdata.push_back(b);
        }
    }
    j["data"] = jdata;
    return j;
}

void DevicePacketLog::LogEntry::fromJSON(nlohmann::json j)
{
    type = QString::fromStdString(j.value("type", "")) == "Packet" ? Type::Packet : Type::InvalidBytes;
    timestamp = QDateTime::fromMSecsSinceEpoch(j.value("timestamp", 0ULL), Qt::TimeSpec::UTC);
    serial = QString::fromStdString(j.value("serial", ""));
    datapoint = nullptr;
    p = nullptr;
    if(type == Type::Packet) {
        p = new Protocol::PacketInfo;
        auto jdata = j["data"];
        for(unsigned int i=0;i<sizeof(Protocol::PacketInfo);i++) {
            *(((uint8_t*) p) + i) = jdata[i];
        }
        if(j.contains("datapoint")) {
            datapoint = new Protocol::VNADatapoint<32>();
            auto jdatapoint = j["datapoint"];
            for(unsigned int i=0;i<sizeof(*datapoint);i++) {
                *(((uint8_t*) datapoint) + i) = jdatapoint[i];
            }
        }
    } else {
        for(auto v : j["data"]) {
            bytes.push_back(v);
        }
    }
}
