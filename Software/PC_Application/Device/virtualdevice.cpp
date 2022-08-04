#include "virtualdevice.h"

#include "preferences.h"
#include "../VNA_embedded/Application/Communication/Protocol.hpp"

static VirtualDevice *connected = nullptr;

using namespace std;

class Reference
{
public:
    enum class TypeIn {
        Internal,
        External,
        Auto,
        None
    };
    enum class OutFreq {
        MHZ10,
        MHZ100,
        Off,
        None
    };

    static QString OutFreqToLabel(Reference::OutFreq t)
    {
        switch(t) {
        case OutFreq::MHZ10: return "10 MHz";
        case OutFreq::MHZ100: return "100 MHz";
        case OutFreq::Off: return "Off";
        default: return "Invalid";
        }
    }

    static QString OutFreqToKey(Reference::OutFreq f)
    {
        switch(f) {
        case OutFreq::MHZ10: return "10 MHz";
        case OutFreq::MHZ100: return "100 MHz";
        case OutFreq::Off: return "Off";
        default: return "Invalid";
        }
    }

    static Reference::OutFreq KeyToOutFreq(QString key)
    {
        for (auto r: Reference::getOutFrequencies()) {
            if(OutFreqToKey(r) == key|| OutFreqToLabel(r) == key) {
                return r;
            }
        }
        // not found
        return Reference::OutFreq::None;
    }


    static QString TypeToLabel(TypeIn t)
    {
        switch(t) {
        case TypeIn::Internal: return "Internal";
        case TypeIn::External: return "External";
        case TypeIn::Auto: return "Auto";
        default: return "Invalid";
        }
    }

    static const QString TypeToKey(TypeIn t)
    {
        switch(t) {
        case TypeIn::Internal: return "Int";
        case TypeIn::External: return "Ext";
        case TypeIn::Auto: return "Auto";
        default: return "Invalid";
        }
    }

    static TypeIn KeyToType(QString key)
    {
        for (auto r: Reference::getReferencesIn()) {
            if(TypeToKey(r) == key || TypeToLabel(r) == key) {
                return r;
            }
        }
        // not found
        return TypeIn::None;
    }

    static std::vector<Reference::TypeIn> getReferencesIn()
    {
        return {TypeIn::Internal, TypeIn::External, TypeIn::Auto};
    }

    static std::vector<Reference::OutFreq> getOutFrequencies()
    {
        return {OutFreq::Off, OutFreq::MHZ10, OutFreq::MHZ100};
    }
};

static constexpr VirtualDevice::Info defaultInfo = {
    .ProtocolVersion = Protocol::Version,
    .FW_major = 0,
    .FW_minor = 0,
    .FW_patch = 0,
    .hardware_version = 1,
    .HW_Revision = '0',
    .ports = 2,
    .supportsVNAmode = true,
    .supportsSAmode = true,
    .supportsSGmode = true,
    .Limits = {
        .minFreq = 0,
        .maxFreq = 6000000000,
        .maxFreqHarmonic = 18000000000,
        .minIFBW = 10,
        .maxIFBW = 1000000,
        .maxPoints = 10000,
        .mindBm = -100,
        .maxdBm = 100,
        .minRBW = 1,
        .maxRBW = 1000000,
    }
};

static const VirtualDevice::Status defaultStatus = {
    .statusString = "",
    .overload = false,
    .unlocked = false,
    .unlevel = false,
};

VirtualDevice::VirtualDevice(QString serial)
    : QObject(),
      info{}
{
    isCompound = false;
    zerospan = false;
    auto dev = new Device(serial);
    devices.push_back(dev);

    if(!isCompoundDevice()) {
        // just acting as a wrapper for device, pass on signals
        connect(dev, &Device::ConnectionLost, this, &VirtualDevice::ConnectionLost);
        connect(dev, &Device::DeviceInfoUpdated, [&](){
            auto i = dev->Info();
            info.ProtocolVersion = i.ProtocolVersion;
            info.FW_major = i.FW_major;
            info.FW_minor = i.FW_minor;
            info.FW_patch = i.FW_patch;
            info.hardware_version = i.hardware_version;
            info.HW_Revision = i.HW_Revision;
            info.ports = 2;
            info.supportsVNAmode = true;
            info.supportsSAmode = true;
            info.supportsSGmode = true;
            info.Limits.minFreq = i.limits_minFreq;
            info.Limits.maxFreq = i.limits_maxFreq;
            info.Limits.maxFreqHarmonic = i.limits_maxFreqHarmonic;
            info.Limits.minIFBW = i.limits_minIFBW;
            info.Limits.maxIFBW = i.limits_minIFBW;
            info.Limits.maxPoints = i.limits_maxPoints;
            info.Limits.mindBm = (double) i.limits_cdbm_min / 100;
            info.Limits.maxdBm = (double) i.limits_cdbm_max / 100;
            info.Limits.minRBW = i.limits_minRBW;
            info.Limits.maxRBW = i.limits_minRBW;
            emit InfoUpdated();
        });
        connect(dev, &Device::LogLineReceived, this, &VirtualDevice::LogLineReceived);
        connect(dev, &Device::DeviceStatusUpdated, [&](){
            status.statusString = dev->getLastDeviceInfoString();
            status.overload = dev->StatusV1().ADC_overload;
            status.unlevel = dev->StatusV1().unlevel;
            status.unlocked = !dev->StatusV1().LO1_locked || !dev->StatusV1().source_locked;
            emit StatusUpdated(status);
        });
        connect(dev, &Device::NeedsFirmwareUpdate, this, &VirtualDevice::NeedsFirmwareUpdate);

        connect(dev, &Device::SpectrumResultReceived, [&](Protocol::SpectrumAnalyzerResult res){
            SAMeasurement m;
            m.pointNum = res.pointNum;
            if(zerospan) {
                m.us = res.us;
            } else {
                m.frequency = res.frequency;
            }
            m.measurements["PORT1"] = res.port1;
            m.measurements["PORT2"] = res.port2;
            emit SAmeasurementReceived(m);
        });
        connect(dev, &Device::DatapointReceived, [&](Protocol::Datapoint res){
            VNAMeasurement m;
            m.pointNum = res.pointNum;
            if(zerospan) {
                m.us = res.us;
            } else {
                m.frequency = res.frequency;
                m.dBm = (double) res.cdbm / 100;
            }
            m.measurements["S11"] = complex<double>(res.real_S11, res.imag_S11);
            m.measurements["S21"] = complex<double>(res.real_S21, res.imag_S21);
            m.measurements["S12"] = complex<double>(res.real_S12, res.imag_S12);
            m.measurements["S22"] = complex<double>(res.real_S22, res.imag_S22);
            emit VNAmeasurementReceived(m);
        });
    } else {
        // TODO
    }
    connected = this;
}

VirtualDevice::~VirtualDevice()
{
    connected = nullptr;
    for(auto dev : devices) {
        delete dev;
    }
}

bool VirtualDevice::isCompoundDevice() const
{
    return isCompound;
}

Device *VirtualDevice::getDevice()
{
    if(isCompound || devices.size() < 1) {
        return nullptr;
    } else {
        return devices[0];
    }
}

std::vector<Device *> VirtualDevice::getDevices()
{
    return devices;
}

const VirtualDevice::Info &VirtualDevice::getInfo() const
{
    return info;
}

const VirtualDevice::Info &VirtualDevice::getInfo(VirtualDevice *vdev)
{
    if(vdev) {
        return vdev->info;
    } else {
        return defaultInfo;
    }
}

const VirtualDevice::Status &VirtualDevice::getStatus() const
{
    return status;
}

const VirtualDevice::Status &VirtualDevice::getStatus(VirtualDevice *vdev)
{
    if(vdev) {
        return vdev->status;
    } else {
        return defaultStatus;
    }
}

QStringList VirtualDevice::availableVNAMeasurements()
{
    QStringList ret;
    for(int i=1;i<info.ports;i++) {
        for(int j=1;j<info.ports;i++) {
            ret.push_back("S"+QString::number(i)+QString::number(j));
        }
    }
    return ret;
}

bool VirtualDevice::setVNA(const VirtualDevice::VNASettings &s, std::function<void (bool)> cb)
{
    if(!info.supportsVNAmode) {
        return false;
    }
    zerospan = (s.freqStart == s.freqStop) && (s.dBmStart == s.dBmStop);
    auto pref = Preferences::getInstance();
    if(!isCompoundDevice()) {
        Protocol::SweepSettings sd;
        sd.f_start = s.freqStart;
        sd.f_stop = s.freqStop;
        sd.points = s.points;
        sd.if_bandwidth = s.IFBW;
        sd.cdbm_excitation_start = s.dBmStart * 100;
        sd.cdbm_excitation_stop = s.dBmStop * 100;
        sd.excitePort1 = find(s.excitedPorts.begin(), s.excitedPorts.end(), 1) != s.excitedPorts.end() ? 1 : 0;
        sd.excitePort2 = find(s.excitedPorts.begin(), s.excitedPorts.end(), 2) != s.excitedPorts.end() ? 1 : 0;
        sd.suppressPeaks = pref.Acquisition.suppressPeaks ? 1 : 0;
        sd.fixedPowerSetting = pref.Acquisition.adjustPowerLevel ? 0 : 1;
        sd.logSweep = s.logSweep ? 1 : 0;
        return devices[0]->Configure(sd, [=](Device::TransmissionResult r){
            if(cb) {
                cb(r == Device::TransmissionResult::Ack);
            }
        });
    } else {
        // TODO
        return false;
}
}

QString VirtualDevice::serial()
{
    if(!isCompoundDevice()) {
        return devices[0]->serial();
    } else {
        // TODO
        return "";
    }
}

QStringList VirtualDevice::availableSAMeasurements()
{
    QStringList ret;
    for(int i=1;i<info.ports;i++) {
        ret.push_back("PORT"+QString::number(i));
    }
    return ret;
}

bool VirtualDevice::setSA(const VirtualDevice::SASettings &s, std::function<void (bool)> cb)
{
    if(!info.supportsSAmode) {
        return false;
    }
    zerospan = s.freqStart == s.freqStop;
    auto pref = Preferences::getInstance();
    if(!isCompoundDevice()) {
        Protocol::SpectrumAnalyzerSettings sd;
        sd.f_start = s.freqStart;
        sd.f_stop = s.freqStop;
        sd.pointNum = s.points;
        sd.RBW = s.RBW;
        sd.WindowType = (int) s.window;
        sd.SignalID = s.signalID ? 1 : 0;
        sd.Detector = (int) s.detector;
        sd.UseDFT = 0;
        if(!s.trackingGenerator && pref.Acquisition.useDFTinSAmode && s.RBW <= pref.Acquisition.RBWLimitForDFT) {
            sd.UseDFT = 1;
        }
        sd.applyReceiverCorrection = 1;
        sd.trackingGenerator = s.trackingGenerator ? 1 : 0;
        sd.applySourceCorrection = 1;
        sd.trackingGeneratorPort = s.trackingPort;
        sd.trackingGeneratorOffset = s.trackingOffset;
        sd.trackingPower = s.trackingPower;
        return devices[0]->Configure(sd, [=](Device::TransmissionResult r){
            if(cb) {
                cb(r == Device::TransmissionResult::Ack);
            }
        });
    } else {
        // TODO
        return false;
    }
}

QStringList VirtualDevice::availableSGPorts()
{
    QStringList ret;
    for(int i=1;i<info.ports;i++) {
        ret.push_back("PORT"+QString::number(i));
    }
    return ret;
}

bool VirtualDevice::setSG(const SGSettings &s)
{
    if(!info.supportsSGmode) {
        return false;
    }
    auto pref = Preferences::getInstance();
    if(!isCompoundDevice()) {
        Protocol::PacketInfo packet;
        packet.type = Protocol::PacketType::Generator;
        Protocol::GeneratorSettings &sd = packet.generator;
        sd.frequency = s.freq;
        sd.cdbm_level = s.dBm * 100;
        sd.activePort = s.port;
        sd.applyAmplitudeCorrection = 1;
        return devices[0]->SendPacket(packet);
    } else {
        // TODO
        return false;
    }
}

bool VirtualDevice::setIdle(std::function<void (bool)> cb)
{
    results.clear();
    for(auto dev : devices) {
        dev->SetIdle([&](Device::TransmissionResult r){
            if(cb) {
                results[dev] = r;
                if(results.size() == devices.size()) {
                    // got all responses
                    bool success = true;
                    for(auto res : results) {
                        if(res.second != Device::TransmissionResult::Ack) {
                            success = false;
                            break;
                        }
                    }
                    cb(success);
                }
            }
        });
    }
}

QStringList VirtualDevice::availableExtRefInSettings()
{
    QStringList ret;
    for(auto r : Reference::getReferencesIn()) {
        ret.push_back(Reference::TypeToLabel(r));
    }
    return ret;
}

QStringList VirtualDevice::availableExtRefOutSettings()
{
    QStringList ret;
    for(auto r : Reference::getOutFrequencies()) {
        ret.push_back(Reference::OutFreqToLabel(r));
    }
    return ret;
}

bool VirtualDevice::setExtRef(QString option_in, QString option_out)
{
    if(!info.supportsExtRef) {
        return false;
    }
    auto refIn = Reference::KeyToType(option_in);
    if(refIn == Reference::TypeIn::None) {
        refIn = Reference::TypeIn::Internal;
    }
    auto refOut = Reference::KeyToOutFreq(option_out);
    if(refOut == Reference::OutFreq::None) {
        refOut = Reference::OutFreq::Off;
    }

    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::Reference;
    switch(refIn) {
    case Reference::TypeIn::Internal:
        p.reference.UseExternalRef = 0;
        p.reference.AutomaticSwitch = 0;
        break;
    case Reference::TypeIn::Auto:
        p.reference.UseExternalRef = 0;
        p.reference.AutomaticSwitch = 1;
        break;
    case Reference::TypeIn::External:
        p.reference.UseExternalRef = 1;
        p.reference.AutomaticSwitch = 0;
        break;
    }
    switch(refOut) {
    case Reference::OutFreq::Off: p.reference.ExtRefOuputFreq = 0; break;
    case Reference::OutFreq::MHZ10: p.reference.ExtRefOuputFreq = 10000000; break;
    case Reference::OutFreq::MHZ100: p.reference.ExtRefOuputFreq = 100000000; break;
    }

    bool success = true;
    for(auto dev : devices) {
        success &= dev->SendPacket(p);
    }
    return success;
}

std::set<QString> VirtualDevice::GetDevices()
{
    auto ret = Device::GetDevices();
    // TODO check if compound devices are configured and add them if all sub-devices are available
    return ret;
}

VirtualDevice *VirtualDevice::getConnected()
{
    return connected;
}
