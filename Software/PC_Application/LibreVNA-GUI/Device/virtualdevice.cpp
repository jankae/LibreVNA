#include "virtualdevice.h"

#include "preferences.h"
#include "CustomWidgets/informationbox.h"
#include "../../VNA_embedded/Application/Communication/Protocol.hpp"

#include <cmath>

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

VirtualDevice::VirtualDevice(QString serial)
    : QObject(),
      info{},
      status{}
{
    cdev = nullptr;
    cdev = nullptr;
    zerospan = false;

    // Check if this is a compound device
    auto pref = Preferences::getInstance();
    for(auto cd : pref.compoundDevices) {
        if(cd->name == serial) {
            // connect request to this compound device
            cdev = cd;
            break;
        }
    }

    if(!isCompoundDevice()) {      
        // just acting as a wrapper for device, pass on signals
        auto dev = new Device(serial);
        devices.push_back(dev);
        connect(dev, &Device::ConnectionLost, this, &VirtualDevice::ConnectionLost);
        connect(dev, &Device::DeviceInfoUpdated, [=](){
            info = Info(devices[0]);
            emit InfoUpdated();
        });
        connect(dev, &Device::LogLineReceived, this, &VirtualDevice::LogLineReceived);
        connect(dev, &Device::DeviceStatusUpdated, [=](){
            status = Status(devices[0]);
            emit StatusUpdated(status);
        });
        connect(dev, &Device::NeedsFirmwareUpdate, this, &VirtualDevice::NeedsFirmwareUpdate);
        connect(dev, &Device::SpectrumResultReceived, this, &VirtualDevice::singleSpectrumResultReceived);
        connect(dev, &Device::DatapointReceived, this, &VirtualDevice::singleDatapointReceived);
    } else {
        // Connect to the actual devices
        for(auto devSerial : cdev->deviceSerials) {
            auto dev = new Device(devSerial);
            devices.push_back(dev);
            // Create device connections
            connect(dev, &Device::ConnectionLost, this, &VirtualDevice::ConnectionLost);
            connect(dev, &Device::NeedsFirmwareUpdate, this, &VirtualDevice::NeedsFirmwareUpdate);
            connect(dev, &Device::LogLineReceived, [=](QString line){
                emit LogLineReceived(line.prepend(dev->serial()+": "));
            });
            connect(dev, &Device::DeviceInfoUpdated, this, &VirtualDevice::compoundInfoUpdated, Qt::QueuedConnection);
            connect(dev, &Device::DeviceStatusUpdated, this, &VirtualDevice::compoundStatusUpdated, Qt::QueuedConnection);
            connect(dev, &Device::DatapointReceived, this, &VirtualDevice::compoundDatapointReceivecd, Qt::QueuedConnection);
            connect(dev, &Device::SpectrumResultReceived, this, &VirtualDevice::compoundSpectrumResultReceived, Qt::QueuedConnection);
        }
        if(cdev->sync == CompoundDevice::Synchronization::USB) {
            // create trigger connections for USB synchronization
            for(int i=0;i<devices.size() - 1;i++) {
                connect(devices[i], &Device::TriggerReceived, devices[i+1], &Device::SetTrigger, Qt::QueuedConnection);
            }
            connect(devices.back(), &Device::TriggerReceived, devices.front(), &Device::SetTrigger, Qt::QueuedConnection);
        }

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

void VirtualDevice::RegisterTypes()
{
    qRegisterMetaType<VirtualDevice::Status>("Status");
    qRegisterMetaType<VirtualDevice::VNAMeasurement>("VNAMeasurement");
    qRegisterMetaType<VirtualDevice::SAMeasurement>("SAMeasurement");
}

bool VirtualDevice::isCompoundDevice() const
{
    return cdev != nullptr;
}

Device *VirtualDevice::getDevice()
{
    if(isCompoundDevice() || devices.size() < 1) {
        return nullptr;
    } else {
        return devices[0];
    }
}

CompoundDevice *VirtualDevice::getCompoundDevice()
{
    return cdev;
}

std::vector<Device *> VirtualDevice::getDevices()
{
    return devices;
}

const VirtualDevice::Info &VirtualDevice::getInfo() const
{
    return info;
}

VirtualDevice::Info VirtualDevice::getInfo(VirtualDevice *vdev)
{
    if(vdev) {
        return vdev->info;
    } else {
        return Info();
    }
}

const VirtualDevice::Status &VirtualDevice::getStatus() const
{
    return status;
}

VirtualDevice::Status VirtualDevice::getStatus(VirtualDevice *vdev)
{
    if(vdev) {
        return vdev->status;
    } else {
        return Status();
    }
}

QStringList VirtualDevice::availableVNAMeasurements()
{
    QStringList ret;
    for(int i=1;i<=info.ports;i++) {
        for(int j=1;j<=info.ports;j++) {
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
    if(s.excitedPorts.size() == 0) {
        return setIdle(cb);
    }

    // create port->stage mapping
    portStageMapping.clear();
    for(unsigned int i=0;i<s.excitedPorts.size();i++) {
        portStageMapping[s.excitedPorts[i]] = i;
    }

    auto pref = Preferences::getInstance();
    Protocol::SweepSettings sd;
    sd.f_start = s.freqStart;
    sd.f_stop = s.freqStop;
    sd.points = s.points;
    sd.if_bandwidth = s.IFBW;
    sd.cdbm_excitation_start = s.dBmStart * 100;
    sd.cdbm_excitation_stop = s.dBmStop * 100;
    sd.stages = s.excitedPorts.size() - 1;
    sd.suppressPeaks = pref.Acquisition.suppressPeaks ? 1 : 0;
    sd.fixedPowerSetting = pref.Acquisition.adjustPowerLevel || s.dBmStart != s.dBmStop ? 0 : 1;
    sd.logSweep = s.logSweep ? 1 : 0;

    zerospan = (s.freqStart == s.freqStop) && (s.dBmStart == s.dBmStop);
    if(!isCompoundDevice()) {
        sd.port1Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 0) - s.excitedPorts.begin();
        sd.port2Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 1) - s.excitedPorts.begin();
        sd.syncMode = 0;
        sd.syncMaster = 0;
        return devices[0]->Configure(sd, [=](Device::TransmissionResult r){
            if(cb) {
                cb(r == Device::TransmissionResult::Ack);
            }
        });
    } else {
        // set the synchronization mode
        switch(cdev->sync) {
            case CompoundDevice::Synchronization::USB: sd.syncMode = 1; break;
            case CompoundDevice::Synchronization::ExtRef: sd.syncMode = 2; break;
            case CompoundDevice::Synchronization::Trigger: sd.syncMode = 3; break;
        }
        // create vector of currently used stimulus ports
        vector<CompoundDevice::PortMapping> activeMapping;
        for(auto p : s.excitedPorts) {
            activeMapping.push_back(cdev->portMapping[p]);
        }
        // Configure the devices
        results.clear();
        bool success = true;
        for(unsigned int i=0;i<devices.size();i++) {
            sd.port1Stage = CompoundDevice::PortMapping::findActiveStage(activeMapping, i, 0);
            sd.port2Stage = CompoundDevice::PortMapping::findActiveStage(activeMapping, i, 1);
            sd.syncMaster = i == 0 ? 1 : 0;
            success &= devices[i]->Configure(sd, [=](Device::TransmissionResult r){
                if(cb) {
                    results[devices[i]] = r;
                    checkIfAllTransmissionsComplete(cb);
                }
            });
        }
        return success;
    }
}

QString VirtualDevice::serial()
{
    if(!isCompoundDevice()) {
        return devices[0]->serial();
    } else {
        return cdev->name;
    }
}

QStringList VirtualDevice::availableSAMeasurements()
{
    QStringList ret;
    for(int i=1;i<=info.ports;i++) {
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
    sd.trackingGeneratorOffset = s.trackingOffset;
    sd.trackingPower = s.trackingPower;

    if(!isCompoundDevice()) {
        sd.trackingGenerator = s.trackingGenerator ? 1 : 0;
        sd.trackingGeneratorPort = s.trackingPort;
        sd.syncMode = 0;
        sd.syncMaster = 0;
        return devices[0]->Configure(sd, [=](Device::TransmissionResult r){
            if(cb) {
                cb(r == Device::TransmissionResult::Ack);
            }
        });
    } else {
        // set the synchronization mode
        switch(cdev->sync) {
            case CompoundDevice::Synchronization::USB: sd.syncMode = 1; break;
            case CompoundDevice::Synchronization::ExtRef: sd.syncMode = 2; break;
            case CompoundDevice::Synchronization::Trigger: sd.syncMode = 3; break;
        }
        // Configure the devices
        results.clear();
        bool success = true;
        for(unsigned int i=0;i<devices.size();i++) {
            sd.trackingGenerator = 0;
            sd.trackingGeneratorPort = 0;
            if(s.trackingGenerator) {
                if(CompoundDevice::PortMapping::findActiveStage(cdev->portMapping, i, 0) == s.trackingPort) {
                    sd.trackingGenerator = 1;
                    sd.trackingGeneratorPort = 0;
                } else if(CompoundDevice::PortMapping::findActiveStage(cdev->portMapping, i, 1) == s.trackingPort) {
                    sd.trackingGenerator = 1;
                    sd.trackingGeneratorPort = 1;
                }
            }
            sd.syncMaster = i == 0 ? 1 : 0;
            success &= devices[i]->Configure(sd, [=](Device::TransmissionResult r){
                if(cb) {
                    results[devices[i]] = r;
                    checkIfAllTransmissionsComplete(cb);
                }
            });
        }
        return success;
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
    Protocol::PacketInfo packet;
    packet.type = Protocol::PacketType::Generator;
    Protocol::GeneratorSettings &sd = packet.generator;
    sd.frequency = s.freq;
    sd.cdbm_level = s.dBm * 100;
    sd.applyAmplitudeCorrection = 1;

    if(!isCompoundDevice()) {
        sd.activePort = s.port;
        return devices[0]->SendPacket(packet);
    } else {
        // configure all devices
        bool success = true;
        for(unsigned int i=0;i<devices.size();i++) {
            sd.activePort = 0;
            if(s.port > 0) {
                if(cdev->portMapping[s.port-1].device == i) {
                    // this device has the active port
                    sd.activePort = cdev->portMapping[s.port-1].port+1;
                }
            }
            success &= devices[i]->SendPacket(packet);
        }
        return success;
    }
}

bool VirtualDevice::setIdle(std::function<void (bool)> cb)
{
    auto success = true;
    results.clear();
    for(auto dev : devices) {
        success &= dev->SetIdle([=](Device::TransmissionResult r){
            if(cb) {
                results[dev] = r;
                checkIfAllTransmissionsComplete(cb);
            }
        });
    }
    return success;
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
    case Reference::TypeIn::None:
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
    case Reference::OutFreq::None:
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

std::set<QString> VirtualDevice::GetAvailableVirtualDevices()
{
    auto pref = Preferences::getInstance();
    auto ret = Device::GetDevices();
    // Add compound devices as well
    for(auto vdev : pref.compoundDevices) {
        // check if all serial number required for this compound device are available
        bool serialMissing = false;
        for(auto s : vdev->deviceSerials) {
            if(ret.count(s) == 0) {
                serialMissing = true;
                break;
            }
        }
        if(!serialMissing) {
            // this compound device is available
            ret.insert(vdev->name);
        }
    }
    return ret;
}

VirtualDevice *VirtualDevice::getConnected()
{
    return connected;
}

void VirtualDevice::singleDatapointReceived(Device *dev, Protocol::VNADatapoint<32> *res)
{
    Q_UNUSED(dev)
    VNAMeasurement m;
    m.pointNum = res->pointNum;
    m.Z0 = 50.0;
    if(zerospan) {
        m.us = res->us;
    } else {
        m.frequency = res->frequency;
        m.dBm = (double) res->cdBm / 100;
    }
    for(auto map : portStageMapping) {
        // map.first is the port (starts at zero)
        // map.second is the stage at which this port had the stimulus (starts at zero)
        complex<double> ref = res->getValue(map.second, map.first, true);
        for(int i=0;i<2;i++) {
            complex<double> input = res->getValue(map.second, i, false);
            if(!std::isnan(ref.real()) && !std::isnan(input.real())) {
                // got both required measurements
                QString name = "S"+QString::number(i+1)+QString::number(map.first+1);
                m.measurements[name] = input / ref;
            }
        }
    }
    delete res;
    emit VNAmeasurementReceived(m);
}

void VirtualDevice::compoundDatapointReceivecd(Device *dev, Protocol::VNADatapoint<32> *data)
{
    if(!compoundVNABuffer.count(data->pointNum)) {
        compoundVNABuffer[data->pointNum] = std::map<Device*, Protocol::VNADatapoint<32>*>();
    }
    auto &buf = compoundVNABuffer[data->pointNum];
    buf[dev] = data;
    if(buf.size() == devices.size()) {
        // Got datapoints from all devices, can create merged VNA result
        VNAMeasurement m;
        m.pointNum = data->pointNum;
        m.Z0 = 50.0;
        if(zerospan) {
            m.us = data->us;
        } else {
            m.frequency = data->frequency;
            m.dBm = (double) data->cdBm / 100;
        }
        // assemble data
        for(auto map : portStageMapping) {
            // map.first is the port (starts at zero)
            // map.second is the stage at which this port had the stimulus (starts at zero)

            // figure out which device had the stimulus for the port...
            auto stimulusDev = devices[cdev->portMapping[map.first].device];
            // ...and which device port was used for the stimulus...
            auto stimulusDevPort = cdev->portMapping[map.first].port;
            // ...grab the reference receiver data
            complex<double> ref = buf[stimulusDev]->getValue(map.second, stimulusDevPort, true);

            // for all ports of the compound device...
            for(unsigned int i=0;i<cdev->portMapping.size();i++) {
                // ...figure out which physical device and port was used for this input...
                auto inputDevice = devices[cdev->portMapping[i].device];
                // ...and grab the data
                auto inputPort = cdev->portMapping[i].port;
                complex<double> input = buf[inputDevice]->getValue(map.second, inputPort, false);
                if(!std::isnan(ref.real()) && !std::isnan(input.real())) {
                    // got both required measurements
                    QString name = "S"+QString::number(i+1)+QString::number(map.first+1);
                    auto S = input / ref;
                    if(inputDevice != stimulusDev) {
                        // can't use phase information when measuring across devices
                        S = abs(S);
                    }
                    m.measurements[name] = S;
                }
            }
        }

        emit VNAmeasurementReceived(m);

        // Clear this and all (incomplete) older datapoint buffers
        int pointNum = data->pointNum;
        auto it = compoundVNABuffer.begin();
        while(it != compoundVNABuffer.end()) {
            if(it->first <= pointNum) {
                for(auto d : it->second) {
                    delete d.second;
                }
                it = compoundVNABuffer.erase(it);
            } else {
                it++;
            }
        }
    }
}

void VirtualDevice::singleSpectrumResultReceived(Device *dev, Protocol::SpectrumAnalyzerResult res)
{
    Q_UNUSED(dev)
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
}

void VirtualDevice::compoundSpectrumResultReceived(Device *dev, Protocol::SpectrumAnalyzerResult res)
{
    if(!compoundSABuffer.count(res.pointNum)) {
        compoundSABuffer[res.pointNum] = std::map<Device*, Protocol::SpectrumAnalyzerResult>();
    }
    auto &buf = compoundSABuffer[res.pointNum];
    buf[dev] = res;
    if(buf.size() == devices.size()) {
        // Got datapoints from all devices, can create merged VNA result
        SAMeasurement m;
        m.pointNum = res.pointNum;
        if(zerospan) {
            m.us = res.us;
        } else {
            m.frequency = res.frequency;
        }
        // assemble data
        for(unsigned int port=0;port<cdev->portMapping.size();port++) {
            auto device = devices[cdev->portMapping[port].device];
            auto devicePort = cdev->portMapping[port].port;

            QString name = "PORT"+QString::number(port+1);
            if(devicePort == 0) {
                m.measurements[name] = buf[device].port1;
            } else {
                m.measurements[name] = buf[device].port2;
            }
        }

        emit SAmeasurementReceived(m);

        // Clear this and all (incomplete) older datapoint buffers
        auto it = compoundSABuffer.begin();
        while(it != compoundSABuffer.end()) {
            if(it->first <= res.pointNum) {
                it = compoundSABuffer.erase(it);
            } else {
                it++;
            }
        }
    }
}

void VirtualDevice::compoundInfoUpdated(Device *dev)
{
    compoundInfoBuffer[dev] = dev->Info();
    if(compoundInfoBuffer.size() == devices.size()) {
        // got information of all devices
        info = Info(devices[0]);
        for(int i=1;i<devices.size();i++) {
            try {
                info.subset(Info(devices[i]));
            } catch (exception &e) {
                InformationBox::ShowError("Failed to get device information", e.what());
                emit ConnectionLost();
                return;
            }
        }
        if(cdev->sync == CompoundDevice::Synchronization::ExtRef) {
            // can't use the external reference if it is used for synchronization
            info.supportsExtRef = false;
        }
        info.ports = cdev->portMapping.size();
        emit InfoUpdated();
    }
}

void VirtualDevice::compoundStatusUpdated(Device *dev)
{
    compoundStatusBuffer[dev] = dev->StatusV1();
    if(compoundStatusBuffer.size() == devices.size()) {
        // got status of all devices
        status = Status(devices[0]);
        for(int i=1;i<devices.size();i++) {
            status.merge(Status(devices[i]));
        }
        emit StatusUpdated(status);
    }
}

void VirtualDevice::checkIfAllTransmissionsComplete(std::function<void (bool)> cb)
{
    if(results.size() == devices.size()) {
        // got all responses
        bool success = true;
        for(auto res : results) {
            if(res.second != Device::TransmissionResult::Ack) {
                success = false;
                break;
            }
        }
        if(cb) {
            cb(success);
        }
    }
}

Sparam VirtualDevice::VNAMeasurement::toSparam(int port1, int port2) const
{
    Sparam S;
    S.m11 = measurements.at("S"+QString::number(port1)+QString::number(port1));
    S.m12 = measurements.at("S"+QString::number(port1)+QString::number(port2));
    S.m21 = measurements.at("S"+QString::number(port2)+QString::number(port1));
    S.m22 = measurements.at("S"+QString::number(port2)+QString::number(port2));
    return S;
}

void VirtualDevice::VNAMeasurement::fromSparam(Sparam S, int port1, int port2)
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

VirtualDevice::VNAMeasurement VirtualDevice::VNAMeasurement::interpolateTo(const VirtualDevice::VNAMeasurement &to, double a)
{
    VNAMeasurement ret;
    ret.frequency = frequency * (1.0 - a) + to.frequency * a;
    ret.dBm = dBm * (1.0 - a) + to.dBm * a;
    ret.Z0 = Z0 * (1.0 - a) + to.Z0 * a;
    for(auto m : measurements) {
        if(to.measurements.count(m.first) == 0) {
            throw runtime_error("Nothing to interpolate to, expected measurement +\""+m.first.toStdString()+"\"");
        }
        ret.measurements[m.first] = measurements[m.first] * (1.0 - a) + to.measurements.at(m.first) * a;
    }
    return ret;
}

VirtualDevice::Info::Info()
{
    ProtocolVersion = Protocol::Version;
    FW_major = 0;
    FW_minor = 0;
    FW_patch = 0;
    hardware_version = 1;
    HW_Revision = '0';
    ports = 2;
    supportsVNAmode = true;
    supportsSAmode = true;
    supportsSGmode = true;
    supportsExtRef = true;
    Limits = {
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
    };
}

VirtualDevice::Info::Info(Device *dev)
{
    auto info = dev->Info();
    ProtocolVersion = info.ProtocolVersion;
    FW_major = info.FW_major;
    FW_minor = info.FW_minor;
    FW_patch = info.FW_patch;
    hardware_version = info.hardware_version;
    HW_Revision = info.HW_Revision;
    ports = 2;
    supportsVNAmode = true;
    supportsSAmode = true;
    supportsSGmode = true;
    supportsExtRef = true;
    Limits.minFreq = info.limits_minFreq;
    Limits.maxFreq = info.limits_maxFreq;
    Limits.maxFreqHarmonic = info.limits_maxFreqHarmonic;
    Limits.minIFBW = info.limits_minIFBW;
    Limits.maxIFBW = info.limits_maxIFBW;
    Limits.maxPoints = info.limits_maxPoints;
    Limits.mindBm = (double) info.limits_cdbm_min / 100;
    Limits.maxdBm = (double) info.limits_cdbm_max / 100;
    Limits.minRBW = info.limits_minRBW;
    Limits.maxRBW = info.limits_maxRBW;
}

void VirtualDevice::Info::subset(const VirtualDevice::Info &merge)
{
    if((merge.ProtocolVersion != ProtocolVersion)
            || (merge.FW_major != FW_major)
            || (merge.FW_minor != FW_minor)
            || (merge.FW_patch != FW_patch)) {
        throw runtime_error("Incompatible device, unable to create compound device. All devices must run the same firmware version.");
    }
    ports += merge.ports;
    supportsVNAmode &= merge.supportsVNAmode;
    supportsSGmode &= merge.supportsSGmode;
    supportsSAmode &= merge.supportsSAmode;
    supportsExtRef &= merge.supportsExtRef;
    Limits.minFreq = max(Limits.minFreq, merge.Limits.minFreq);
    Limits.maxFreq = min(Limits.maxFreq, merge.Limits.maxFreq);
    Limits.maxFreqHarmonic = min(Limits.maxFreqHarmonic, merge.Limits.maxFreqHarmonic);
    Limits.minIFBW = max(Limits.minIFBW, merge.Limits.minIFBW);
    Limits.maxIFBW = min(Limits.maxIFBW, merge.Limits.maxIFBW);
    Limits.maxPoints = min(Limits.maxPoints, merge.Limits.maxPoints);
    Limits.mindBm = max(Limits.mindBm, merge.Limits.mindBm);
    Limits.maxdBm = min(Limits.maxdBm, merge.Limits.maxdBm);
    Limits.minRBW = max(Limits.minRBW, merge.Limits.minRBW);
    Limits.maxRBW = min(Limits.maxRBW, merge.Limits.maxRBW);
}

VirtualDevice::Status::Status()
{
    statusString = "";
    overload = false;
    unlocked = false;
    unlevel = false;
    extRef = false;
}

VirtualDevice::Status::Status(Device *dev)
{
    auto status = dev->StatusV1();
    statusString = dev->getLastDeviceInfoString();
    overload = status.ADC_overload;
    unlevel = status.unlevel;
    unlocked = !status.LO1_locked || !status.source_locked;
    extRef = status.extRefInUse;
}

void VirtualDevice::Status::merge(const VirtualDevice::Status &merge)
{
    statusString += " / "+merge.statusString;
    overload |= merge.overload;
    unlevel |= merge.unlevel;
    unlocked |= merge.unlocked;
    extRef &= merge.extRef;
}
