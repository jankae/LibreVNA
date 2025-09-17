#include "librevnadriver.h"

#include "manualcontroldialogV1.h"
#include "manualcontroldialogvff.h"
#include "manualcontroldialogvfe.h"
#include "manualcontroldialogVE0.h"
#include "deviceconfigurationdialogv1.h"
#include "deviceconfigurationdialogvff.h"
#include "deviceconfigurationdialogvfe.h"
#include "firmwareupdatedialog.h"
#include "frequencycaldialog.h"
#include "sourcecaldialog.h"
#include "receivercaldialog.h"
#include "unit.h"
#include "CustomWidgets/informationbox.h"
#include "devicepacketlogview.h"

#include "ui_librevnadriversettingswidget.h"

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
    isIdle = true;
    SApoints = 0;
    hardwareVersion = 0;
    protocolVersion = 0;
    setSynchronization(Synchronization::Disabled, false);
    manualControlDialog = nullptr;

    // Add driver specific actions

    auto startManualControl = [=](){
        manualControlDialog = nullptr;
        switch(hardwareVersion) {
        case 1:
            manualControlDialog = new ManualControlDialogV1(*this);
            break;
        case 0xE0:
            manualControlDialog = new ManualControlDialogVE0(*this);
            break;
        case 0xFE:
            manualControlDialog = new ManualControlDialogVFE(*this);
            break;
        case 0xFF:
            manualControlDialog = new ManualControlDialogVFF(*this);
            break;
        }
        if(manualControlDialog) {
            manualControlDialog->show();
            connect(manualControlDialog, &QDialog::finished, this, [=](){
                manualControlDialog = nullptr;
            });
        }
    };

    auto manual = new QAction("Manual Control");
    connect(manual, &QAction::triggered, this, startManualControl);
    specificActions.push_back(manual);

    auto config = new QAction("Configuration");
    connect(config, &QAction::triggered, this, [=](){
        QDialog *d = nullptr;
        switch(hardwareVersion) {
        case 1:
            d = new DeviceConfigurationDialogV1(*this);
            break;
        case 0xFE:
            d = new DeviceConfigurationDialogVFE(*this);
            break;
        case 0xFF:
            d = new DeviceConfigurationDialogVFF(*this);
            break;
        }
        if(d) {
            d->show();
        }
    });
    specificActions.push_back(config);

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

    sep = new QAction();
    sep->setSeparator(true);
    specificActions.push_back(sep);

    auto log = new QAction("View Packet Log");
    connect(log, &QAction::triggered, this, [=](){
       auto d = new DevicePacketLogView();
       d->show();
    });
    specificActions.push_back(log);

    // Create driver specific commands
    specificSCPIcommands.push_back(new SCPICommand("DEVice:INFo:TEMPeratures", nullptr, [=](QStringList) -> QString {
        if(!connected) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return QString::number(lastStatus.V1.temp_source)+"/"+QString::number(lastStatus.V1.temp_LO1)+"/"+QString::number(lastStatus.V1.temp_MCU);
    }));

    specificSCPIcommands.push_back(new SCPICommand("DEVice:UPDATE", [=](QStringList params) -> QString {
        if(!connected) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(params.size() != 1) {
            // no file given
            return SCPI::getResultName(SCPI::Result::Error);
        }
        auto ret = updateFirmware(params[0]);
        if(!ret) {
            // update failed
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            // update succeeded
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, nullptr, false));

    specificSCPIcommands.push_back(new SCPICommand("MANual:STArt", [=](QStringList) -> QString {
        if(!manualControlDialog) {
            startManualControl();
            if(!manualControlDialog) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));

    specificSCPIcommands.push_back(new SCPICommand("MANual:STOp", [=](QStringList) -> QString {
        if(manualControlDialog) {
            delete manualControlDialog;
            manualControlDialog = nullptr;
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));

    specificSCPIcommands.push_back(new SCPICommand("DEVice:PACKETLOG", nullptr, [=](QStringList) -> QString {
        auto &log = DevicePacketLog::getInstance();
        return QString::fromStdString(log.toJSON().dump());
    }));
}

std::set<DeviceDriver::Flag> LibreVNADriver::getFlags()
{
    std::set<DeviceDriver::Flag> ret;
    switch(hardwareVersion) {
    case 1:
        if(lastStatus.V1.extRefInUse) {
            ret.insert(Flag::ExtRef);
        }
        if(!lastStatus.V1.source_locked || !lastStatus.V1.LO1_locked) {
            ret.insert(Flag::Unlocked);
        }
        if(lastStatus.V1.unlevel) {
            ret.insert(Flag::Unlevel);
        }
        if(lastStatus.V1.ADC_overload) {
            ret.insert(Flag::Overload);
        }
        break;
    case 0xFE:
        if(!lastStatus.VFE.source_locked || !lastStatus.VFE.LO_locked) {
            ret.insert(Flag::Unlocked);
        }
        if(lastStatus.VFE.unlevel) {
            ret.insert(Flag::Unlevel);
        }
        if(lastStatus.VFE.ADC_overload) {
            ret.insert(Flag::Overload);
        }
        break;
    case 0xFF:
        if(!lastStatus.VFF.source_locked || !lastStatus.VFF.LO_locked) {
            ret.insert(Flag::Unlocked);
        }
        if(lastStatus.VFF.unlevel) {
            ret.insert(Flag::Unlevel);
        }
        if(lastStatus.VFF.ADC_overload) {
            ret.insert(Flag::Overload);
        }
        break;
    }
    return ret;
}

QString LibreVNADriver::getStatus()
{
    QString ret;
    ret.append("HW ");
    ret.append(info.hardware_version);
    ret.append(" FW "+info.firmware_version);
    switch (hardwareVersion) {
    case 1:
        ret.append(" Temps: Source PLL: "+QString::number(lastStatus.V1.temp_source)+"°C LO PLL: "+QString::number(lastStatus.V1.temp_LO1)+"°C MCU: "+QString::number(lastStatus.V1.temp_MCU)+"°C");
        ret.append(" Reference:");
        if(lastStatus.V1.extRefInUse) {
            ret.append("External");
        } else {
            ret.append("Internal");
            if(lastStatus.V1.extRefAvailable) {
                ret.append(" (External available)");
            }
        }
        break;
    case 0xFE:
        ret.append(" MCU Temp: "+QString::number(lastStatus.VFE.temp_MCU)+"°C");
        ret.append(" eCal Temp: "+QString::number(lastStatus.VFE.temp_eCal / 100.0)+"°C");
        ret.append(" eCal Power: "+QString::number(lastStatus.VFE.power_heater / 1000.0)+"W");
        break;
    case 0xFF:
        ret.append(" MCU Temp: "+QString::number(lastStatus.VFF.temp_MCU)+"°C");
        break;
    }
    return ret;
}

QWidget *LibreVNADriver::createSettingsWidget()
{
    auto w = new QWidget;
    auto ui = new Ui::LibreVNADriverSettingsWidget;
    ui->setupUi(w);

    // Set initial values
    ui->CaptureRawReceiverValues->setChecked(captureRawReceiverValues);
    ui->UseHarmonicMixing->setChecked(harmonicMixing);
    ui->UseSignalID->setChecked(SASignalID);
    ui->SuppressPeaks->setChecked(VNASuppressInvalidPeaks);
    ui->AdjustPowerLevel->setChecked(VNAAdjustPowerLevel);
    ui->DFTlimitRBW->setEnabled(false);
    connect(ui->UseDFT, &QCheckBox::toggled, ui->DFTlimitRBW, &SIUnitEdit::setEnabled);
    ui->UseDFT->setChecked(SAUseDFT);
    ui->DFTlimitRBW->setUnit("Hz");
    ui->DFTlimitRBW->setPrefixes(" kM");
    ui->DFTlimitRBW->setPrecision(3);
    ui->DFTlimitRBW->setValue(SARBWLimitForDFT);

    connect(ui->UseHarmonicMixing, &QCheckBox::toggled, [=](bool enabled) {
       if(enabled) {
           InformationBox::ShowMessage("Harmonic Mixing", "When harmonic mixing is enabled, the frequency range of the VNA is (theoretically) extended up to 18GHz "
                                       "by using higher harmonics of the source signal as well as the 1.LO. The fundamental frequency is still present "
                                       "in the output signal and might disturb the measurement if the DUT is not linear. Performance above 6GHz is not "
                                       "specified and generally not very good. However, this mode might be useful if the signal of interest is just above "
                                       "6GHz (typically useful values up to 7-8GHz). Performance below 6GHz is not affected by this setting");
       }
    });

    // make connections to change the values
    connect(ui->CaptureRawReceiverValues, &QCheckBox::toggled, this, [=](){
        captureRawReceiverValues = ui->CaptureRawReceiverValues->isChecked();
    });
    connect(ui->UseHarmonicMixing, &QCheckBox::toggled, this, [=](){
        harmonicMixing = ui->UseHarmonicMixing->isChecked();
    });
    connect(ui->UseSignalID, &QCheckBox::toggled, this, [=](){
        SASignalID = ui->UseSignalID->isChecked();
    });
    connect(ui->SuppressPeaks, &QCheckBox::toggled, this, [=](){
        VNASuppressInvalidPeaks = ui->SuppressPeaks->isChecked();
    });
    connect(ui->AdjustPowerLevel, &QCheckBox::toggled, this, [=](){
        VNAAdjustPowerLevel = ui->AdjustPowerLevel->isChecked();
    });
    connect(ui->UseDFT, &QCheckBox::toggled, this, [=](){
        SAUseDFT = ui->UseDFT->isChecked();
    });
    connect(ui->DFTlimitRBW, &SIUnitEdit::valueChanged, this, [=](){
       SARBWLimitForDFT = ui->DFTlimitRBW->value();
    });

    return w;
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
        qDebug() << "VNA does not support features \"VNA\" (has the DeviceInfo been received?)";
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
    auto dwell_us = s.dwellTime * 1e6;
    if(dwell_us < 0) {
        dwell_us = 0;
    } else if(dwell_us > UINT16_MAX) {
        dwell_us = UINT16_MAX;
    }
    p.settings.dwell_time = dwell_us;
    p.settings.suppressPeaks = VNASuppressInvalidPeaks ? 1 : 0;
    p.settings.fixedPowerSetting = VNAAdjustPowerLevel || s.dBmStart != s.dBmStop ? 0 : 1;
    p.settings.logSweep = s.logSweep ? 1 : 0;

    zerospan = (s.freqStart == s.freqStop) && (s.dBmStart == s.dBmStop);
    p.settings.port1Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 1) - s.excitedPorts.begin();
    p.settings.port2Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 2) - s.excitedPorts.begin();
    p.settings.port3Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 3) - s.excitedPorts.begin();
    p.settings.port4Stage = find(s.excitedPorts.begin(), s.excitedPorts.end(), 4) - s.excitedPorts.begin();
    p.settings.syncMode = (int) sync;
    p.settings.syncMaster = syncMaster ? 1 : 0;

    isIdle = false;
    lastNonIdlePacket = p;

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
    if(!s.trackingGenerator && SAUseDFT && s.RBW <= SARBWLimitForDFT && s.freqStart != s.freqStop) {
        p.spectrumSettings.UseDFT = 1;
    }
    p.spectrumSettings.applyReceiverCorrection = 1;
    p.spectrumSettings.trackingGeneratorOffset = s.trackingOffset;
    p.spectrumSettings.trackingPower = s.trackingPower * 100;

    p.spectrumSettings.trackingGenerator = s.trackingGenerator ? 1 : 0;
    p.spectrumSettings.trackingGeneratorPort = s.trackingPort - 1;
    p.spectrumSettings.syncMode = (int) sync;
    p.spectrumSettings.syncMaster = syncMaster ? 1 : 0;

    isIdle = false;
    lastNonIdlePacket = p;

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
    isIdle = false;
    lastNonIdlePacket = p;
    return SendPacket(p);
}

bool LibreVNADriver::setIdle(std::function<void (bool)> cb)
{
    isIdle = true;
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
    if(hardwareVersion == 0x01) {
        for(auto r : Reference::getReferencesIn()) {
            ret.push_back(Reference::TypeToLabel(r));
        }
    }
    return ret;
}

QStringList LibreVNADriver::availableExtRefOutSettings()
{
    QStringList ret;
    if(hardwareVersion == 0x01) {
        for(auto r : Reference::getOutFrequencies()) {
            ret.push_back(Reference::OutFreqToLabel(r));
        }
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
        if(hardwareVersion == 0x01) {
            lastStatus.V1.extRefInUse = 0;
        }
        break;
    case Reference::TypeIn::Auto:
        p.reference.UseExternalRef = 0;
        p.reference.AutomaticSwitch = 1;
        break;
    case Reference::TypeIn::External:
        p.reference.UseExternalRef = 1;
        p.reference.AutomaticSwitch = 0;
        if(hardwareVersion == 0x01) {
            lastStatus.V1.extRefInUse = 1;
        }
        break;
    }
    switch(refOut) {
    case Reference::OutFreq::None:
    case Reference::OutFreq::Off: p.reference.ExtRefOuputFreq = 0; break;
    case Reference::OutFreq::MHZ10: p.reference.ExtRefOuputFreq = 10000000; break;
    case Reference::OutFreq::MHZ100: p.reference.ExtRefOuputFreq = 100000000; break;
    }

    bool ret;
    if(isIdle) {
        // can switch reference directly
        ret = SendPacket(p);
    } else {
        // switching the reference while a sweep (or any frequency generation is active)
        // can result in wrong frequencies when a frequency calibration is applied to
        // the internal reference. Stop any activity before switching the reference and
        // start it again afterwards
        ret = sendWithoutPayload(Protocol::PacketType::SetIdle);
        ret &= SendPacket(p);
        ret &= SendPacket(lastNonIdlePacket);
    }

    return ret;
}

void LibreVNADriver::registerTypes()
{
    qRegisterMetaType<Protocol::PacketInfo>();
    qRegisterMetaType<TransmissionResult>();
    qRegisterMetaType<Protocol::AmplitudeCorrectionPoint>();
}

void LibreVNADriver::setSynchronization(LibreVNADriver::Synchronization s, bool master)
{
    sync = s;
    syncMaster = master;
}

void LibreVNADriver::handleReceivedPacket(const Protocol::PacketInfo &packet)
{
    emit passOnReceivedPacket(packet);

    if(skipOwnPacketHandling) {
        return;
    }

    switch(packet.type) {
    case Protocol::PacketType::DeviceInfo: {
        // Check protocol version
        protocolVersion = packet.info.ProtocolVersion;
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

        hardwareVersion = packet.info.hardware_version;
        info.firmware_version = QString::number(packet.info.FW_major)+"."+QString::number(packet.info.FW_minor)+"."+QString::number(packet.info.FW_patch);
        info.hardware_version = hardwareVersionToString(packet.info.hardware_version)+" Rev."+QString(packet.info.HW_Revision);
        info.supportedFeatures = {
            Feature::VNA, Feature::VNAFrequencySweep, Feature::VNALogSweep, Feature::VNAPowerSweep, Feature::VNAZeroSpan, Feature::VNADwellTime,
            Feature::Generator,
            Feature::SA, Feature::SATrackingGenerator, Feature::SATrackingOffset,
            Feature::ExtRefIn, Feature::ExtRefOut,
        };
        info.Limits.VNA.ports = packet.info.num_ports;
        info.Limits.VNA.minFreq = packet.info.limits_minFreq;
        info.Limits.VNA.maxFreq = harmonicMixing ? packet.info.limits_maxFreqHarmonic : packet.info.limits_maxFreq;
        info.Limits.VNA.maxPoints = packet.info.limits_maxPoints;
        info.Limits.VNA.minIFBW = packet.info.limits_minIFBW;
        info.Limits.VNA.maxIFBW = packet.info.limits_maxIFBW;
        info.Limits.VNA.mindBm = (double) packet.info.limits_cdbm_min / 100;
        info.Limits.VNA.maxdBm = (double) packet.info.limits_cdbm_max / 100;
        info.Limits.VNA.maxDwellTime = (double) packet.info.limits_maxDwellTime * 1e-6;

        info.Limits.Generator.ports = packet.info.num_ports;
        info.Limits.Generator.minFreq = packet.info.limits_minFreq;
        info.Limits.Generator.maxFreq = packet.info.limits_maxFreq;
        info.Limits.Generator.mindBm = (double) packet.info.limits_cdbm_min / 100;
        info.Limits.Generator.maxdBm = (double) packet.info.limits_cdbm_max / 100;

        info.Limits.SA.ports = packet.info.num_ports;
        info.Limits.SA.minFreq = packet.info.limits_minFreq;
        info.Limits.SA.maxFreq = packet.info.limits_maxFreq;
        info.Limits.SA.minRBW = packet.info.limits_minRBW;
        info.Limits.SA.maxRBW = packet.info.limits_maxRBW;
        info.Limits.SA.mindBm = (double) packet.info.limits_cdbm_min / 100;
        info.Limits.SA.maxdBm = (double) packet.info.limits_cdbm_max / 100;

        limits_maxAmplitudePoints = packet.info.limits_maxAmplitudePoints;
        emit InfoUpdated();
    }
        break;
    case Protocol::PacketType::DeviceStatus:
        lastStatus = packet.status;
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
            // map.first is the port (starts at one)
            // map.second is the stage at which this port had the stimulus (starts at zero)
            complex<double> ref = res->getValue(map.second, map.first-1, true);
            for(unsigned int i=1;i<=info.Limits.VNA.ports;i++) {
                complex<double> input = res->getValue(map.second, i-1, false);
                if(!std::isnan(ref.real()) && !std::isnan(input.real())) {
                    // got both required measurements
                    QString name = "S"+QString::number(i)+QString::number(map.first);
                    m.measurements[name] = input / ref;
                }
                if(captureRawReceiverValues) {
                    QString name = "RawPort"+QString::number(i)+"Stage"+QString::number(map.second);
                    m.measurements[name] = input;
                    name = "RawPort"+QString::number(i)+"Stage"+QString::number(map.second)+"Ref";
                    m.measurements[name] = res->getValue(map.second, i-1, true);
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

QString LibreVNADriver::hardwareVersionToString(uint8_t version)
{
    switch(version) {
    case 0x01: return "1";
    case 0xE0: return "SAP1";
    case 0xFE: return "P2";
    case 0xFF: return "PT";
    default: return "Unknown";
    }
}

unsigned int LibreVNADriver::getProtocolVersion() const
{
    return protocolVersion;
}

unsigned int LibreVNADriver::getMaxAmplitudePoints() const
{
    return limits_maxAmplitudePoints;
}

QString LibreVNADriver::getFirmwareMagicString()
{
    switch(hardwareVersion) {
    case 0x01: return "VNA!";
    case 0xE0: return "VNS1";
    case 0xFE: return "VNP2";
    case 0xFF: return "VNPT";
    default: return "XXXX";
    }
}

bool LibreVNADriver::sendWithoutPayload(Protocol::PacketType type, std::function<void(TransmissionResult)> cb)
{
    Protocol::PacketInfo p;
    p.type = type;
    return SendPacket(p, cb);
}

bool LibreVNADriver::updateFirmware(QString file)
{
    return FirmwareUpdateDialog::FirmwareUpdate(this, file);
}
