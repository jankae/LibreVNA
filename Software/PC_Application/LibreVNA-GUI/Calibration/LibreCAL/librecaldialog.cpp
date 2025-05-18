#include "librecaldialog.h"
#include "ui_librecaldialog.h"

#include "caldevice.h"
#include "usbdevice.h"
#include "CustomWidgets/informationbox.h"

#include <set>

#include <QFormLayout>

using namespace std;

LibreCALDialog::LibreCALDialog(Calibration *cal) :
    QDialog(nullptr),
    ui(new Ui::LibreCALDialog),
    cal(cal),
    device(nullptr),
    busy(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    createPortAssignmentUI();

    connect(this, &LibreCALDialog::portAssignmentChanged, this, &LibreCALDialog::updateCalibrationStartStatus);

    connect(ui->cbDevice, &QComboBox::currentTextChanged, [=](QString text) {
        if(device) {
            delete device;
            device = nullptr;
        }
        try {
            device = new CalDevice(text);
        } catch (exception &e) {
            device = nullptr;
            InformationBox::ShowError("Failed to connect", e.what(), this);
        }
        if(device) {
            createPortAssignmentUI();
            connect(device, &CalDevice::updateCoefficientsPercent, ui->progressCoeff, &QProgressBar::setValue);
            connect(device, &CalDevice::updateCoefficientsDone, this, [=](bool success){
                busy = false;
                updateCalibrationStartStatus();
                if(success) {
                    ui->progressCoeff->setValue(100);
                    ui->lCalibrationStatus->setText("Coefficients loaded.");
                    coeffSet = device->getCoefficientSets()[0];

                    if(validateCoefficients()) {
                        startCalibration();
                    } else {
                        enableUI();
                    }
                } else {
                    ui->progressCal->setValue(0);
                    ui->lCalibrationStatus->setText("Failed to load coefficients");
                }
            }, Qt::QueuedConnection);

            ui->cbCoefficients->clear();
            ui->cbCoefficients->addItem("Select...");
            for(auto c : device->getCoefficientSetNames()) {
                ui->cbCoefficients->addItem(c);
            }
            ui->cbCoefficients->setEnabled(true);
            // select first available coefficient set
            if(ui->cbCoefficients->count() > 1) {
                ui->cbCoefficients->setCurrentIndex(1);
            }
        } else {
            ui->cbCoefficients->clear();
            ui->cbCoefficients->setEnabled(false);
            ui->start->setEnabled(false);
        }
    });

    connect(ui->cbCoefficients, &QComboBox::currentTextChanged, [=](){
        // no coefficient set selected
        ui->progressCoeff->setValue(0);
        coeffSet = CalDevice::CoefficientSet();
        ui->lCalibrationStatus->setText("No coefficients loaded");
        updateCalibrationStartStatus();
    });

    auto deviceList = USBDevice::GetDevices();
    for(auto device : deviceList) {
        ui->cbDevice->addItem(device);
    }

    connect(ui->start, &QPushButton::clicked, this, &LibreCALDialog::determineAutoPorts);
    connect(this, &LibreCALDialog::autoPortComplete, this, &LibreCALDialog::loadCoefficients);

    updateCalibrationStartStatus();
    updateDeviceStatus();
    connect(&updateTimer, &QTimer::timeout, this, &LibreCALDialog::updateDeviceStatus);
    updateTimer.start(1000);
}

LibreCALDialog::~LibreCALDialog()
{
    if(device) {
        device->abortCoefficientLoading();
    }
    delete device;
    delete ui;
}

bool LibreCALDialog::validatePortSelection(bool autoAllowed)
{
    set<int> usedCalPorts;
    QString status;
    bool canStart = true;
    // Check port mapping for duplicate entries (and at least one used port)
    for(auto port : portAssignment) {
        if(autoAllowed && port == -1) {
            // auto port, to be determined later
            usedCalPorts.insert(port);
            // skip duplicate selection
            continue;
        } else if(port < 1) {
            // skip unused ports
            continue;
        }
        if(usedCalPorts.count(port)) {
            status = "LibreCAL port "+QString::number(port)+" is assigned to multiple VNA ports.";
            canStart = false;
            break;
        } else {
            usedCalPorts.insert(port);
        }
    }
    if(canStart) {
        // at least one port must be used
        if(usedCalPorts.size() == 0) {
            status = "At least one port must be assigned.";
            canStart = false;
        }
    }
    if(!canStart) {
        ui->lCalibrationStatus->setText(status);
        ui->lCalibrationStatus->setStyleSheet("QLabel { color : red; }");
    }
    return canStart;
}

bool LibreCALDialog::validateCoefficients()
{
    bool canStart = true;
    QString status;

    double coeffMinFreq = numeric_limits<double>::max();
    double coeffMaxFreq = numeric_limits<double>::lowest();

    auto checkCoefficient = [&](CalDevice::CoefficientSet::Coefficient *c) -> bool {
        if(!c) {
            return false;
        }
        if(c->t.points() == 0) {
            return false;
        } else {
            if(c->t.minFreq() < coeffMinFreq) {
                coeffMinFreq = c->t.minFreq();
            }
            if(c->t.maxFreq() > coeffMaxFreq) {
                coeffMaxFreq = c->t.maxFreq();
            }
            return true;
        }
    };

    set<int> usedCalPorts;
    // Check port mapping for duplicate entries (and at least one used port)
    for(auto port : portAssignment) {
        if(port > 0) {
            usedCalPorts.insert(port);
        }
    }

    // check if coefficients for all ports are available
    for(auto i : usedCalPorts) {
        // Check if OSL coefficients are there
        if(!checkCoefficient(coeffSet.getOpen(i))) {
            status = "Open coefficient for LibreCAL port "+QString::number(i)+" is missing.";
            canStart = false;
            break;
        }
        if(!checkCoefficient(coeffSet.getShort(i))) {
            status = "Short coefficient for LibreCAL port "+QString::number(i)+" is missing.";
            canStart = false;
            break;
        }
        if(!checkCoefficient(coeffSet.getLoad(i))) {
            status = "Load coefficient for LibreCAL port "+QString::number(i)+" is missing.";
            canStart = false;
            break;
        }
        for(auto j : usedCalPorts) {
            if(j <= i) {
                continue;
            }
            if(!checkCoefficient(coeffSet.getThrough(i,j))) {
                status = "Through coefficient for LibreCAL port "+QString::number(i)+" to "+QString::number(j)+" is missing.";
                canStart = false;
                break;
            }
        }
    }
    if(!canStart) {
        ui->lCalibrationStatus->setText(status);
        ui->lCalibrationStatus->setStyleSheet("QLabel { color : red; }");
    }
    return canStart;
}

void LibreCALDialog::updateCalibrationStartStatus()
{
    bool canStart = true;
    if(!device) {
        canStart = false;
        ui->lCalibrationStatus->setText("Not connected to a LibreCAL device");
        ui->lCalibrationStatus->setStyleSheet("QLabel { color : red; }");
    }

    if(canStart) {
        if(ui->cbCoefficients->currentIndex() == 0) {
            canStart = false;
            ui->lCalibrationStatus->setText("No coefficient set selected");
            ui->lCalibrationStatus->setStyleSheet("QLabel { color : red; }");
        }
    }

    if(canStart) {
        canStart = validatePortSelection(true);
    }

    ui->start->setEnabled(canStart);
    if(canStart) {
        ui->lCalibrationStatus->setText("Ready to start");
        ui->lCalibrationStatus->setStyleSheet("");
    }
}

void LibreCALDialog::updateDeviceStatus()
{
    if(!device) {
        ui->lDeviceStatus->setText("No LibreCAL connected");
        ui->lDeviceStatus->setStyleSheet("QLabel { color : red; }");
        return;
    }
    if(busy) {
        // can't update while busy reading coefficients
        return;
    }
    if(device->stabilized()) {
        ui->lDeviceStatus->setText("LibreCAL ready for calibration");
        ui->lDeviceStatus->setStyleSheet("");
    } else {
        ui->lDeviceStatus->setText("Heating up, please wait with calibration");
        ui->lDeviceStatus->setStyleSheet("QLabel { color : orange; }");
    }
}

void LibreCALDialog::determineAutoPorts()
{
    disableUI();
    ui->progressCal->setValue(0);
    ui->lCalibrationStatus->setText("Autodetecting port connections...");
    ui->lCalibrationStatus->setStyleSheet("QLabel { color : green; }");

    // check if any ports are set to auto
    bool usesAuto = false;
    for(auto port : portAssignment) {
        if(port == -1) {
            usesAuto = true;
            break;
        }
    }
    if(usesAuto) {
        driver = DeviceDriver::getActiveDriver();
        emit driver->acquireControl();
        // Determine ports by setting all ports to open and then switching them to short one at a time while observing the change in the VNA measurement
        for(unsigned int i=1;i<=device->getNumPorts();i++) {
            device->setStandard(i, CalDevice::Standard::Type::Open);
        }
        autoPortMeasurements.clear();
        startSweep();
    } else {
        // no auto ports
        emit autoPortComplete();
    }
}

void LibreCALDialog::loadCoefficients()
{
    // validate ports again, at this point no "auto" selection is allowed anymore
    if(!validatePortSelection(false)) {
        enableUI();
        updateCalibrationStartStatus();
    } else {
        // can continue with loading coefficients
        // they might already be loaded from a previous calibration run, check first
        if(!validateCoefficients()) {
            // only load required coefficients
            ui->progressCal->setValue(0);
            ui->lCalibrationStatus->setText("Loading coefficients...");
            ui->lCalibrationStatus->setStyleSheet("QLabel { color : green; }");

            // determine the used ports of the LibreCAL
            QList<int> usedPorts;
            for(auto port : portAssignment) {
                if(port > 0) {
                    usedPorts.append(port);
                }
            }

            busy = true;            
            device->loadCoefficientSets({ui->cbCoefficients->currentText()}, usedPorts);
        } else {
            // can proceed with calibration directly
            startCalibration();
        }
    }
}

void LibreCALDialog::handleIncomingMeasurement(DeviceDriver::VNAMeasurement m)
{
    stopSweep();
    autoPortMeasurements.push_back(m);
    auto nextPort = autoPortMeasurements.size();
    // switch LibreCAL to next port
    if(nextPort <= device->getNumPorts()) {
        device->setStandard(nextPort, CalDevice::Standard::Type::Short);
    }
    if(nextPort > 1) {
        device->setStandard(nextPort-1, CalDevice::Standard::Type::Open);
    }
    if(nextPort > device->getNumPorts()) {
        // auto port measurements complete

        // reset LibreCAL ports
        for(unsigned int i=1;i<=device->getNumPorts();i++) {
            device->setStandard(i, CalDevice::Standard::Type::None);
        }
        // evaluate results
        for(unsigned int p=0;p<portAssignment.size();p++) {
            auto port = portAssignment[p];
            if(port != -1) {
                // not set to auto, ignore
                continue;
            }
            auto param = "S"+QString::number(p+1)+QString::number(p+1);
            std::vector<std::complex<double>> measurements;
            for(auto m : autoPortMeasurements) {
                if(m.measurements.count(param)) {
                    measurements.push_back(m.measurements[param]);
                }
            }
            if(measurements.size() != device->getNumPorts()+1) {
                // not all measurements available (maybe the port has no stimulus?), set to unused
                portAssignmentComboBoxes[p]->setCurrentIndex(0);
                continue;
            }
            // got all required measurements, determine which one deviates the most from the baseline
            double maxDeviation = 0.0;
            int maxDevIndex = 0;
            for(unsigned int i=1;i<=device->getNumPorts();i++) {
                auto diff = abs(measurements[i] - measurements[0]);
                if (diff > maxDeviation) {
                    maxDeviation = diff;
                    maxDevIndex = i;
                }
            }
            constexpr double minRequiredDeviation = 0.25;
            if(maxDeviation > minRequiredDeviation) {
                portAssignmentComboBoxes[p]->setCurrentIndex(maxDevIndex+1);
            } else {
                // not enough deviation, probably unused
                portAssignmentComboBoxes[p]->setCurrentIndex(0);
            }
        }
        emit driver->releaseControl();
        emit autoPortComplete();
    } else {
        // trigger the next measurement
        startSweep();
    }
}

void LibreCALDialog::startSweep()
{
    // set up a sweep with a single measurement point at the start frequency
    auto info = driver->getInfo();
    DeviceDriver::VNASettings s = {};
    s.dBmStart = info.Limits.VNA.maxdBm;
    s.dBmStop = info.Limits.VNA.maxdBm;

    auto freq = info.Limits.VNA.minFreq == 0 ? 100000 : info.Limits.VNA.minFreq;

    s.freqStart = freq;
    s.freqStop = freq;
    s.IFBW = 100;
    s.logSweep = false;
    s.points = 1;

    for(unsigned int i=1;i<=info.Limits.VNA.ports;i++) {
        s.excitedPorts.push_back(i);
    }
    driver->setVNA(s, [=](bool){
        connect(driver, &DeviceDriver::VNAmeasurementReceived, this, &LibreCALDialog::handleIncomingMeasurement, Qt::DirectConnection);
    });
}

void LibreCALDialog::stopSweep()
{
    disconnect(driver, &DeviceDriver::VNAmeasurementReceived, this, &LibreCALDialog::handleIncomingMeasurement);
    driver->setIdle();
}

void LibreCALDialog::startCalibration()
{
    disableUI();

    ui->progressCal->setValue(0);
    ui->lCalibrationStatus->setText("Creating calibration kit from coefficients...");
    ui->lCalibrationStatus->setStyleSheet("");
    auto& kit = cal->getKit();
    kit.clearStandards();
    kit.manufacturer = "LibreCAL ("+coeffSet.name+")";
    kit.serialnumber = device->serial();
    kit.description = "Automatically created from LibreCAL module";

    // determine the used ports of the LibreCAL
    QList<int> usedPorts;
    for(auto port : portAssignment) {
        if(port > 0) {
            usedPorts.append(port);
        }
    }
    std::sort(usedPorts.begin(), usedPorts.end());

    std::map<int, CalStandard::Virtual*> openStandards;
    std::map<int, CalStandard::Virtual*> shortStandards;
    std::map<int, CalStandard::Virtual*> loadStandards;
    std::map<int, CalStandard::Virtual*> throughStandards;
    for(unsigned int idx=0;idx<usedPorts.size();idx++) {
        int i=usedPorts[idx];
        if(coeffSet.getOpen(i)->t.points() > 0) {
            auto o = new CalStandard::Open();
            o->setName("Port "+QString::number(i));
            o->setMeasurement(coeffSet.getOpen(i)->t);
            openStandards[i] = o;
            kit.addStandard(o);
        }
        if(coeffSet.getShort(i)->t.points() > 0) {
            auto o = new CalStandard::Short();
            o->setName("Port "+QString::number(i));
            o->setMeasurement(coeffSet.getShort(i)->t);
            shortStandards[i] = o;
            kit.addStandard(o);
        }
        if(coeffSet.getLoad(i)->t.points() > 0) {
            auto o = new CalStandard::Load();
            o->setName("Port "+QString::number(i));
            o->setMeasurement(coeffSet.getLoad(i)->t);
            loadStandards[i] = o;
            kit.addStandard(o);
        }
        for(unsigned int jdx=idx+1;jdx<usedPorts.size();jdx++) {
            int j=usedPorts[jdx];
            auto c = coeffSet.getThrough(i,j);
            if(!c) {
                continue;
            }
            if(c->t.points() > 0) {
                auto o = new CalStandard::Through();
                o->setName("Port "+QString::number(i)+" to "+QString::number(j));
                o->setMeasurement(c->t);
                throughStandards[coeffSet.portsToThroughIndex(i, j)] = o;
                kit.addStandard(o);
            }
        }
    }
    ui->lCalibrationStatus->setText("Creating calibration measurements...");
    cal->reset();
    auto vnaPorts = DeviceDriver::getInfo(DeviceDriver::getActiveDriver()).Limits.VNA.ports;
    set<CalibrationMeasurement::Base*> openMeasurements;
    set<CalibrationMeasurement::Base*> shortMeasurements;
    set<CalibrationMeasurement::Base*> loadMeasurements;
    vector<CalibrationMeasurement::TwoPort*> throughMeasurements;
    for(unsigned int p=0;p<vnaPorts;p++) {
        if(portAssignment[p] == 0) {
            continue;
        }
        // Create SOL measurements with correct port of calkit
        auto open = new CalibrationMeasurement::Open(cal);
        open->setPort(p+1);
        open->setStandard(openStandards[portAssignment[p]]);
        openMeasurements.insert(open);
        cal->measurements.push_back(open);

        auto _short = new CalibrationMeasurement::Short(cal);
        _short->setPort(p+1);
        _short->setStandard(shortStandards[portAssignment[p]]);
        shortMeasurements.insert(_short);
        cal->measurements.push_back(_short);

        auto load = new CalibrationMeasurement::Load(cal);
        load->setPort(p+1);
        load->setStandard(loadStandards[portAssignment[p]]);
        loadMeasurements.insert(load);
        cal->measurements.push_back(load);
        for(unsigned int p2=p+1;p2<vnaPorts;p2++) {
            if(portAssignment[p2] == 0) {
                continue;
            }
            auto through = new CalibrationMeasurement::Through(cal);
            through->setPort1(p+1);
            through->setPort2(p2+1);
            // find correct through standard
            int libreCALp1 = portAssignment[p];
            int libreCALp2 = portAssignment[p2];
            QString forwardName = "Port "+QString::number(libreCALp1)+" to "+QString::number(libreCALp2);
            QString reverseName = "Port "+QString::number(libreCALp2)+" to "+QString::number(libreCALp1);
            for(auto ts : throughStandards) {
                if(ts.second->getName() == forwardName) {
                    through->setStandard(ts.second);
                    through->setReverseStandard(false);
                } else if(ts.second->getName() == reverseName) {
                    through->setStandard(ts.second);
                    through->setReverseStandard(true);
                }
            }
            throughMeasurements.push_back(through);
            cal->measurements.push_back(through);
        }
    }

    ui->lCalibrationStatus->setText("Taking calibration measurements...");

    measurementsTaken = 0;

    auto setTerminationOnAllUsedPorts = [=](CalDevice::Standard s) {
        for(auto p : portAssignment) {
            if(p > 0) {
                device->setStandard(p, s);
            }
        }
    };

    auto startNextCalibrationStep = [=]() {
        // indicate calibration percentage
        auto totalMeasurements = 3 + throughMeasurements.size();
        ui->progressCal->setValue(measurementsTaken * 100 / totalMeasurements);
        switch(measurementsTaken) {
        case 0:
            setTerminationOnAllUsedPorts(CalDevice::Standard(CalDevice::Standard::Type::Open));
            emit cal->startMeasurements(openMeasurements);
            break;
        case 1:
            setTerminationOnAllUsedPorts(CalDevice::Standard(CalDevice::Standard::Type::Short));
            emit cal->startMeasurements(shortMeasurements);
            break;
        case 2:
            setTerminationOnAllUsedPorts(CalDevice::Standard(CalDevice::Standard::Type::Load));
            emit cal->startMeasurements(loadMeasurements);
            break;
        default: {
            // into through measurements now
            unsigned int throughIndex = measurementsTaken - 3;
            if(throughIndex >= throughMeasurements.size()) {
                // this was the last measurement
                // Try to apply the calibration
                Calibration::CalType type;
                type.type = Calibration::Type::SOLT;
                for(unsigned int i=0;i<vnaPorts;i++) {
                    if(portAssignment[i] > 0) {
                        // this VNA port was used in the calibration
                       type.usedPorts.push_back(i+1);
                    }
                }
                auto res = cal->compute(type);
                if(res) {
                    ui->progressCal->setValue(100);
                    ui->lCalibrationStatus->setText("Calibration activated.");
                } else {
                    ui->progressCal->setValue(0);
                    ui->lCalibrationStatus->setText("Failed to activate calibration.");
                    ui->lCalibrationStatus->setStyleSheet("QLabel { color : red; }");
                }
                // sever connection to this function
                disconnect(cal, &Calibration::measurementsUpdated, this, nullptr);
                setTerminationOnAllUsedPorts(CalDevice::Standard(CalDevice::Standard::Type::None));
                enableUI();
                break;
            }
            setTerminationOnAllUsedPorts(CalDevice::Standard(CalDevice::Standard::Type::None));
            auto m = throughMeasurements[throughIndex];
            device->setStandard(portAssignment[m->getPort1()-1], CalDevice::Standard(portAssignment[m->getPort2()-1]));
            emit cal->startMeasurements({m});
        }
            break;
        }
        measurementsTaken++;
    };

    disconnect(cal, &Calibration::measurementsUpdated, this, nullptr);
    connect(cal, &Calibration::measurementsUpdated, this, startNextCalibrationStep, Qt::QueuedConnection);
    connect(cal, &Calibration::measurementsAborted, this, [=](){
        setTerminationOnAllUsedPorts(CalDevice::Standard(CalDevice::Standard::Type::None));
        enableUI();
        ui->lCalibrationStatus->setText("Ready to start");
    });

    startNextCalibrationStep();
}

void LibreCALDialog::disableUI()
{
    ui->cbDevice->setEnabled(false);
    ui->cbCoefficients->setEnabled(false);
    ui->start->setEnabled(false);
    for(auto cb : portAssignmentComboBoxes) {
        cb->setEnabled(false);
    }
}

void LibreCALDialog::enableUI()
{
    ui->cbDevice->setEnabled(true);
    ui->cbCoefficients->setEnabled(true);
    ui->start->setEnabled(true);
    for(auto cb : portAssignmentComboBoxes) {
        cb->setEnabled(true);
    }
}

void LibreCALDialog::createPortAssignmentUI()
{
    auto layout = static_cast<QFormLayout*>(ui->assignmentBox->layout());
    // Clear any possible previous elements
    portAssignment.clear();
    portAssignmentComboBoxes.clear();
    while(layout->rowCount() > 1) {
        layout->removeRow(1);
    }
    auto vnaPorts = DeviceDriver::getInfo(DeviceDriver::getActiveDriver()).Limits.VNA.ports;
    portAssignment.resize(vnaPorts, 0);
    auto calPorts = 0;
    if(device) {
        calPorts = device->getNumPorts();
    }
    QStringList choices = {"Unused", "Auto"};
    for(int i=1;i<=calPorts;i++) {
        choices.push_back("Port "+QString::number(i));
    }
    for(unsigned int p = 1;p<=vnaPorts;p++) {
        auto label = new QLabel("Port "+QString::number(p)+":");
        auto comboBox = new QComboBox();
        comboBox->addItems(choices);
        connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
            if(comboBox->currentText().startsWith("Unused")) {
                portAssignment[p-1] = 0;
            } else if(comboBox->currentText().startsWith("Auto")) {
                portAssignment[p-1] = -1;
            } else if(comboBox->currentText().startsWith("Port")) {
                portAssignment[p-1] = comboBox->currentText().back().digitValue();
            }
            emit portAssignmentChanged();
        });
        comboBox->setCurrentIndex(1);
        layout->addRow(label, comboBox);
        portAssignmentComboBoxes.push_back(comboBox);
    }
}
