#include "devicedriver.h"

#include "LibreVNA/librevnatcpdriver.h"
#include "LibreVNA/librevnausbdriver.h"
#include "LibreVNA/Compound/compounddriver.h"
#include "SSA3000X/ssa3000xdriver.h"
#include "SNA5000A/sna5000adriver.h"

DeviceDriver *DeviceDriver::activeDriver = nullptr;

DeviceDriver::~DeviceDriver()
{
    for(auto a : specificActions) {
        delete a;
    }
}

std::vector<DeviceDriver *> DeviceDriver::getDrivers()
{
    static std::vector<DeviceDriver*> ret;
    if (ret.size() == 0) {
        // first function call
        ret.push_back(new LibreVNAUSBDriver);
        ret.push_back(new LibreVNATCPDriver);
        ret.push_back(new CompoundDriver);
        ret.push_back(new SSA3000XDriver);
        ret.push_back(new SNA5000ADriver);
    }
    return ret;
}

bool DeviceDriver::connectDevice(QString serial, bool isIndepedentDriver)
{
    if(!isIndepedentDriver) {
        if(activeDriver && activeDriver != this) {
            activeDriver->disconnect();
        }
    }
    if(connectTo(serial)) {
        if(!isIndepedentDriver) {
            activeDriver = this;
        }
        return true;
    } else {
        return false;
    }
}

void DeviceDriver::disconnectDevice()
{
    disconnect();
    activeDriver = nullptr;
}

unsigned int DeviceDriver::SApoints() {
    if(activeDriver) {
        return activeDriver->getSApoints();
    } else {
        // return default value instead
        return 1001;
    }
}

Sparam DeviceDriver::VNAMeasurement::toSparam(int ports) const
{
    if(ports == 0) {
        // determine number of ports by highest available S parameter
        for(const auto &m : measurements) {
            if(!m.first.startsWith("S")) {
                // something else we can not handle
                continue;
            }
            int to = m.first.mid(1,1).toUInt();
            int from = m.first.mid(2,1).toUInt();
            if(to > ports) {
                ports = to;
            }
            if(from > ports) {
                ports = from;
            }
        }
    }
    // create S paramters
    auto S = Sparam(ports);
    // fill data
    for(const auto &m : measurements) {
        if(!m.first.startsWith("S")) {
            // something else we can not handle
            continue;
        }
        int to = m.first.mid(1,1).toUInt();
        int from = m.first.mid(2,1).toUInt();
        S.set(to, from, m.second);
    }
    return S;
}

void DeviceDriver::VNAMeasurement::fromSparam(Sparam S, std::vector<unsigned int> portMapping)
{
    if(portMapping.size() == 0) {
        // set up default port mapping
        for(unsigned int i=1;i<=S.ports();i++) {
            portMapping.push_back(i);
        }
    }
    for(unsigned int i=0;i<portMapping.size();i++) {
        for(unsigned int j=0;j<portMapping.size();j++) {
            QString name = "S"+QString::number(i+1)+QString::number(j+1);
            if(measurements.count(name)) {
                measurements[name] = S.get(portMapping[i], portMapping[j]);
            }
        }
    }
}

DeviceDriver::VNAMeasurement DeviceDriver::VNAMeasurement::interpolateTo(const DeviceDriver::VNAMeasurement &to, double a)
{
    VNAMeasurement ret;
    ret.frequency = frequency * (1.0 - a) + to.frequency * a;
    ret.dBm = dBm * (1.0 - a) + to.dBm * a;
    ret.Z0 = Z0 * (1.0 - a) + to.Z0 * a;
    for(auto m : measurements) {
        if(to.measurements.count(m.first) == 0) {
            throw std::runtime_error("Nothing to interpolate to, expected measurement +\""+m.first.toStdString()+"\"");
        }
        ret.measurements[m.first] = measurements[m.first] * (1.0 - a) + to.measurements.at(m.first) * a;
    }
    return ret;
}

DeviceDriver::Info::Info()
{
    firmware_version = "missing";
    hardware_version = "missing";
    Limits.VNA.ports = 2;
    Limits.VNA.minFreq = 0;
    Limits.VNA.maxFreq = 100000000000;
    Limits.VNA.mindBm = -100;
    Limits.VNA.maxdBm = 30;
    Limits.VNA.minIFBW = 1;
    Limits.VNA.maxIFBW = 100000000;
    Limits.VNA.maxPoints = 65535;
    Limits.VNA.maxDwellTime = 1;

    Limits.Generator.ports = 2;
    Limits.Generator.minFreq = 0;
    Limits.Generator.maxFreq = 100000000000;
    Limits.Generator.mindBm = -100;
    Limits.Generator.maxdBm = 30;

    Limits.SA.ports = 2;
    Limits.SA.minFreq = 0;
    Limits.SA.maxFreq = 100000000000;
    Limits.SA.mindBm = -100;
    Limits.SA.maxdBm = 30;
    Limits.SA.minRBW = 1;
    Limits.SA.maxRBW = 100000000;
}

void DeviceDriver::Info::subset(const DeviceDriver::Info &info)
{
    if (info.firmware_version != firmware_version) {
        firmware_version = "Mixed";
    }
    if (info.hardware_version != hardware_version) {
        hardware_version = "Mixed";
    }

    Limits.VNA.ports += info.Limits.VNA.ports;
    Limits.VNA.minFreq = std::max(Limits.VNA.minFreq, info.Limits.VNA.minFreq);
    Limits.VNA.maxFreq = std::min(Limits.VNA.maxFreq, info.Limits.VNA.maxFreq);
    Limits.VNA.mindBm = std::max(Limits.VNA.mindBm, info.Limits.VNA.mindBm);
    Limits.VNA.maxdBm = std::min(Limits.VNA.maxdBm, info.Limits.VNA.maxdBm);
    Limits.VNA.minIFBW = std::max(Limits.VNA.minIFBW, info.Limits.VNA.minIFBW);
    Limits.VNA.maxIFBW = std::min(Limits.VNA.maxIFBW, info.Limits.VNA.maxIFBW);
    Limits.VNA.maxPoints = std::min(Limits.VNA.maxPoints, info.Limits.VNA.maxPoints);

    Limits.Generator.ports += info.Limits.Generator.ports;
    Limits.Generator.minFreq = std::max(Limits.Generator.minFreq, info.Limits.Generator.minFreq);
    Limits.Generator.maxFreq = std::min(Limits.Generator.maxFreq, info.Limits.Generator.maxFreq);
    Limits.Generator.mindBm = std::max(Limits.Generator.mindBm, info.Limits.Generator.mindBm);
    Limits.Generator.maxdBm = std::min(Limits.Generator.maxdBm, info.Limits.Generator.maxdBm);

    Limits.SA.ports += info.Limits.SA.ports;
    Limits.SA.minFreq = std::max(Limits.SA.minFreq, info.Limits.SA.minFreq);
    Limits.SA.maxFreq = std::min(Limits.SA.maxFreq, info.Limits.SA.maxFreq);
    Limits.SA.mindBm = std::max(Limits.SA.mindBm, info.Limits.SA.mindBm);
    Limits.SA.maxdBm = std::min(Limits.SA.maxdBm, info.Limits.SA.maxdBm);
    Limits.SA.minRBW = std::max(Limits.SA.minRBW, info.Limits.SA.minRBW);
    Limits.SA.maxRBW = std::min(Limits.SA.maxRBW, info.Limits.SA.maxRBW);

    std::set<Feature> intersectFeatures;
    std::set_intersection(supportedFeatures.begin(), supportedFeatures.end(), info.supportedFeatures.begin(), info.supportedFeatures.end(),
                          std::inserter(intersectFeatures, intersectFeatures.begin()));
    supportedFeatures = intersectFeatures;
}
