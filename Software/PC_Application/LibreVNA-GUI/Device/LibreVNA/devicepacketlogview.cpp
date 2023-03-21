#include "devicepacketlogview.h"
#include "ui_devicepacketlogview.h"

#include "CustomWidgets/informationbox.h"
#include "unit.h"

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
        QString filename = QFileDialog::getSaveFileName(nullptr, "Load LibreVNA log data", "", "LibreVNA log files (*.vnalog)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(!filename.endsWith(".usblog")) {
            filename.append(".usblog");
        }
        ofstream file;
        file.open(filename.toStdString());
        file << setw(1) << DevicePacketLog::getInstance().toJSON() << endl;
        file.close();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        QString filename = QFileDialog::getOpenFileName(nullptr, "Load LibreVNA log data", "", "LibreVNA log files (*.vnalog)", nullptr, QFileDialog::DontUseNativeDialog);
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
            addInteger(VFF, "MCU temperature", s.temp_MCU);
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
        case Protocol::PacketType::ManualControl:
        case Protocol::PacketType::ManualStatus:
            // TODO
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
