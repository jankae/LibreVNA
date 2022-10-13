#include "compounddevice.h"

CompoundDevice::CompoundDevice()
{
    name = "";
    sync = Synchronization::USB;
}

nlohmann::json CompoundDevice::toJSON()
{
    nlohmann::json j;
    j["name"] = name.toStdString();
    j["synchronization"] = SyncToString(sync).toStdString();
    nlohmann::json jserials;
    for(auto d : deviceSerials) {
        jserials.push_back(d.toStdString());
    }
    j["devices"] = jserials;
    nlohmann::json jmappings;
    for(auto m : portMapping) {
        nlohmann::json jmapping;
        jmapping["device"] = m.device;
        jmapping["port"] = m.port;
        jmappings.push_back(jmapping);
    }
    j["mapping"] = jmappings;

    return j;
}

void CompoundDevice::fromJSON(nlohmann::json j)
{
    name = QString::fromStdString(j.value("name", "CompoundDevice"));
    sync = SyncFromString(QString::fromStdString(j.value("synchronization", "USB")));
    deviceSerials.clear();
    if(j.contains("devices")) {
        for(auto js : j["devices"]) {
            deviceSerials.push_back(QString::fromStdString(js));
        }
    }
    portMapping.clear();
    if(j.contains("mapping")) {
        for(auto jm : j["mapping"]) {
            PortMapping mapping;
            mapping.device = jm.value("device", 0);
            mapping.port = jm.value("port", 0);
            portMapping.push_back(mapping);
        }
    }
}

QString CompoundDevice::SyncToString(CompoundDevice::Synchronization sync)
{
    switch(sync) {
    case Synchronization::USB: return "USB";
    case Synchronization::ExtRef: return "Ext. Ref.";
    case Synchronization::Trigger: return "Trigger";
    default:
    case Synchronization::Last: return "Invalid";
    }
}

CompoundDevice::Synchronization CompoundDevice::SyncFromString(QString s)
{
    for(int i=0;i<(int) Synchronization::Last;i++) {
        if(SyncToString((Synchronization)i) == s) {
            return (Synchronization) i;
        }
    }
    // default to USB
    return Synchronization::USB;
}

QString CompoundDevice::getDesription()
{
    return name + ", "+QString::number(deviceSerials.size())+" devices, "+QString::number(portMapping.size())+" ports in total";
}

int CompoundDevice::PortMapping::findActiveStage(std::vector<CompoundDevice::PortMapping> map, unsigned int device, unsigned int port)
{
    for(unsigned int i=0;i<map.size();i++) {
        if(map[i].device == device && map[i].port == port) {
            return i;
        }
    }
    return map.size();
}
