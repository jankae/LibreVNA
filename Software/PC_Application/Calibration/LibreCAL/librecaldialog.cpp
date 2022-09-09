#include "librecaldialog.h"
#include "ui_librecaldialog.h"

#include "caldevice.h"
#include "usbdevice.h"
#include "Device/virtualdevice.h"

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

    createPortAssignmentUI();

    connect(this, &LibreCALDialog::portAssignmentChanged, this, &LibreCALDialog::updateCalibrationStartStatus);

    connect(ui->cbDevice, &QComboBox::currentTextChanged, [=](QString text) {
        if(device) {
            delete device;
        }
        device = new CalDevice(text);
        if(device) {
            createPortAssignmentUI();
            connect(device, &CalDevice::updateCoefficientsPercent, ui->progressCoeff, &QProgressBar::setValue);
            connect(device, &CalDevice::updateCoefficientsDone, [=](bool success){
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
            });

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

    connect(this, &QDialog::finished, [=](){
        delete device;
        device = nullptr;
    });

    connect(ui->start, &QPushButton::clicked, this, &LibreCALDialog::startCalibration);

    updateCalibrationStartStatus();
    updateDeviceStatus();
    connect(&updateTimer, &QTimer::timeout, this, &LibreCALDialog::updateDeviceStatus);
    updateTimer.start(1000);
}

LibreCALDialog::~LibreCALDialog()
{
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
    ui->cbDevice->setEnabled(false);
    ui->cbCoefficients->setEnabled(false);
    ui->start->setEnabled(false);

    ui->lCalibrationStatus->setText("Creating calibration kit from coefficients...");
    auto& kit = cal->getKit();
    kit = Calkit::fromLibreCAL(device, coeffSet);
}

void LibreCALDialog::createPortAssignmentUI()
{
    auto layout = static_cast<QFormLayout*>(ui->assignmentBox->layout());
    // Clear any possible previous elements
    portAssignment.clear();
    while(layout->rowCount() > 1) {
        layout->removeRow(1);
    }
    auto vnaPorts = VirtualDevice::getInfo(VirtualDevice::getConnected()).ports;
    portAssignment.resize(vnaPorts, 0);
    auto calPorts = 0;
    if(device) {
        calPorts = device->getNumPorts();
    }
    QStringList choices = {"Unused"};
    for(int i=1;i<=calPorts;i++) {
        choices.push_back("Port "+QString::number(i));
    }
    for(int p = 1;p<=vnaPorts;p++) {
        auto label = new QLabel("Port "+QString::number(p)+":");
        auto comboBox = new QComboBox();
        comboBox->addItems(choices);
        connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
            portAssignment[p-1] = comboBox->currentIndex();
            emit portAssignmentChanged();
        });
        // try to set the default
        comboBox->setCurrentIndex(p);
        layout->addRow(label, comboBox);
    }
}
