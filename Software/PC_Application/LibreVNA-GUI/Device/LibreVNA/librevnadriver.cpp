#include "librevnadriver.h"

#include "manualcontroldialog.h"
#include "firmwareupdatedialog.h"
#include "frequencycaldialog.h"
#include "sourcecaldialog.h"
#include "receivercaldialog.h"

#include "unit.h"
#include "CustomWidgets/informationbox.h"

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

LibreVNADriver::LibreVNADriver()
{
    connected = false;
    skipOwnPacketHandling = false;
    SApoints = 0;

    auto manual = new QAction("Manual Control");
    connect(manual, &QAction::triggered, this, [=](){
        auto d = new ManualControlDialog(*this);
        d->show();
    });
    specificActions.push_back(manual);

    auto update = new QAction("Firmware Update");
    connect(update, &QAction::triggered, this, [=](){
       auto d = new FirmwareUpdateDialog(this);
       d->show();
    });
    specificActions.push_back(update);

    auto sep = new QAction();
    sep->setSeparator(true);
    specificActions.push_back(sep);

    auto srccal = new QAction("Source Calibration");
    connect(srccal, &QAction::triggered, this, [=](){
       auto d = new SourceCalDialog(this);
       d->show();
    });
    specificActions.push_back(srccal);

    auto recvcal = new QAction("Receiver Calibration");
    connect(recvcal, &QAction::triggered, this, [=](){
       auto d = new ReceiverCalDialog(this);
       d->show();
    });
    specificActions.push_back(recvcal);

    auto freqcal = new QAction("Frequency Calibration");
    connect(freqcal, &QAction::triggered, this, [=](){
       auto d = new FrequencyCalDialog(this);
       d->show();
    });
    specificActions.push_back(freqcal);

    specificSettings.push_back(Savable::SettingDescription(&captureRawReceiverValues, "captureRawReceiverValues", false));
    specificSettings.push_back(Savable::SettingDescription(&harmonicMixing, "harmonicMixing", false));
    specificSettings.push_back(Savable::SettingDescription(&SASignalID, "signalID", true));
    specificSettings.push_back(Savable::SettingDescription(&VNASuppressInvalidPeaks, "suppressInvalidPeaks", true));
    specificSettings.push_back(Savable::SettingDescription(&VNAAdjustPowerLevel, "adjustPowerLevel", false));
    specificSettings.push_back(Savable::SettingDescription(&SAUseDFT, "useDFT", true));
    specificSettings.push_back(Savable::SettingDescription(&SARBWLimitForDFT, "RBWlimitDFT", 3000));
}

std::set<DeviceDriver::Flag> LibreVNADriver::getFlags()
{
    std::set<DeviceDriver::Flag> ret;
    if(lastStatus.extRefInUse) {
        ret.insert(Flag::ExtRef);
    }
    if(!lastStatus.source_locked || !lastStatus.LO1_locked) {
        ret.insert(Flag::Unlocked);
    }
    if(lastStatus.unlevel) {
        ret.insert(Flag::Unlevel);
    }
    if(lastStatus.ADC_overload) {
        ret.insert(Flag::Overload);
    }
    return ret;
}

QString LibreVNADriver::getStatus()
{
    QString ret;
    ret.append("HW Rev.");
    ret.append(info.hardware_version);
    ret.append(" FW "+info.firmware_version);
    ret.append(" Temps: "+QString::number(lastStatus.temp_source)+"°C/"+QString::number(lastStatus.temp_LO1)+"°C/"+QString::number(lastStatus.temp_MCU)+"°C");
    ret.append(" Reference:");
    if(lastStatus.extRefInUse) {
        ret.append("External");
    } else {
        ret.append("Internal");
        if(lastStatus.extRefAvailable) {
            ret.append(" (External available)");
        }
    }
    return ret;
}

QStringList LibreVNADriver::availableVNAMeasurements()
{
    QStringList ret;
    for(unsigned int i=1;i<=info.Limits.VNA.ports;i++) {
        for(unsigned int j=1;j<=info.Limits.VNA.ports;j++) {
            ret.push_back("S"+QString::number(i)+QString::number(j));
        }
    }

    if(captureRawReceiverValues) {
        for(unsigned int i=1;i<=info.Limits.VNA.ports;i++) {
            for(unsigned int j=0;j<info.Limits.VNA.ports;j++) {
                ret.push_back("RawPort"+QString::number(i)+"Stage"+QString::number(j));
                ret.push_back("RawPort"+QString::number(i)+"Stage"+QString::number(j)+"Ref");
            }
        }
    }
    return ret;
}

bool LibreVNADriver::setVNA(const DeviceDriver::VNASettings &s, std::function<void (bool)> cb)
{
    if(!supports(Feature::VNA)) {
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

    Protocol::PacketInfo p = {};
    p.type = Protocol::PacketType::SweepSettings;
    p.settings.f_start = s.freqStart;
    p.settings.f_stop = s.freqStop;
    p.settings.points = s.points;
    p.settings.if_bandwidth = s.IFBW;
    p.settings.cdbm_excitation_start = s.dBmStart * 100;
    p.settings.cdbm_excitation_stop = s.dBmStop * 100;
    p.settings.stages = s.excitedPorts.size() - 1;
    p.settings.suppressPeaks = VNASuppressInvalidPeaks ? 1 : 0;
    p.settings.fixedPowerSetting = VNAAdjustPowerLevel || s.dBmStart != s.dBmStop ? 0 : 1;
    p.settings.logSweep = s.logSweep ? 1 : 0;

    zerospan = (s.freqStart == s.freqStop) && (s.dBmStart == s.dBmStop);
    p.settings.port1Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 0) - s.excitedPorts.begin();
    p.settings.port2Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 1) - s.excitedPorts.begin();
    p.settings.syncMode = 0;
    p.settings.syncMaster = 0;

    return SendPacket(p, [=](TransmissionResult r){
        if(cb) {
            cb(r == TransmissionResult::Ack);
        }
    });
}

QStringList LibreVNADriver::availableSAMeasurements()
{
    QStringList ret;
    for(unsigned int i=1;i<=info.Limits.SA.ports;i++) {
        ret.push_back("PORT"+QString::number(i));
    }
    return ret;
}

bool LibreVNADriver::setSA(const DeviceDriver::SASettings &s, std::function<void (bool)> cb)
{
    if(!supports(Feature::SA)) {
        return false;
    }

    zerospan = s.freqStart == s.freqStop;

    Protocol::PacketInfo p = {};
    p.type = Protocol::PacketType::SpectrumAnalyzerSettings;
    p.spectrumSettings.f_start = s.freqStart;
    p.spectrumSettings.f_stop = s.freqStop;

    constexpr unsigned int maxSApoints = 1001;
    if(s.freqStop - s.freqStart >= maxSApoints || s.freqStop - s.freqStart <= 0) {
        SApoints = maxSApoints;
    } else {
        SApoints = s.freqStop - s.freqStart + 1;
    }

    p.spectrumSettings.pointNum = SApoints;
    p.spectrumSettings.RBW = s.RBW;
    p.spectrumSettings.WindowType = (int) s.window;
    p.spectrumSettings.SignalID = SASignalID ? 1 : 0;
    p.spectrumSettings.Detector = (int) s.detector;
    p.spectrumSettings.UseDFT = 0;
    if(!s.trackingGenerator && SAUseDFT && s.RBW <= SARBWLimitForDFT) {
        p.spectrumSettings.UseDFT = 1;
    }
    p.spectrumSettings.applyReceiverCorrection = 1;
    p.spectrumSettings.trackingGeneratorOffset = s.trackingOffset;
    p.spectrumSettings.trackingPower = s.trackingPower;

    p.spectrumSettings.trackingGenerator = s.trackingGenerator ? 1 : 0;
    p.spectrumSettings.trackingGeneratorPort = s.trackingPort;
    p.spectrumSettings.syncMode = 0;
    p.spectrumSettings.syncMaster = 0;

    if(p.spectrumSettings.trackingGenerator && p.spectrumSettings.f_stop >= 25000000) {
        // Check point spacing.
        // The highband PLL used as the tracking generator is not able to reach every frequency exactly. This
        // could lead to sharp drops in the spectrum at certain frequencies. If the span is wide enough with
        // respect to the point number, it is ensured that every displayed point has at least one sample with
        // a reachable PLL frequency in it. Display a warning message if this is not the case with the current
        // settings.
        auto pointSpacing = (p.spectrumSettings.f_stop - p.spectrumSettings.f_start) / (p.spectrumSettings.pointNum - 1);
        // The frequency resolution of the PLL is frequency dependent (due to PLL divider).
        // This code assumes some knowledge of the actual hardware and probably should be moved
        // onto the device at some point
        double minSpacing = 25000;
        auto stop = p.spectrumSettings.f_stop;
        while(stop <= 3000000000) {
            minSpacing /= 2;
            stop *= 2;
        }
        if(pointSpacing < minSpacing) {
            auto requiredMinSpan = minSpacing * (p.spectrumSettings.pointNum - 1);
            auto message = QString() + "Due to PLL limitations, the tracking generator can not reach every frequency exactly. "
                            "With your current span, this could result in the signal not being detected at some bands. A minimum"
                            " span of " + Unit::ToString(requiredMinSpan, "Hz", " kMG") + " is recommended at this stop frequency.";
            InformationBox::ShowMessage("Warning", message, "TrackingGeneratorSpanTooSmallWarning");
        }
    }

    return SendPacket(p, [=](TransmissionResult r){
        if(cb) {
            cb(r == TransmissionResult::Ack);
        }
    });
}

QStringList LibreVNADriver::availableSGPorts()
{
    QStringList ret;
    for(unsigned int i=1;i<info.Limits.Generator.ports;i++) {
        ret.push_back("PORT"+QString::number(i));
    }
    return ret;
}

bool LibreVNADriver::setSG(const DeviceDriver::SGSettings &s)
{
    Protocol::PacketInfo p = {};
    p.type = Protocol::PacketType::Generator;
    p.generator.frequency = s.freq;
    p.generator.cdbm_level = s.dBm * 100;
    p.generator.activePort = s.port;
    p.generator.applyAmplitudeCorrection = true;
    return SendPacket(p);
}

bool LibreVNADriver::setIdle(std::function<void (bool)> cb)
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::SetIdle;
    return SendPacket(p, [=](TransmissionResult res) {
        if(cb) {
            cb(res == TransmissionResult::Ack);
        }
    });
}

QStringList LibreVNADriver::availableExtRefInSettings()
{
    QStringList ret;
    for(auto r : Reference::getReferencesIn()) {
        ret.push_back(Reference::TypeToLabel(r));
    }
    return ret;
}

QStringList LibreVNADriver::availableExtRefOutSettings()
{
    QStringList ret;
    for(auto r : Reference::getOutFrequencies()) {
        ret.push_back(Reference::OutFreqToLabel(r));
    }
    return ret;
}

bool LibreVNADriver::setExtRef(QString option_in, QString option_out)
{
    auto refIn = Reference::KeyToType(option_in);
    if(refIn == Reference::TypeIn::None) {
        refIn = Reference::TypeIn::Internal;
    }
    auto refOut = Reference::KeyToOutFreq(option_out);
    if(refOut == Reference::OutFreq::None) {
        refOut = Reference::OutFreq::Off;
    }

    Protocol::PacketInfo p = {};
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

    return SendPacket(p);
}

void LibreVNADriver::registerTypes()
{
    qRegisterMetaType<Protocol::PacketInfo>();
    qRegisterMetaType<TransmissionResult>();
}

void LibreVNADriver::handleReceivedPacket(const Protocol::PacketInfo &packet)
{
    emit passOnReceivedPacket(packet);

    if(skipOwnPacketHandling) {
        return;
    }

    switch(packet.type) {
    case Protocol::PacketType::DeviceInfo:
        // Check protocol version
        if(packet.info.ProtocolVersion != Protocol::Version) {
            auto ret = InformationBox::AskQuestion("Warning",
                                        "The device reports a different protocol"
                                        "version (" + QString::number(packet.info.ProtocolVersion) + ") than expected (" + QString::number(Protocol::Version) + ").\n"
                                        "A firmware update is strongly recommended. Do you want to update now?", false);
            if (ret) {
                auto d = new FirmwareUpdateDialog(this);
                d->show();
            }
        }

        info.firmware_version = QString::number(packet.info.FW_major)+"."+QString::number(packet.info.FW_minor)+"."+QString::number(packet.info.FW_patch);
        info.hardware_version = QString::number(packet.info.hardware_version)+QString(packet.info.HW_Revision);
        info.supportedFeatures = {
            Feature::VNA, Feature::VNAFrequencySweep, Feature::VNALogSweep, Feature::VNAPowerSweep, Feature::VNAZeroSpan,
            Feature::Generator,
            Feature::SA, Feature::SATrackingGenerator, Feature::SATrackingOffset,
            Feature::ExtRefIn, Feature::ExtRefOut,
        };
        info.Limits.VNA.ports = 2;
        info.Limits.VNA.minFreq = packet.info.limits_minFreq;
        info.Limits.VNA.maxFreq = harmonicMixing ? packet.info.limits_maxFreqHarmonic : packet.info.limits_maxFreq;
        info.Limits.VNA.maxPoints = packet.info.limits_maxPoints;
        info.Limits.VNA.minIFBW = packet.info.limits_minIFBW;
        info.Limits.VNA.maxIFBW = packet.info.limits_maxIFBW;
        info.Limits.VNA.mindBm = (double) packet.info.limits_cdbm_min / 100;
        info.Limits.VNA.maxdBm = (double) packet.info.limits_cdbm_max / 100;

        info.Limits.Generator.ports = 2;
        info.Limits.Generator.minFreq = packet.info.limits_minFreq;
        info.Limits.Generator.maxFreq = packet.info.limits_maxFreq;
        info.Limits.Generator.mindBm = (double) packet.info.limits_cdbm_min / 100;
        info.Limits.Generator.maxdBm = (double) packet.info.limits_cdbm_max / 100;

        info.Limits.SA.ports = 2;
        info.Limits.SA.minFreq = packet.info.limits_minFreq;
        info.Limits.SA.maxFreq = packet.info.limits_maxFreq;
        info.Limits.SA.minRBW = packet.info.limits_minRBW;
        info.Limits.SA.maxRBW = packet.info.limits_maxRBW;
        info.Limits.SA.mindBm = (double) packet.info.limits_cdbm_min / 100;
        info.Limits.SA.maxdBm = (double) packet.info.limits_cdbm_max / 100;

        limits_maxAmplitudePoints = packet.info.limits_maxAmplitudePoints;
        emit InfoUpdated();
        break;
    case Protocol::PacketType::DeviceStatusV1:
        lastStatus = packet.statusV1;
        emit StatusUpdated();
        emit FlagsUpdated();
        break;
    case Protocol::PacketType::VNADatapoint: {
        VNAMeasurement m;
        Protocol::VNADatapoint<32> *res = packet.VNAdatapoint;
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
            for(unsigned int i=0;i<info.Limits.VNA.ports;i++) {
                complex<double> input = res->getValue(map.second, i, false);
                if(!std::isnan(ref.real()) && !std::isnan(input.real())) {
                    // got both required measurements
                    QString name = "S"+QString::number(i+1)+QString::number(map.first+1);
                    m.measurements[name] = input / ref;
                }
                if(captureRawReceiverValues) {
                    QString name = "RawPort"+QString::number(i+1)+"Stage"+QString::number(map.first);
                    m.measurements[name] = input;
                    name = "RawPort"+QString::number(i+1)+"Stage"+QString::number(map.first)+"Ref";
                    m.measurements[name] = res->getValue(map.second, i, true);
                }
            }
        }
        delete res;
        emit VNAmeasurementReceived(m);
    }
        break;
    case Protocol::PacketType::SpectrumAnalyzerResult: {
        SAMeasurement m;
        m.pointNum = packet.spectrumResult.pointNum;
        if(zerospan) {
            m.us = packet.spectrumResult.us;
        } else {
            m.frequency = packet.spectrumResult.frequency;
        }
        m.measurements["PORT1"] = packet.spectrumResult.port1;
        m.measurements["PORT2"] = packet.spectrumResult.port2;
        emit SAmeasurementReceived(m);
    }
        break;
    default:
        break;
    }
}

unsigned int LibreVNADriver::getMaxAmplitudePoints() const
{
    return limits_maxAmplitudePoints;
}

bool LibreVNADriver::sendWithoutPayload(Protocol::PacketType type, std::function<void(TransmissionResult)> cb)
{
    Protocol::PacketInfo p;
    p.type = type;
    return SendPacket(p, cb);
}
