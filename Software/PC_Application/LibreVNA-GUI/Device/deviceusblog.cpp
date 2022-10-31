#include "deviceusblog.h"

#include "preferences.h"

#include <QDateTime>

DeviceUSBLog::DeviceUSBLog()
    : usedStorageSize(0)
{
    auto &pref = Preferences::getInstance();
    maxStorageSize = pref.Debug.USBlogSizeLimit;
}

DeviceUSBLog::~DeviceUSBLog()
{

}

void DeviceUSBLog::reset()
{
    entries.clear();
    usedStorageSize = 0;
}

void DeviceUSBLog::addPacket(Protocol::PacketInfo &p, QString serial)
{
    LogEntry e;
    e.timestamp = QDateTime::currentDateTimeUtc();
    e.serial = serial;
    e.type = LogEntry::Type::Packet;
    e.p = new Protocol::PacketInfo;
    *e.p = p;
    addEntry(e);
}

void DeviceUSBLog::addInvalidBytes(const uint8_t *bytes, uint16_t len, QString serial)
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

nlohmann::json DeviceUSBLog::toJSON()
{
    nlohmann::json j;
    for(auto &e : entries) {
        j.push_back(e.toJSON());
    }
    return j;
}

void DeviceUSBLog::fromJSON(nlohmann::json j)
{
    reset();
    for(auto jd : j) {
        LogEntry e;
        e.fromJSON(jd);
        addEntry(e);
    }
}

void DeviceUSBLog::addEntry(const DeviceUSBLog::LogEntry &e)
{
    usedStorageSize += e.storageSize();
    while(usedStorageSize > maxStorageSize) {
        usedStorageSize -= entries.front().storageSize();
        entries.pop_front();
    }
    entries.push_back(e);
    emit entryAdded(e);
}

std::deque<DeviceUSBLog::LogEntry> DeviceUSBLog::getEntries() const
{
    return entries;
}

nlohmann::json DeviceUSBLog::LogEntry::toJSON()
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
    } else {
        for(auto b : bytes) {
            jdata.push_back(b);
        }
    }
    j["data"] = jdata;
    return j;
}

void DeviceUSBLog::LogEntry::fromJSON(nlohmann::json j)
{
    type = QString::fromStdString(j.value("type", "")) == "Packet" ? Type::Packet : Type::InvalidBytes;
    timestamp = QDateTime::fromMSecsSinceEpoch(j.value("timestamp", 0UL), Qt::TimeSpec::UTC);
    serial = QString::fromStdString(j.value("serial", ""));
    if(type == Type::Packet) {
        p = new Protocol::PacketInfo;
        auto jdata = j["data"];
        for(unsigned int i=0;i<sizeof(Protocol::PacketInfo);i++) {
            *(((uint8_t*) p) + i) = jdata[i];
        }
    } else {
        for(auto v : j["data"]) {
            bytes.push_back(v);
        }
    }
}
