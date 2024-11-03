#include "devicepacketlogview.h"
#include "ui_devicepacketlogview.h"

#include "CustomWidgets/informationbox.h"
#include "unit.h"
#include "preferences.h"

#include <fstream>
#include <iomanip>

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QtEndian>

using namespace std;

DevicePacketLogView::DevicePacketLogView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DevicePacketLogView)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

//    connect(&log, &DeviceUSBLog::entryAdded, this, &DeviceUSBLogView::addEntry);
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, [=](){
        DevicePacketLog::getInstance().reset();
        updateTree();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        QString filename = QFileDialog::getSaveFileName(nullptr, "Load LibreVNA log data", "", "LibreVNA log files (*.vnalog)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(!filename.endsWith(".vnalog")) {
            filename.append(".vnalog");
        }
        ofstream file;
        file.open(filename.toStdString());
        file << setw(1) << DevicePacketLog::getInstance().toJSON() << endl;
        file.close();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        QString filename = QFileDialog::getOpenFileName(nullptr, "Load LibreVNA log data", "", "LibreVNA log files (*.vnalog)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        ifstream file;
        file.open(filename.toStdString());
        if(!file.is_open()) {
            qWarning() << "Unable to open file:" << filename;
            return;
        }
        nlohmann::json j;
        try {
            // TODO this can take a long time, move to thread
            file >> j;
        } catch (exception &e) {
            InformationBox::ShowError("Error", "Failed to parse the USB log file (" + QString(e.what()) + ")");
            qWarning() << "Parsing of USB log file failed: " << e.what();
            file.close();
            return;
        }
        file.close();
        DevicePacketLog::getInstance().fromJSON(j);
        updateTree();
    });

    updateTree();
}

DevicePacketLogView::~DevicePacketLogView()
{
    delete ui;
}

void DevicePacketLogView::updateTree()
{
    auto &log = DevicePacketLog::getInstance();

    ui->tree->clear();
    ui->tree->setColumnCount(4);
    ui->tree->setHeaderLabels({"Timestamp","Source","Type","Content"});

    unsigned int i=0;
    try {
        // will throw once all entries have been added (number of entries may change while the loop is running)
        for(i=0;;i++) {
            addEntry(log.getEntry(i));
        }
    } catch (...) {
    }

    QString status = "Log contains "+QString::number(i) + " entries, using ";
    status += Unit::ToString(log.getUsedStorageSize(), "B", " kMG") + " (maximum: "+Unit::ToString(log.getMaxStorageSize(), "B", " kMG")+")";
    ui->status->setText(status);
}

void DevicePacketLogView::addEntry(const DevicePacketLog::LogEntry &e)
{
    auto item = new QTreeWidgetItem;
    item->setData(0, Qt::DisplayRole, e.timestamp.toString(Qt::DateFormat::ISODateWithMs));
    item->setData(1, Qt::DisplayRole, e.serial.size() > 0 ? e.serial : "LibreVNA-GUI");
    item->setData(2, Qt::DisplayRole, e.type == DevicePacketLog::LogEntry::Type::Packet ? "Packet" : "Invalid bytes");
    if(e.type == DevicePacketLog::LogEntry::Type::Packet) {
        item->setData(2, Qt::DisplayRole, "Packet");

        static const QStringList packetNames = {"None", "Datapoint", "SweepSettings", "ManualStatus", "ManualControl", "DeviceInfo", "FirmwarePacket", "Ack",
                                               "ClearFlash", "PerformFirmwareUpdate", "Nack", "Reference", "Generator", "SpectrumAnalyzerSettings",
                                               "SpectrumAnalyzerResult", "RequestDeviceInfo", "RequestSourceCal", "RequestReceiverCal", "SourceCalPoint",
                                               "ReceiverCalPoint", "SetIdle", "RequestFrequencyCorrection", "FrequencyCorrection", "RequestDeviceConfiguration",
                                               "DeviceConfiguration", "DeviceStatus", "RequestDeviceStatus", "VNADatapoint", "SetTrigger", "ClearTrigger"};

        item->setData(3, Qt::DisplayRole, "Type "+QString::number((int)e.p->type)+"("+packetNames[(int)e.p->type]+")");
        auto addDouble = [=](QTreeWidgetItem *parent, QString name, double value, QString unit = "", int precision = 8) {
            auto subitem = new QTreeWidgetItem;
            subitem->setData(2, Qt::DisplayRole, name);
            subitem->setData(3, Qt::DisplayRole, Unit::ToString(value, unit, "pnum kMG", precision));
            parent->addChild(subitem);
        };
        auto addInteger = [=](QTreeWidgetItem *parent, QString name, int value) {
            auto subitem = new QTreeWidgetItem;
            subitem->setData(2, Qt::DisplayRole, name);
            subitem->setData(3, Qt::DisplayRole, value);
            parent->addChild(subitem);
        };
        auto addBool = [=](QTreeWidgetItem *parent, QString name, bool value) {
            auto subitem = new QTreeWidgetItem;
            subitem->setData(2, Qt::DisplayRole, name);
            subitem->setData(3, Qt::DisplayRole, value ? "true" : "false");
            parent->addChild(subitem);
        };
        auto addEnum = [=](QTreeWidgetItem *parent, QString name, int value, QStringList names) {
            auto subitem = new QTreeWidgetItem;
            subitem->setData(2, Qt::DisplayRole, name);
            subitem->setData(3, Qt::DisplayRole, names[value]);
            parent->addChild(subitem);
        };
        auto addString = [=](QTreeWidgetItem *parent, QString name, QString value) {
            auto subitem = new QTreeWidgetItem;
            subitem->setData(2, Qt::DisplayRole, name);
            subitem->setData(3, Qt::DisplayRole, value);
            parent->addChild(subitem);
        };

        switch(e.p->type) {
        case Protocol::PacketType::SweepSettings: {
            Protocol::SweepSettings s = e.p->settings;
            addDouble(item, "Start frequency", s.f_start, "Hz");
            addDouble(item, "Stop frequency", s.f_stop, "Hz");
            addInteger(item, "Points", s.points);
            addDouble(item, "IF bandwidth", s.if_bandwidth, "Hz");
            addDouble(item, "Start power", (double) s.cdbm_excitation_start / 100.0, "dBm");
            addDouble(item, "Stop power", (double) s.cdbm_excitation_stop / 100.0, "dBm");
            addEnum(item, "Sync mode", s.syncMode, {"no synchronization", "USB", "Ext. reference", "Ext. trigger"});
            addBool(item, "Sync Master", s.syncMaster);
            addBool(item, "Suppress peaks", s.suppressPeaks);
            addBool(item, "Fixed power setting", s.fixedPowerSetting);
            addBool(item, "Logarithmic sweep", s.logSweep);
            addInteger(item, "Stages", s.stages);
            addInteger(item, "Port 1 stage", s.port1Stage);
            addInteger(item, "Port 2 stage", s.port2Stage);
        }
            break;
        case Protocol::PacketType::Reference: {
            Protocol::ReferenceSettings s = e.p->reference;
            addDouble(item, "Output frequency", s.ExtRefOuputFreq, "Hz");
            addBool(item, "Switch automatically", s.AutomaticSwitch);
            addBool(item, "Use external reference", s.UseExternalRef);
        }
            break;
        case Protocol::PacketType::Generator: {
            Protocol::GeneratorSettings s = e.p->generator;
            addDouble(item, "Frequency", s.frequency, "Hz");
            addDouble(item, "Power", (double) s.cdbm_level / 100.0, "dBm");
            addBool(item, "Use source calibration", s.applyAmplitudeCorrection);
            addInteger(item, "Active port", s.activePort);
        }
            break;
        case Protocol::PacketType::DeviceStatus: {
            auto s = e.p->status.V1;
            auto V1 = new QTreeWidgetItem();
            V1->setData(2, Qt::DisplayRole, "V1");
            item->addChild(V1);
            addBool(V1, "External reference available", s.extRefAvailable);
            addBool(V1, "External reference in use", s.extRefInUse);
            addBool(V1, "FPGA configured", s.FPGA_configured);
            addBool(V1, "Source locked", s.source_locked);
            addBool(V1, "1.LO locked", s.LO1_locked);
            addBool(V1, "ADC overload", s.ADC_overload);
            addBool(V1, "Unlevel", s.unlevel);
            addInteger(V1, "Source temperature", s.temp_source);
            addInteger(V1, "1.LO temperature", s.temp_LO1);
            addInteger(V1, "MCU temperature", s.temp_MCU);

            auto sFF = e.p->status.VFF;
            auto VFF = new QTreeWidgetItem();
            VFF->setData(2, Qt::DisplayRole, "VFF");
            item->addChild(VFF);
            addBool(VFF, "Source locked", sFF.source_locked);
            addBool(VFF, "LO locked", sFF.LO_locked);
            addBool(VFF, "ADC overload", sFF.ADC_overload);
            addBool(VFF, "Unlevel", sFF.unlevel);
            addInteger(VFF, "MCU temperature", sFF.temp_MCU);

            auto sFE = e.p->status.VFE;
            auto VFE = new QTreeWidgetItem();
            VFE->setData(2, Qt::DisplayRole, "VFE");
            item->addChild(VFE);
            addBool(VFE, "Source locked", sFE.source_locked);
            addBool(VFE, "LO locked", sFE.LO_locked);
            addBool(VFE, "ADC overload", sFE.ADC_overload);
            addBool(VFE, "Unlevel", sFE.unlevel);
            addInteger(VFE, "MCU temperature", sFE.temp_MCU);
            addDouble(VFE, "eCal temperature", (double) sFE.temp_eCal / 100.0);
            addDouble(VFE, "eCal heater power", (double) sFE.power_heater / 1000.0);
        }
            break;
        case Protocol::PacketType::DeviceInfo: {
            Protocol::DeviceInfo s = e.p->info;
            addInteger(item, "Protocol version", s.ProtocolVersion);
            addInteger(item, "Firmware major", s.FW_major);
            addInteger(item, "Firmware minor", s.FW_minor);
            addInteger(item, "Firmware patch", s.FW_patch);
            addInteger(item, "Hardware version", s.hardware_version);
            addString(item, "Hardware version", QString(s.HW_Revision));
            addDouble(item, "Minimum frequency", s.limits_minFreq, "Hz");
            addDouble(item, "Maximum frequency", s.limits_maxFreq, "Hz");
            addDouble(item, "Minimum IF bandwidth", s.limits_minIFBW, "Hz");
            addDouble(item, "Maximum IF bandwidht", s.limits_maxIFBW, "Hz");
            addInteger(item, "Maximum points", s.limits_maxPoints);
            addDouble(item, "Minimum power", (double) s.limits_cdbm_min / 100.0, "dBm");
            addDouble(item, "Maximum power", (double) s.limits_cdbm_max / 100.0, "dBm");
            addDouble(item, "Minimum RBW", s.limits_minRBW, "Hz");
            addDouble(item, "Maximum RBW", s.limits_maxRBW, "Hz");
            addInteger(item, "Maximum amplitude points", s.limits_maxAmplitudePoints);
            addDouble(item, "Maximum harmonic frequency", s.limits_maxFreqHarmonic, "Hz");
        }
            break;
        case Protocol::PacketType::ManualControl: {
            Protocol::ManualControl s = e.p->manual;
            auto V1 = new QTreeWidgetItem();
            V1->setData(2, Qt::DisplayRole, "V1");
            item->addChild(V1);
            addBool(V1, "High Source chip enable", s.V1.SourceHighCE);
            addBool(V1, "High Source RF enable", s.V1.SourceHighRFEN);
            addEnum(V1, "High Source power", s.V1.SourceHighPower, {"-4 dBm", "-1 dBm", "2 dBm", "5 dBm"});
            addEnum(V1, "High Source lowpass", s.V1.SourceHighLowpass, {"947 MHz", "1880 MHz", "3550 MHz", "None"});
            addDouble(V1, "High Source frequency", s.V1.SourceHighFrequency);
            addBool(V1, "Low Source enable", s.V1.SourceLowEN);
            addEnum(V1, "Low Source power", s.V1.SourceLowPower, {"2 mA", "4 mA", "6 mA", "8 mA"});
            addDouble(V1, "Low Source frequency", s.V1.SourceLowFrequency);
            addDouble(V1, "Attenuator", s.V1.attenuator * 0.25);
            addEnum(V1, "Source band selection", s.V1.SourceHighband, {"Low Source", "High Source"});
            addBool(V1, "Amplifier enable", s.V1.AmplifierEN);
            addEnum(V1, "Port switch", s.V1.PortSwitch, {"Port 1", "Port 2"});
            addBool(V1, "LO1 chip enable", s.V1.LO1CE);
            addBool(V1, "LO1 RF enable", s.V1.LO1RFEN);
            addDouble(V1, "LO1 frequency", s.V1.LO1Frequency);
            addBool(V1, "LO2 enable", s.V1.LO2EN);
            addDouble(V1, "LO2 frequency", s.V1.LO2Frequency);
            addBool(V1, "Port 1 receiver enable", s.V1.Port1EN);
            addBool(V1, "Port 2 receiver enable", s.V1.Port2EN);
            addBool(V1, "Reference receiver enable", s.V1.RefEN);
            addInteger(V1, "Samples", s.V1.Samples);
            addEnum(V1, "Window type", s.V1.WindowType, {"None", "Kaiser", "Hann", "Flattop"});

            auto VFF = new QTreeWidgetItem();
            VFF->setData(2, Qt::DisplayRole, "VFF");
            item->addChild(VFF);
            addBool(VFF, "Source chip enable", s.VFF.SourceCE);
            addBool(VFF, "Source RF enable", s.VFF.SourceRFEN);
            addEnum(VFF, "Source power", s.VFF.SourcePower, {"-1 dBm", "1 dBm", "2.5 dBm", "3.5 dBm", "4.5 dBm", "5.5 dBm", "6.5 dBm", "7 dBm"});
            addDouble(VFF, "Source frequency", s.VFF.SourceFrequency);
            addDouble(VFF, "Attenuator", s.VFF.attenuator * 0.25);
            addBool(VFF, "Source amplifier enable", s.VFF.SourceAmplifierEN);
            addBool(VFF, "LO chip enable", s.VFF.LOCE);
            addBool(VFF, "LO RF enable", s.VFF.LORFEN);
            addBool(VFF, "LO amplifier enable", s.VFF.LOAmplifierEN);
            addEnum(VFF, "LO selection", s.VFF.LOexternal, {"Internal", "External"});
            addDouble(VFF, "LO frequency", s.VFF.LOFrequency);
            addBool(VFF, "Port receiver enable", s.VFF.PortEN);
            addBool(VFF, "Reference receiver enable", s.VFF.RefEN);
            addInteger(VFF, "Samples", s.VFF.Samples);
            addEnum(VFF, "Window type", s.VFF.WindowType, {"None", "Kaiser", "Hann", "Flattop"});
            addEnum(VFF, "Port gain", s.VFF.PortGain, {"1 V/V", "10 V/V", "20 V/V", "30 V/V", "40 V/V", "60 V/V", "80 V/V", "120 V/V", "157 V/V", "0.25 V/V"});
            addEnum(VFF, "Reference gain", s.VFF.RefGain, {"1 V/V", "10 V/V", "20 V/V", "30 V/V", "40 V/V", "60 V/V", "80 V/V", "120 V/V", "157 V/V", "0.25 V/V"});

            auto VFE = new QTreeWidgetItem();
            VFE->setData(2, Qt::DisplayRole, "VFE");
            item->addChild(VFE);
            addBool(VFE, "Source chip enable", s.VFE.SourceCE);
            addBool(VFE, "Source RF enable", s.VFE.SourceRFEN);
            addDouble(VFE, "Source frequency", s.VFE.SourceFrequency);
            addDouble(VFE, "Attenuator", s.VFE.attenuator * 0.25);
            addBool(VFE, "Source amplifier 1 enable", s.VFE.SourceAmplifier1EN);
            addBool(VFE, "Source amplifier 2 enable", s.VFE.SourceAmplifier2EN);
            addBool(VFE, "LO chip enable", s.VFE.LOCE);
            addBool(VFE, "LO RF enable", s.VFE.LORFEN);
            addDouble(VFE, "LO frequency", s.VFE.LOFrequency);
            addBool(VFE, "Port receiver enable", s.VFE.PortEN);
            addBool(VFE, "Reference receiver enable", s.VFE.RefEN);
            addInteger(VFE, "Samples", s.VFE.Samples);
            addEnum(VFE, "Window type", s.VFE.WindowType, {"None", "Kaiser", "Hann", "Flattop"});
            addEnum(VFE, "Port gain", s.VFE.PortGain, {"1 V/V", "10 V/V", "20 V/V", "30 V/V", "40 V/V", "60 V/V", "80 V/V", "120 V/V", "157 V/V", "0.25 V/V"});
            addEnum(VFE, "Reference gain", s.VFE.RefGain, {"1 V/V", "10 V/V", "20 V/V", "30 V/V", "40 V/V", "60 V/V", "80 V/V", "120 V/V", "157 V/V", "0.25 V/V"});
            addEnum(VFE, "eCal state", s.VFE.eCal_state, {"Port", "Open", "Short", "Load"});
            addDouble(VFE, "eCal target temperature", (double) s.VFE.eCal_target / 100.0);
        }
            break;
        case Protocol::PacketType::ManualStatus: {
            Protocol::ManualStatus s = e.p->manualStatus;
            auto V1 = new QTreeWidgetItem();
            V1->setData(2, Qt::DisplayRole, "V1");
            item->addChild(V1);
            addInteger(V1, "ADC port 1 minimum", s.V1.port1min);
            addInteger(V1, "ADC port 1 maximum", s.V1.port1max);
            addInteger(V1, "ADC port 2 minimum", s.V1.port2min);
            addInteger(V1, "ADC port 2 maximum", s.V1.port2max);
            addInteger(V1, "ADC reference minimum", s.V1.refmin);
            addInteger(V1, "ADC reference maximum", s.V1.refmax);
            addDouble(V1, "Port 1 real", s.V1.port1real);
            addDouble(V1, "Port 1 imaginary", s.V1.port1imag);
            addDouble(V1, "Port 2 real", s.V1.port2real);
            addDouble(V1, "Port 2 imaginary", s.V1.port2imag);
            addDouble(V1, "Reference real", s.V1.refreal);
            addDouble(V1, "Reference imaginary", s.V1.refimag);
            addInteger(V1, "Source temperature", s.V1.temp_source);
            addInteger(V1, "LO1 temperature", s.V1.temp_LO);
            addBool(V1, "Source locked", s.V1.source_locked);
            addBool(V1, "LO1 locked", s.V1.LO_locked);

            auto VFF = new QTreeWidgetItem();
            VFF->setData(2, Qt::DisplayRole, "VFF");
            item->addChild(VFF);
            addInteger(VFF, "ADC port minimum", s.VFF.portmin);
            addInteger(VFF, "ADC port maximum", s.VFF.portmax);
            addInteger(VFF, "ADC reference minimum", s.VFF.refmin);
            addInteger(VFF, "ADC reference maximum", s.VFF.refmax);
            addDouble(VFF, "Port real", s.VFF.portreal);
            addDouble(VFF, "Port imaginary", s.VFF.portimag);
            addDouble(VFF, "Reference real", s.VFF.refreal);
            addDouble(VFF, "Reference imaginary", s.VFF.refimag);
            addBool(VFF, "Source locked", s.VFF.source_locked);
            addBool(VFF, "LO locked", s.VFF.LO_locked);

            auto VFE = new QTreeWidgetItem();
            VFE->setData(2, Qt::DisplayRole, "VFE");
            item->addChild(VFE);
            addInteger(VFE, "ADC port minimum", s.VFE.portmin);
            addInteger(VFE, "ADC port maximum", s.VFE.portmax);
            addInteger(VFE, "ADC reference minimum", s.VFE.refmin);
            addInteger(VFE, "ADC reference maximum", s.VFE.refmax);
            addDouble(VFE, "Port real", s.VFE.portreal);
            addDouble(VFE, "Port imaginary", s.VFE.portimag);
            addDouble(VFE, "Reference real", s.VFE.refreal);
            addDouble(VFE, "Reference imaginary", s.VFE.refimag);
            addBool(VFE, "Source locked", s.VFE.source_locked);
            addBool(VFE, "LO locked", s.VFE.LO_locked);
            addDouble(VFE, "eCal temperature", (double) s.VFE.temp_eCal / 100.0);
            addDouble(VFE, "eCal heater power", (double) s.VFE.power_heater / 1000.0);
        }
            break;
        case Protocol::PacketType::SpectrumAnalyzerSettings: {
            Protocol::SpectrumAnalyzerSettings s = e.p->spectrumSettings;
            addDouble(item, "Start frequency", s.f_start, "Hz");
            addDouble(item, "Stop frequency", s.f_stop, "Hz");
            addInteger(item, "Points", s.pointNum);
            addDouble(item, "Resolution bandwidth", s.RBW, "Hz");
            addEnum(item, "Window", s.syncMode, {"None", "Kaiser", "Hann", "Flattop"});
            addBool(item, "Signal ID", s.SignalID);
            addBool(item, "Use DFT", s.UseDFT);
            addBool(item, "Use receiver calibration", s.applyReceiverCorrection);
            addBool(item, "Use source calibration", s.applySourceCorrection);
            addEnum(item, "Detector", s.Detector, {"Positive peak", "Negative peak", "Sample", "Normal", "Average"});
            addEnum(item, "Sync mode", s.syncMode, {"no synchronization", "USB", "Ext. reference", "Ext. trigger"});
            addBool(item, "Sync Master", s.syncMaster);
            addBool(item, "Enable tracking generator", s.trackingGenerator);
            addInteger(item, "Tracking generator port", s.trackingGeneratorPort+1);
            addDouble(item, "Tracking generator offset", s.trackingGeneratorOffset, "Hz");
            addDouble(item, "Tracking generator power", (double) s.trackingPower / 100.0, "dBm");
        }
            break;
        case Protocol::PacketType::VNADatapoint: {
            Protocol::VNADatapoint<32>* s = e.datapoint;
            addInteger(item, "Point number", s->pointNum);
            addDouble(item, "Frequency/time", s->frequency, "Hz");
            addDouble(item, "Power", (double) s->cdBm / 100.0, "dBm");
            for(unsigned int i=0;i<s->getNumValues();i++) {
                auto v = s->getValue(i);
                vector<int> ports;
                if(v.flags & 0x01) {
                    ports.push_back(1);
                }
                if(v.flags & 0x02) {
                    ports.push_back(2);
                }
                if(v.flags & 0x04) {
                    ports.push_back(3);
                }
                if(v.flags & 0x08) {
                    ports.push_back(4);
                }
                bool reference = v.flags & 0x10;
                int stage = v.flags >> 5;
                auto vitem = new QTreeWidgetItem;
                vitem->setData(2, Qt::DisplayRole, "Measurement "+QString::number(i+1));
                vitem->setData(3, Qt::DisplayRole, "Real: "+QString::number(v.value.real())+" Imag: "+QString::number(v.value.imag()));
                addInteger(vitem, "Stage", stage);
                addBool(vitem, "Reference", reference);
                QString sports = QString::number(ports.front());
                for(unsigned int j=1;j<ports.size();j++) {
                    sports += ","+QString::number(ports[j]);
                }
                addString(vitem, "Ports", sports);
                item->addChild(vitem);
            }
        }
            break;
        case Protocol::PacketType::SpectrumAnalyzerResult: {
            Protocol::SpectrumAnalyzerResult s = e.p->spectrumResult;
            addDouble(item, "Port 1 level", s.port1);
            addDouble(item, "Port 2 level", s.port2);
            addDouble(item, "Frequency/time", s.frequency, "Hz");
            addInteger(item, "Point number", s.pointNum);
        }
            break;
        case Protocol::PacketType::SourceCalPoint:
        case Protocol::PacketType::ReceiverCalPoint: {
            Protocol::AmplitudeCorrectionPoint s = e.p->amplitudePoint;
            addInteger(item, "Total points", s.totalPoints);
            addInteger(item, "Point number", s.pointNum);
            addDouble(item, "Frequency", s.freq, "Hz");
            addInteger(item, "Port 1 correction", s.port1);
            addInteger(item, "Port 2 correction", s.port2);
        }
            break;
        case Protocol::PacketType::FrequencyCorrection: {
            Protocol::FrequencyCorrection s = e.p->frequencyCorrection;
            addDouble(item, "ppm", s.ppm, "");
        }
            break;
        case Protocol::PacketType::DeviceConfiguration: {
            auto s1 = e.p->deviceConfig.V1;
            auto V1 = new QTreeWidgetItem();
            V1->setData(2, Qt::DisplayRole, "V1");
            item->addChild(V1);
            addDouble(V1, "1.IF", s1.IF1, "Hz");
            addInteger(V1, "ADC prescaler", s1.ADCprescaler);
            addInteger(V1, "DFT phase increment", s1.DFTphaseInc);

            auto sFF = e.p->deviceConfig.VFF;
            auto VFF = new QTreeWidgetItem();
            VFF->setData(2, Qt::DisplayRole, "VFF");
            item->addChild(VFF);
            addBool(VFF, "DHCP enabled", sFF.dhcp);
            addString(VFF, "IP", QHostAddress(qFromBigEndian(sFF.ip)).toString());
            addString(VFF, "Mask", QHostAddress(qFromBigEndian(sFF.mask)).toString());
            addString(VFF, "Gateway", QHostAddress(qFromBigEndian(sFF.gw)).toString());
            addBool(VFF, "PGA autogain", sFF.autogain);
            addInteger(VFF, "Port gain", sFF.portGain);
            addInteger(VFF, "Reference gain", sFF.refGain);

            auto sFE = e.p->deviceConfig.VFE;
            auto VFE = new QTreeWidgetItem();
            VFE->setData(2, Qt::DisplayRole, "VFE");
            item->addChild(VFE);
            addBool(VFE, "PGA autogain", sFE.autogain);
            addInteger(VFE, "Port gain", sFE.portGain);
            addInteger(VFE, "Reference gain", sFE.refGain);
        }
            break;
        default:
            break;
        }
    } else {
        item->setData(2, Qt::DisplayRole, "Invalid bytes");
        item->setData(3, Qt::DisplayRole, QString::number(e.bytes.size())+ " bytes");
        for(auto b : e.bytes) {
            auto subitem = new QTreeWidgetItem;
            subitem->setData(3, Qt::DisplayRole, "0x"+QString::number(b, 16));
            item->addChild(subitem);
        }
    }
    ui->tree->addTopLevelItem(item);
}
