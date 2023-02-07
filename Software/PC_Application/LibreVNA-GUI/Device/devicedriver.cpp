#include "devicedriver.h"

#include "LibreVNA/librevnatcpdriver.h"
#include "LibreVNA/librevnausbdriver.h"
#include "LibreVNA/Compound/compounddriver.h"
#include "SSA3000X/ssa3000xdriver.h"

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

Sparam DeviceDriver::VNAMeasurement::toSparam(int port1, int port2) const
{
    Sparam S;
    S.m11 = measurements.at("S"+QString::number(port1)+QString::number(port1));
    S.m12 = measurements.at("S"+QString::number(port1)+QString::number(port2));
    S.m21 = measurements.at("S"+QString::number(port2)+QString::number(port1));
    S.m22 = measurements.at("S"+QString::number(port2)+QString::number(port2));
    return S;
}

void DeviceDriver::VNAMeasurement::fromSparam(Sparam S, int port1, int port2)
{
    QString s11 = "S"+QString::number(port1)+QString::number(port1);
    QString s12 = "S"+QString::number(port1)+QString::number(port2);
    QString s21 = "S"+QString::number(port2)+QString::number(port1);
    QString s22 = "S"+QString::number(port2)+QString::number(port2);
    if(measurements.count(s11)) {
        measurements[s11] = S.m11;
    }
    if(measurements.count(s12)) {
        measurements[s12] = S.m12;
    }
    if(measurements.count(s21)) {
        measurements[s21] = S.m21;
    }
    if(measurements.count(s22)) {
        measurements[s22] = S.m22;
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
    Limits.VNA.maxFreq = 6000000000;
    Limits.VNA.mindBm = -100;
    Limits.VNA.maxdBm = 30;
    Limits.VNA.minIFBW = 1;
    Limits.VNA.maxIFBW = 1000000;
    Limits.VNA.maxPoints = 65535;

    Limits.Generator.ports = 2;
    Limits.Generator.minFreq = 0;
    Limits.Generator.maxFreq = 6000000000;
    Limits.Generator.mindBm = -100;
    Limits.Generator.maxdBm = 30;

    Limits.SA.ports = 2;
    Limits.SA.minFreq = 0;
    Limits.SA.maxFreq = 6000000000;
    Limits.SA.mindBm = -100;
    Limits.SA.maxdBm = 30;
    Limits.SA.minRBW = 1;
    Limits.SA.maxRBW = 1000000;
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
