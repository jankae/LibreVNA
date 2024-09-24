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
                if(success) {
                    ui->progressCoeff->setValue(100);
                    ui->lCoefficientStatus->setText("Coefficients loaded.");
                    coeffSet = device->getCoefficientSets()[0];
                } else {
                    ui->progressCal->setValue(0);
                    ui->lCoefficientStatus->setText("Failed to load coefficients");
                }
                updateCalibrationStartStatus();
            }, Qt::QueuedConnection);

            ui->cbCoefficients->clear();
            ui->cbCoefficients->addItem("Select...");
            for(auto c : device->getCoefficientSetNames()) {
                ui->cbCoefficients->addItem(c);
            }
            ui->cbCoefficients->setEnabled(true);
        } else {
            ui->cbCoefficients->clear();
            ui->cbCoefficients->setEnabled(false);
            ui->start->setEnabled(false);
        }
    });

    connect(ui->cbCoefficients, &QComboBox::currentTextChanged, [=](){
        // no coefficient set selected
        ui->progressCoeff->setValue(0);
        ui->lCoefficientStatus->setText("No coefficients loaded");
        coeffSet = CalDevice::CoefficientSet();
        updateCalibrationStartStatus();

        if(ui->cbCoefficients->currentIndex() > 0) {
            if(!device) {
                qWarning() << "Coefficients selected without connected device";
                return;
            }
            busy = true;
            ui->lCoefficientStatus->setText("Loading coefficients...");
            device->loadCoefficientSets({ui->cbCoefficients->currentText()});
        }
    });

    auto deviceList = USBDevice::GetDevices();
    for(auto device : deviceList) {
        ui->cbDevice->addItem(device);
    }

    connect(ui->start, &QPushButton::clicked, this, &LibreCALDialog::startCalibration);

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

void LibreCALDialog::updateCalibrationStartStatus()
{
    bool canStart = true;
    QString status = "Ready to start";
    if(!device) {
        status = "Not connected to a LibreCAL device.";
        canStart = false;
    }
    set<int> usedCalPorts;
    if(canStart) {
        // Check port mapping for duplicate entries (and at least one used port)
        for(auto port : portAssignment) {
            if(port < 1) {
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
    }
    if(canStart) {
        // at least one port must be used
        if(usedCalPorts.size() == 0) {
            status = "At least one port must be assigned.";
            canStart = false;
        }
    }
    if(canStart) {
        // check if coefficients have been loaded
        if(coeffSet.opens.size() != device->getNumPorts()) {
            status = "Coefficients not loaded";
            canStart = false;
        }
    }
    if(canStart) {
        double coeffMinFreq = numeric_limits<double>::max();
        double coeffMaxFreq = numeric_limits<double>::lowest();

        auto checkCoefficient = [&](CalDevice::CoefficientSet::Coefficient *c) -> bool {
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

        // check if coefficients for all ports are available
        for(auto i : usedCalPorts) {
            // Check if OSL coefficients are there
            if(!checkCoefficient(coeffSet.opens[i-1])) {
                status = "Open coefficient for LibreCAL port "+QString::number(i)+" is missing.";
                canStart = false;
                break;
            }
            if(!checkCoefficient(coeffSet.shorts[i-1])) {
                status = "Short coefficient for LibreCAL port "+QString::number(i)+" is missing.";
                canStart = false;
                break;
            }
            if(!checkCoefficient(coeffSet.loads[i-1])) {
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
    }

    ui->lCalibrationStatus->setText(status);
    ui->start->setEnabled(canStart);
    if(canStart) {
        ui->lCalibrationStatus->setStyleSheet("QLabel { color : black; }");
    } else {
        ui->lCalibrationStatus->setStyleSheet("QLabel { color : red; }");
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
        ui->lDeviceStatus->setStyleSheet("QLabel { color : black; }");
    } else {
        ui->lDeviceStatus->setText("Heating up, please wait with calibration");
        ui->lDeviceStatus->setStyleSheet("QLabel { color : orange; }");
    }
}

void LibreCALDialog::startCalibration()
{
    disableUI();

    ui->progressCal->setValue(0);
    ui->lCalibrationStatus->setText("Creating calibration kit from coefficients...");
    ui->lCalibrationStatus->setStyleSheet("QLabel { color : black; }");
    auto& kit = cal->getKit();
    kit.clearStandards();
    kit.manufacturer = "LibreCAL ("+coeffSet.name+")";
    kit.serialnumber = device->serial();
    kit.description = "Automatically created from LibreCAL module";
    std::vector<CalStandard::Virtual*> openStandards;
    std::vector<CalStandard::Virtual*> shortStandards;
    std::vector<CalStandard::Virtual*> loadStandards;
    std::vector<CalStandard::Virtual*> throughStandards;
    for(unsigned int i=1;i<=device->getNumPorts();i++) {
        if(coeffSet.opens[i-1]->t.points() > 0) {
            auto o = new CalStandard::Open();
            o->setName("Port "+QString::number(i));
            o->setMeasurement(coeffSet.opens[i-1]->t);
            openStandards.push_back(o);
            kit.addStandard(o);
        }
        if(coeffSet.shorts[i-1]->t.points() > 0) {
            auto o = new CalStandard::Short();
            o->setName("Port "+QString::number(i));
            o->setMeasurement(coeffSet.shorts[i-1]->t);
            shortStandards.push_back(o);
            kit.addStandard(o);
        }
        if(coeffSet.loads[i-1]->t.points() > 0) {
            auto o = new CalStandard::Load();
            o->setName("Port "+QString::number(i));
            o->setMeasurement(coeffSet.loads[i-1]->t);
            loadStandards.push_back(o);
            kit.addStandard(o);
        }
        for(unsigned int j=i+1;j<=device->getNumPorts();j++) {
            auto c = coeffSet.getThrough(i,j);
            if(!c) {
                continue;
            }
            if(c->t.points() > 0) {
                auto o = new CalStandard::Through();
                o->setName("Port "+QString::number(i)+" to "+QString::number(j));
                o->setMeasurement(c->t);
                throughStandards.push_back(o);
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
        open->setStandard(openStandards[portAssignment[p]-1]);
        openMeasurements.insert(open);
        cal->measurements.push_back(open);

        auto _short = new CalibrationMeasurement::Short(cal);
        _short->setPort(p+1);
        _short->setStandard(shortStandards[portAssignment[p]-1]);
        shortMeasurements.insert(_short);
        cal->measurements.push_back(_short);

        auto load = new CalibrationMeasurement::Load(cal);
        load->setPort(p+1);
        load->setStandard(loadStandards[portAssignment[p]-1]);
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
                if(ts->getName() == forwardName) {
                    through->setStandard(ts);
                    through->setReverseStandard(false);
                } else if(ts->getName() == reverseName) {
                    through->setStandard(ts);
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
    connect(cal, &Calibration::measurementsUpdated, this, startNextCalibrationStep);
    connect(cal, &Calibration::measurementsAborted, this, [=](){
        enableUI();
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
    QStringList choices = {"Unused"};
    for(int i=1;i<=calPorts;i++) {
        choices.push_back("Port "+QString::number(i));
    }
    for(unsigned int p = 1;p<=vnaPorts;p++) {
        auto label = new QLabel("Port "+QString::number(p)+":");
        auto comboBox = new QComboBox();
        comboBox->addItems(choices);
        connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
            portAssignment[p-1] = comboBox->currentIndex();
            emit portAssignmentChanged();
        });
        // try to set the default
        if(comboBox->count() > (int) p) {
            comboBox->setCurrentIndex(p);
        } else {
            // port not available, set to unused
            comboBox->setCurrentIndex(0);
        }
        layout->addRow(label, comboBox);
        portAssignmentComboBoxes.push_back(comboBox);
    }
}
