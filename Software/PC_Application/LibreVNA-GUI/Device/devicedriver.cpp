#include "devicedriver.h"

DeviceDriver *DeviceDriver::activeDriver = nullptr;

DeviceDriver::~DeviceDriver()
{
    for(auto a : specificActions) {
        delete a;
    }
}

bool DeviceDriver::connectDevice(QString serial)
{
    if(activeDriver && activeDriver != this) {
        activeDriver->disconnect();
    }
    if(connectTo(serial)) {
        activeDriver = this;
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
