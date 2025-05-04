#include "compounddriver.h"

#include "../librevnatcpdriver.h"
#include "../librevnausbdriver.h"

#include "ui_compounddriversettingswidget.h"
#include "compounddeviceeditdialog.h"
#include "preferences.h"
#include "Device/LibreVNA/devicepacketlogview.h"

#include <exception>

CompoundDriver::CompoundDriver()
{
    connected = false;
    isIdle = true;
    triggerForwarding = false;
    SApoints = 0;

    drivers.push_back(new LibreVNAUSBDriver);
    drivers.push_back(new LibreVNATCPDriver);

    auto log = new QAction("View Packet Log");
    connect(log, &QAction::triggered, this, [=](){
       auto d = new DevicePacketLogView();
       d->show();
    });
    specificActions.push_back(log);

    auto &p = Preferences::getInstance();
    for(auto d : drivers) {
        p.load(d->driverSpecificSettings());
    }

    specificSettings.push_back(Savable::SettingDescription(&compoundJSONString, "compoundDriver.compoundDeviceJSON", "{}"));
    specificSettings.push_back(Savable::SettingDescription(&captureRawReceiverValues, "compoundDriver.captureRawReceiverValues", false));
    specificSettings.push_back(Savable::SettingDescription(&preservePhase, "compoundDriver.preservePhase", false));
}

CompoundDriver::~CompoundDriver()
{
    disconnect();
    for(auto d : drivers) {
        delete d;
    }
}

std::set<QString> CompoundDriver::GetAvailableDevices()
{
    parseCompoundJSON();

    std::set<QString> availableSerials;
    for(auto d : drivers) {
        availableSerials.merge(d->GetAvailableDevices());
    }

    std::set<QString> ret;

    for(auto cd : configuredDevices) {
        bool allAvailable = true;
        for(auto s : cd->deviceSerials) {
            if(availableSerials.count(s) == 0) {
                allAvailable = false;
                break;
            }
        }
        if(allAvailable) {
            ret.insert(cd->name);
        }
    }

    return ret;
}

bool CompoundDriver::connectTo(QString getSerial)
{
    if(connected) {
        disconnect();
    }
    bool found = false;
    for(auto cd : configuredDevices) {
        if(cd->name == getSerial) {
            activeDevice = *cd;
            found = true;
            break;
        }
    }
    if(!found) {
        qWarning() << "Attempted to connect to an unknown compound device";
        return false;
    }
    std::vector<std::set<QString>> availableSerials;
    for(auto d : drivers) {
        availableSerials.push_back(d->GetAvailableDevices());
    }
    // attempt to connect to all individual devices
    for(auto s : activeDevice.deviceSerials) {
        LibreVNADriver *device = nullptr;
        for(unsigned int i=0;i<availableSerials.size();i++) {
            if(availableSerials[i].count(s) > 0) {
                // this driver can connect to the requested device
                if (i == 0) {
                    device = new LibreVNAUSBDriver();
                    break;
                } else if(i == 1) {
                    auto tcp = new LibreVNATCPDriver();
                    tcp->copyDetectedDevices(*static_cast<LibreVNATCPDriver*>(drivers[i]));
                    device = tcp;
                    break;
                }
            }
        }
        if(!device) {
            qWarning() << "Unable to find required serial for compound device:" <<s;
            disconnect();
            return false;
        }
        auto &p = Preferences::getInstance();
        p.load(device->driverSpecificSettings());
        if(!device->connectDevice(s, true)) {
            qWarning() << "Unable to connect to required serial for compound device:" <<s;
            delete device;
            disconnect();
            return false;
        } else {
            devices.push_back(device);
        }
    }

    // make device connections
    for(auto dev : devices) {
        // Create device connections
        connect(dev, &LibreVNADriver::ConnectionLost, this, &CompoundDriver::ConnectionLost, Qt::QueuedConnection);
        connect(dev, &LibreVNADriver::LogLineReceived, this, [=](QString line){
            emit LogLineReceived(line.prepend(dev->getSerial()+": "));
        });
        connect(dev, &LibreVNADriver::InfoUpdated, this, [=]() {
            updatedInfo(dev);
        });
        connect(dev, &LibreVNADriver::passOnReceivedPacket, this, [=](const Protocol::PacketInfo& packet) {
            incomingPacket(dev, packet);
        });
    }

    connected = true;

    return true;
}

void CompoundDriver::disconnect()
{
    for(auto d : devices) {
        QObject::disconnect(d, nullptr, this, nullptr);
        d->disconnect();
        delete d;
    }
    devices.clear();
    deviceInfos.clear();
    deviceStatus.clear();
    compoundSABuffer.clear();
    compoundVNABuffer.clear();
    connected = false;
}

QString CompoundDriver::getSerial()
{
    if(connected) {
        return activeDevice.name;
    } else {
        return "";
    }
}

std::set<DeviceDriver::Flag> CompoundDriver::getFlags()
{
    std::set<DeviceDriver::Flag> ret;
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
    return ret;
}

QString CompoundDriver::getStatus()
{
    QString ret;
    ret.append("HW Rev.");
    ret.append(info.hardware_version);
    ret.append(" FW "+info.firmware_version);
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
    return ret;
}

QWidget *CompoundDriver::createSettingsWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::CompoundDriverSettingsWidget;
    ui->setupUi(w);

    // Set initial values
    ui->CaptureRawReceiverValues->setChecked(captureRawReceiverValues);
    ui->PreservePhase->setChecked(preservePhase);

    // make connections
    connect(ui->CaptureRawReceiverValues, &QCheckBox::toggled, this, [=](){
        captureRawReceiverValues = ui->CaptureRawReceiverValues->isChecked();
    });
    connect(ui->PreservePhase, &QCheckBox::toggled, this, [=](){
        preservePhase = ui->PreservePhase->isChecked();
    });

    connect(ui->compoundList, &QListWidget::doubleClicked, [=](){
        auto index = ui->compoundList->currentRow();
        if(index >= 0 && index < (int) configuredDevices.size()) {
            auto d = new CompoundDeviceEditDialog(configuredDevices[index]);
            connect(d, &QDialog::accepted, [=](){
                ui->compoundList->item(index)->setText(configuredDevices[index]->getDesription());
                createCompoundJSON();
            });
            d->show();
        }
    });
    connect(ui->compoundAdd, &QPushButton::clicked, [=](){
        auto cd = new CompoundDevice;
        auto d = new CompoundDeviceEditDialog(cd);
        connect(d, &QDialog::accepted, [=](){
            configuredDevices.push_back(cd);
            ui->compoundList->addItem(cd->getDesription());
            createCompoundJSON();
        });
        connect(d, &QDialog::rejected, [=](){
            delete cd;
        });
        d->show();
    });
    connect(ui->compoundDelete, &QPushButton::clicked, [=](){
        auto index = ui->compoundList->currentRow();
        if(index >= 0 && index < (int) configuredDevices.size()) {
            // delete the actual compound device
            delete configuredDevices[index];
            // delete the line in the GUI list
            delete ui->compoundList->takeItem(index);
            // remove compound device from list
            configuredDevices.erase(configuredDevices.begin() + index);
            createCompoundJSON();
        }
    });

    for(auto cd : configuredDevices) {
        ui->compoundList->addItem(cd->getDesription());
    }

    return w;
}

QStringList CompoundDriver::availableVNAMeasurements()
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

bool CompoundDriver::setVNA(const DeviceDriver::VNASettings &s, std::function<void (bool)> cb)
{
    if(!supports(Feature::VNA)) {
        return false;
    }
    if(s.excitedPorts.size() == 0) {
        return setIdle(cb);
    }

    setIdle([this](bool){
        enableTriggerForwarding();
        qDebug() << "Start trigger forwarding";
    });

    // create port->stage mapping
    portStageMapping.clear();
    for(unsigned int i=0;i<s.excitedPorts.size();i++) {
        portStageMapping[s.excitedPorts[i]] = i;
    }

    zerospan = (s.freqStart == s.freqStop) && (s.dBmStart == s.dBmStop);
    VNApoints = s.points;
    // create vector of currently used stimulus ports
    std::vector<CompoundDevice::PortMapping> activeMapping;
    for(auto p : s.excitedPorts) {
        activeMapping.push_back(activeDevice.portMapping[p-1]);
    }
    // Configure the devices
    results.clear();
    bool success = true;
    for(unsigned int i=0;i<devices.size();i++) {
        auto dev = devices[i];
        dev->setSynchronization(activeDevice.sync, i == 0);
        auto devSetting = s;
        // indicate the number of stages
        devSetting.excitedPorts = std::vector<int>(s.excitedPorts.size(), 0);
        // activate the ports of this specific device at the correct stage
        auto p1Stage = CompoundDevice::PortMapping::findActiveStage(activeMapping, i, 0);
        if(p1Stage < s.excitedPorts.size()) {
            devSetting.excitedPorts[p1Stage] = 1;
        }
        auto p2Stage = CompoundDevice::PortMapping::findActiveStage(activeMapping, i, 1);
        if(p2Stage < s.excitedPorts.size()) {
            devSetting.excitedPorts[p2Stage] = 2;
        }
        success &= devices[i]->setVNA(devSetting, [=](bool success){
            if(cb) {
                results[devices[i]] = success;
                checkIfAllTransmissionsComplete(cb);
            }
        });
    }

    lastNonIdleSettings.type = Types::VNA;
    lastNonIdleSettings.vna = s;
    isIdle = false;

    return success;
}

QStringList CompoundDriver::availableSAMeasurements()
{
    QStringList ret;
    for(unsigned int i=1;i<=info.Limits.SA.ports;i++) {
        ret.push_back("PORT"+QString::number(i));
    }
    return ret;
}

bool CompoundDriver::setSA(const DeviceDriver::SASettings &s, std::function<void (bool)> cb)
{
    if(!supports(Feature::SA)) {
        return false;
    }
    zerospan = s.freqStart == s.freqStop;

    setIdle([this](bool){
        enableTriggerForwarding();
        qDebug() << "Start trigger forwarding";
    });

    // Configure the devices
    results.clear();
    bool success = true;
    for(unsigned int i=0;i<devices.size();i++) {
        auto dev = devices[i];
        dev->setSynchronization(activeDevice.sync, i == 0);
        auto devSettings = s;
        devSettings.trackingGenerator = false;
        devSettings.trackingPort = 0;
        if(s.trackingGenerator) {
            if(activeDevice.portMapping[s.trackingPort-1].device == i) {
                // tracking generator active on this device
                devSettings.trackingGenerator = true;
                devSettings.trackingPort = activeDevice.portMapping[s.trackingPort-1].port + 1;
            }
        }
        success &= devices[i]->setSA(devSettings, [=](bool success){
            if(cb) {
                results[devices[i]] = success;
                checkIfAllTransmissionsComplete(cb);
            }
        });
    }
    SApoints = devices[0]->getSApoints();
    for(unsigned int i=1;i<devices.size();i++) {
        if(devices[i]->getSApoints() != SApoints) {
            qWarning() << "Individual devices report different number of SA points, unable to start compound SA sweep";
            setIdle();
            success = false;
            break;
        }
    }

    lastNonIdleSettings.type = Types::SA;
    lastNonIdleSettings.sa = s;
    isIdle = false;

    return success;
}

QStringList CompoundDriver::availableSGPorts()
{
    QStringList ret;
    for(unsigned int i=1;i<info.Limits.Generator.ports;i++) {
        ret.push_back("PORT"+QString::number(i));
    }
    return ret;
}

bool CompoundDriver::setSG(const DeviceDriver::SGSettings &s)
{
    if(!supports(Feature::Generator)) {
        return false;
    }
    // configure all devices
    bool success = true;
    for(unsigned int i=0;i<devices.size();i++) {
        auto devSettings = s;
        devSettings.port = 0;
        if(s.port > 0) {
            if(activeDevice.portMapping[s.port-1].device == i) {
                // this device has the active port
                devSettings.port = activeDevice.portMapping[s.port-1].port+1;
            }
        }
        success &= devices[i]->setSG(devSettings);
    }

    lastNonIdleSettings.type = Types::SG;
    lastNonIdleSettings.sg = s;
    isIdle = false;

    return success;
}

bool CompoundDriver::setIdle(std::function<void (bool)> cb)
{
    disableTriggerForwarding();
    qDebug() << "Stop trigger forwarding";
    auto success = true;
    results.clear();
    for(auto dev : devices) {
        dev->sendWithoutPayload(Protocol::PacketType::ClearTrigger);
        success &= dev->setIdle([=](bool success){
            if(cb) {
                results[dev] = success;
                checkIfAllTransmissionsComplete(cb);
            }
        });
    }

    isIdle = true;

    return success;
}

QStringList CompoundDriver::availableExtRefInSettings()
{
    if(!connected) {
        return QStringList();
    }
    auto list = devices[0]->availableExtRefInSettings();
    QSet<QString> set(list.begin(), list.end());
    for(unsigned int i=1;i<devices.size();i++) {
        list = devices[i]->availableExtRefInSettings();
        set = set.intersect({list.begin(), list.end()});
    }
    return QStringList(set.begin(), set.end());
}

QStringList CompoundDriver::availableExtRefOutSettings()
{
    if(!connected) {
        return QStringList();
    }
    auto list = devices[0]->availableExtRefOutSettings();
    QSet<QString> set(list.begin(), list.end());
    for(unsigned int i=1;i<devices.size();i++) {
        list = devices[i]->availableExtRefOutSettings();
        set = set.intersect({list.begin(), list.end()});
    }
    return QStringList(set.begin(), set.end());
}

bool CompoundDriver::setExtRef(QString option_in, QString option_out)
{
    auto success = true;

    qDebug() << "Ref change start";
    if(isIdle) {
        // can immediately switch reference settings
        for(auto dev : devices) {
            success &= dev->setExtRef(option_in, option_out);
        }
    } else {
        // can not switch during a sweep
        // set to idle first
        setIdle();
        // change reference
        for(auto dev : devices) {
            success &= dev->setExtRef(option_in, option_out);
        }
        // restore last non idle state
        switch(lastNonIdleSettings.type) {
        case Types::VNA:
            setVNA(lastNonIdleSettings.vna);
            break;
        case Types::SA:
            setSA(lastNonIdleSettings.sa);
            break;
        case Types::SG:
            setSG(lastNonIdleSettings.sg);
            break;
        }
    }
    qDebug() << "Ref change stop";

    return success;
}

std::set<QString> CompoundDriver::getIndividualDeviceSerials()
{
    std::vector<LibreVNADriver*> drivers;
    drivers.push_back(new LibreVNAUSBDriver);
    drivers.push_back(new LibreVNATCPDriver);

    auto &p = Preferences::getInstance();
    std::set<QString> ret;
    for(auto d : drivers) {
        p.load(d->driverSpecificSettings());
        ret.merge(d->GetAvailableDevices());
    }
    return ret;
}

void CompoundDriver::triggerReceived(LibreVNADriver *device, bool set)
{
    triggerMutex.lock();
    if(activeDevice.sync == LibreVNADriver::Synchronization::GUI && triggerForwarding) {
        for(unsigned int i=0;i<devices.size();i++) {
            if(devices[i] == device) {
                // pass on to the next device
                if(i < devices.size() - 1) {
                    qDebug() << "Passing on trigger" << set << "from" << device->getSerial() << "to" << devices[i+1]->getSerial();
                    devices[i+1]->sendWithoutPayload(set ? Protocol::PacketType::SetTrigger : Protocol::PacketType::ClearTrigger);
                } else {
                    qDebug() << "Passing on trigger" << set << "from" << device->getSerial() << "to" << devices[0]->getSerial();
                    devices[0]->sendWithoutPayload(set ? Protocol::PacketType::SetTrigger : Protocol::PacketType::ClearTrigger);
                }
                break;
            }
        }
    }
    triggerMutex.unlock();
}

void CompoundDriver::parseCompoundJSON()
{
    try {
        configuredDevices.clear();
        if(compoundJSONString.isEmpty()) {
            // empty string will fail JSON parsing. Abort now instead of running into the exception
            return;
        }
        nlohmann::json jc = nlohmann::json::parse(compoundJSONString.toStdString());
        for(auto j : jc) {
            auto cd = new CompoundDevice();
            cd->fromJSON(j);
            configuredDevices.push_back(cd);
        }
    } catch(const std::exception& e){
        qDebug() << "Failed to parse compound device string: " << e.what();
    }
}

void CompoundDriver::createCompoundJSON()
{
    if(configuredDevices.size() > 0) {
        nlohmann::json j;
        for(auto cd : configuredDevices) {
            j.push_back(cd->toJSON());
        }
        compoundJSONString = QString::fromStdString(j.dump());
    } else {
        compoundJSONString = "[]";
    }
}

void CompoundDriver::incomingPacket(LibreVNADriver *device, const Protocol::PacketInfo &p)
{
    switch(p.type) {
    case Protocol::PacketType::DeviceStatus:
        updatedStatus(device, p.status);
        break;
    case Protocol::PacketType::VNADatapoint:
        datapointReceivecd(device, p.VNAdatapoint);
        break;
    case Protocol::PacketType::SpectrumAnalyzerResult:
        spectrumResultReceived(device, p.spectrumResult);
        break;
    default:
        // nothing to do for other packet types
        break;
    }
}

void CompoundDriver::updatedInfo(LibreVNADriver *device)
{
    deviceInfos[device] = device->getInfo();
    if(deviceInfos.size() == devices.size()) {
        // got infos from all devices
        info = devices[0]->getInfo();
        for(unsigned int i=1;i<devices.size();i++) {
            info.subset(devices[i]->getInfo());
        }
        // overwrite number of ports (not all physical ports may be configured for this compound device)
        info.Limits.VNA.ports = activeDevice.portMapping.size();
        info.Limits.Generator.ports = activeDevice.portMapping.size();
        info.Limits.SA.ports = activeDevice.portMapping.size();
        emit InfoUpdated();
    }
}

void CompoundDriver::updatedStatus(LibreVNADriver *device, const Protocol::DeviceStatus &status)
{
    deviceStatus[device] = status;
    if(deviceStatus.size() == devices.size()) {
        // got status from all devices
        for(unsigned int i=0;i<devices.size();i++) {
            auto devStat = deviceStatus[devices[i]];
            if(i==0) {
                lastStatus = devStat;
            } else {
                lastStatus.V1.extRefAvailable &= devStat.V1.extRefAvailable;
                lastStatus.V1.extRefInUse |= devStat.V1.extRefInUse;
                lastStatus.V1.FPGA_configured &= devStat.V1.FPGA_configured;
                lastStatus.V1.source_locked &= devStat.V1.source_locked;
                lastStatus.V1.LO1_locked &= devStat.V1.LO1_locked;
                lastStatus.V1.ADC_overload |= devStat.V1.ADC_overload;
                lastStatus.V1.unlevel |= devStat.V1.unlevel;
                lastStatus.V1.temp_source = std::max(lastStatus.V1.temp_source, devStat.V1.temp_source);
                lastStatus.V1.temp_LO1 = std::max(lastStatus.V1.temp_LO1, devStat.V1.temp_LO1);
                lastStatus.V1.temp_MCU = std::max(lastStatus.V1.temp_MCU, devStat.V1.temp_MCU);
            }
        }
        emit StatusUpdated();
        emit FlagsUpdated();
    }
}

void CompoundDriver::spectrumResultReceived(LibreVNADriver *dev, Protocol::SpectrumAnalyzerResult res)
{
    if(!compoundSABuffer.count(res.pointNum)) {
        compoundSABuffer[res.pointNum] = std::map<LibreVNADriver*, Protocol::SpectrumAnalyzerResult>();
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
        for(unsigned int port=0;port<activeDevice.portMapping.size();port++) {
            auto device = devices[activeDevice.portMapping[port].device];
            auto devicePort = activeDevice.portMapping[port].port;

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

void CompoundDriver::enableTriggerForwarding()
{
    triggerMutex.lock();
    for(auto d : devices) {
        connect(d, &LibreVNADriver::receivedTrigger, this, &CompoundDriver::triggerReceived, Qt::UniqueConnection);
    }
    triggerForwarding = true;
    triggerMutex.unlock();
}

void CompoundDriver::disableTriggerForwarding()
{
    triggerMutex.lock();
    triggerForwarding = false;
    for(auto d : devices) {
        QObject::disconnect(d, &LibreVNADriver::receivedTrigger, this, &CompoundDriver::triggerReceived);
    }
    triggerMutex.unlock();
}

void CompoundDriver::datapointReceivecd(LibreVNADriver *dev, Protocol::VNADatapoint<32> *data)
{
    if(!compoundVNABuffer.count(data->pointNum)) {
        compoundVNABuffer[data->pointNum] = std::map<LibreVNADriver*, Protocol::VNADatapoint<32>*>();
    }
    auto &buf = compoundVNABuffer[data->pointNum];
    // create copy of datapoint as it will be deleted by the device driver
    buf[dev] = new Protocol::VNADatapoint<32>(*data);
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
            // map.first is the port (starts at one)
            // map.second is the stage at which this port had the stimulus (starts at zero)

            // figure out which device had the stimulus for the port...
            auto stimulusDev = devices[activeDevice.portMapping[map.first-1].device];
            // ...and which device port was used for the stimulus...
            auto stimulusDevPort = activeDevice.portMapping[map.first-1].port;
            // ...grab the reference receiver data
            std::complex<double> ref = buf[stimulusDev]->getValue(map.second, stimulusDevPort, true);

            // for all ports of the compound device...
            for(unsigned int i=0;i<activeDevice.portMapping.size();i++) {
                // ...figure out which physical device and port was used for this input...
                auto inputDevice = devices[activeDevice.portMapping[i].device];
                // ...and grab the data
                auto inputPort = activeDevice.portMapping[i].port;
                std::complex<double> input = buf[inputDevice]->getValue(map.second, inputPort, false);
                if(!std::isnan(ref.real()) && !std::isnan(input.real())) {
                    // got both required measurements
                    QString name = "S"+QString::number(i+1)+QString::number(map.first);
                    auto S = input / ref;
                    if(!preservePhase && (inputDevice != stimulusDev)) {
                        // can't use phase information when measuring across devices
                        S = abs(S);
                    }
                    m.measurements[name] = S;
                }

                if(captureRawReceiverValues) {
                    QString name = "RawPort"+QString::number(inputPort+1)+"Stage"+QString::number(map.second);
                    m.measurements[name] = input;
                    name = "RawPort"+QString::number(inputPort+1)+"Stage"+QString::number(map.second)+"Ref";
                    m.measurements[name] = buf[inputDevice]->getValue(map.second, inputPort, true);
                }
            }
        }

        emit VNAmeasurementReceived(m);

        // Clear this and all (incomplete) older datapoint buffers
        int pointNum = data->pointNum;
        while(compoundVNABuffer.count(pointNum)) {
            auto &buf = compoundVNABuffer[pointNum];
            for(auto d : buf) {
                delete d.second;
            }
            compoundVNABuffer.erase(pointNum);
            // move on to previous point
            if(pointNum > 0) {
                pointNum--;
            } else {
                pointNum = VNApoints - 1;
            }
        }
    }
}

void CompoundDriver::checkIfAllTransmissionsComplete(std::function<void (bool)> cb)
{
    if(results.size() == devices.size()) {
        // got all responses
        bool success = true;
        for(auto res : results) {
            if(res.second != true) {
                success = false;
                break;
            }
        }
        if(cb) {
            cb(success);
        }
    }
}
