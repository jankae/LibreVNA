#include "calibration.h"
#include "ui_calibrationdialogui.h"
#include "CustomWidgets/informationbox.h"
#include "Util/app_common.h"
#include "unit.h"
#include "Util/util.h"
#include "LibreCAL/librecaldialog.h"
#include "preferences.h"
#include "Traces/sparamtraceselectordialog.h"

#include "Tools/Eigen/Dense"

#include <fstream>
#include <iomanip>

#include <QDialog>
#include <QMenu>
#include <QStyle>
#include <QDebug>
#include <QFileDialog>

using namespace std;
using Eigen::MatrixXcd;

bool operator==(const Calibration::CalType &lhs, const Calibration::CalType &rhs) {
    if(lhs.type != rhs.type) {
        return false;
    }
    if(lhs.usedPorts.size() != rhs.usedPorts.size()) {
        return false;
    }
    for(unsigned int i=0;i<lhs.usedPorts.size();i++) {
        if(lhs.usedPorts[i] != rhs.usedPorts[i]) {
            return false;
        }
    }
    // all fields are equal
    return true;
}

Calibration::Calibration()
    : SCPINode("CALibration")
{
    caltype.type = Type::None;
    unsavedChanges = false;

    // create SCPI commands
    add(new SCPICommand("ACTivate", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            auto availableCals = getAvailableCalibrations();
            for(auto caltype : availableCals) {
                if(caltype.getShortString().compare(params[0], Qt::CaseInsensitive) == 0) {
                    // found a match
                    // check if calibration can be activated
                    if(canCompute(caltype)) {
                        compute(caltype);
                        return SCPI::getResultName(SCPI::Result::Empty);
                    } else {
                        return SCPI::getResultName(SCPI::Result::Error);
                    }
                }
            }
            // if we get here, the supplied parameter did not match any of the available calibrations
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        auto availableCals = getAvailableCalibrations();
        if(availableCals.size() == 0) {
            return SCPI::getResultName(SCPI::Result::Empty);
        }
        auto ret = availableCals[0].getShortString();
        for(unsigned int i=1;i<availableCals.size();i++) {
            ret += ",";
            ret += availableCals[i].getShortString();
        }
        return ret;
    }));
    add(new SCPICommand("ACTIVE", nullptr, [=](QStringList) -> QString {
        return caltype.getShortString();
    }));
    add(new SCPICommand("NUMber", nullptr, [=](QStringList) -> QString {
        return QString::number(measurements.size());
    }));
    add(new SCPICommand("RESET", [=](QStringList) -> QString {
        reset();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("ADD", [=](QStringList params) -> QString {
        if(params.size() < 1) {
            // no measurement type specified
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            // parse measurement type
            auto type = CalibrationMeasurement::Base::TypeFromString(params[0]);
            auto newMeas = newMeasurement(type);
            if(!newMeas) {
                // failed to create this type of measurement
                return SCPI::getResultName(SCPI::Result::Error);
            }
            if(params.size() == 2) {
                // standard name given
                CalStandard::Virtual *standard = nullptr;
                for(auto s : newMeas->supportedStandards()) {
                    if(s->getName().compare(params[1], Qt::CaseInsensitive) == 0) {
                        // use this standard
                        standard = s;
                    }
                }
                if(!standard) {
                    // specified standard not available
                    return SCPI::getResultName(SCPI::Result::Error);
                }
                newMeas->setStandard(standard);
            }
            measurements.push_back(newMeas);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, nullptr));
    add(new SCPICommand("TYPE", nullptr, [=](QStringList params) -> QString {
        if(params.size() < 1) {
            // no measurement number specified
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            bool okay;
            unsigned int number = params[0].toUInt(&okay);
            if(!okay || number >= measurements.size()) {
                // invalid measurement specified
                return SCPI::getResultName(SCPI::Result::Error);
            }
            return CalibrationMeasurement::Base::TypeToString(measurements[number]->getType());
        }
    }));
    add(new SCPICommand("PORT", [=](QStringList params) -> QString {
        if(params.size() < 1) {
            // invalid number of parameters
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            bool okay;
            unsigned int number = params[0].toUInt(&okay);
            if(!okay || number >= measurements.size()) {
                // invalid measurement specified
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto meas = measurements[number];
            auto onePort = dynamic_cast<CalibrationMeasurement::OnePort*>(meas);
            if(onePort) {
                if(params.size() != 2) {
                    // invalid number of parameters
                    return SCPI::getResultName(SCPI::Result::Error);
                }
                bool okay;
                unsigned int number = params[1].toInt(&okay);
                if(!okay || number < 1 || number > DeviceDriver::getInfo(DeviceDriver::getActiveDriver()).Limits.VNA.ports) {
                    // invalid port specified
                    return SCPI::getResultName(SCPI::Result::Error);
                }
                onePort->setPort(number);
                return SCPI::getResultName(SCPI::Result::Empty);
            }
            auto twoPort = dynamic_cast<CalibrationMeasurement::TwoPort*>(meas);
            if(twoPort) {
                if(params.size() != 3) {
                    // invalid number of parameters
                    return SCPI::getResultName(SCPI::Result::Error);
                }
                bool okay1;
                unsigned int port1 = params[1].toInt(&okay1);
                bool okay2;
                unsigned int port2 = params[2].toInt(&okay2);
                if(!okay1 || !okay2 || port1 < 1 || port2 > DeviceDriver::getInfo(DeviceDriver::getActiveDriver()).Limits.VNA.ports
                         || port2 < 1 || port2 > DeviceDriver::getInfo(DeviceDriver::getActiveDriver()).Limits.VNA.ports) {
                    // invalid port specified
                    return SCPI::getResultName(SCPI::Result::Error);
                }
                twoPort->setPort1(port1);
                twoPort->setPort2(port2);
                return SCPI::getResultName(SCPI::Result::Empty);
            }
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }, [=](QStringList params) -> QString {
        if(params.size() != 1) {
            // invalid number of parameters
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            bool okay;
            unsigned int number = params[0].toUInt(&okay);
            if(!okay || number >= measurements.size()) {
                // invalid measurement specified
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto meas = measurements[number];
            auto onePort = dynamic_cast<CalibrationMeasurement::OnePort*>(meas);
            if(onePort) {
                return QString::number(onePort->getPort());
            }
            auto twoPort = dynamic_cast<CalibrationMeasurement::TwoPort*>(meas);
            if(twoPort) {
                return QString::number(twoPort->getPort1()) + " " + QString::number(twoPort->getPort2());
            }
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    add(new SCPICommand("STANDARD", [=](QStringList params) -> QString {
        if(params.size() != 2) {
            // invalid number of parameters
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            bool okay;
            unsigned int number = params[0].toUInt(&okay);
            if(!okay || number >= measurements.size()) {
                // invalid measurement specified
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto meas = measurements[number];
            for(auto s : meas->supportedStandards()) {
                if(s->getName().compare(params[1], Qt::CaseInsensitive) == 0) {
                    // use this standard
                    meas->setStandard(s);
                    return SCPI::getResultName(SCPI::Result::Empty);
                }
            }
            // specified standard not available
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }, [=](QStringList params) -> QString {
        if(params.size() != 1) {
            // invalid number of parameters
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            bool okay;
            unsigned int number = params[0].toUInt(&okay);
            if(!okay || number >= measurements.size()) {
                // invalid measurement specified
                return SCPI::getResultName(SCPI::Result::Error);
            }
            auto s = measurements[number]->getStandard();
            if(s) {
                return s->getName();
            } else {
                // no standard set
                return "None";
            }
        }
    }));
    add(new SCPICommand("MEASure", [=](QStringList params) -> QString {
        if(params.size() < 1 ) {
            // no measurement specified
            // TODO check if measurement is already running
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            // assemble list of measurements to take
            std::set<CalibrationMeasurement::Base*> m;
            for(QString p : params) {
                bool okay = false;
                unsigned int number = p.toUInt(&okay);
                if(!okay || number >= measurements.size()) {
                    // invalid measurement specified
                    return SCPI::getResultName(SCPI::Result::Error);
                } else {
                    m.insert(measurements[number]);
                }
            }
            if(CalibrationMeasurement::Base::canMeasureSimultaneously(m)) {
                emit startMeasurements(m);
                return SCPI::getResultName(SCPI::Result::Empty);
            } else {
                // can't start these measurements simultaneously
                return SCPI::getResultName(SCPI::Result::Error);
            }
        }
    }, nullptr));
    add(new SCPICommand("SAVE", [=](QStringList params) -> QString {
        if(params.size() != 1 || getCaltype().type == Calibration::Type::None) {
            // no filename given or no calibration active
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(!toFile(params[0])) {
            // some error when writing the calibration file
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr, false));
    add(new SCPICommand("LOAD", nullptr, [=](QStringList params) -> QString {
        if(params.size() != 1) {
            // no filename given or no calibration active
            return SCPI::getResultName(SCPI::Result::False);
        }
        if(!fromFile(params[0])) {
            // some error when loading the calibration file
            return SCPI::getResultName(SCPI::Result::False);
        }
        return SCPI::getResultName(SCPI::Result::True);
    }, false));
    add(&kit);
}

QString Calibration::TypeToString(Calibration::Type type)
{
    switch(type) {
    case Type::None: return "None";
    case Type::OSL: return "OSL";
    case Type::SOLT: return "SOLT";
    case Type::SOLTwithoutRxMatch: return "SOLTwithoutRxMatch";
    case Type::ThroughNormalization: return "ThroughNormalization";
    case Type::TRL: return "TRL";
    case Type::Last: return "Invalid";
    }
    return "";
}

Calibration::Type Calibration::TypeFromString(QString s)
{
    for(int i=0;i<(int) Type::Last;i++) {
        if(TypeToString((Type) i) == s) {
            return (Type) i;
        }
    }
    return Type::None;
}

void Calibration::correctMeasurement(DeviceDriver::VNAMeasurement &d)
{
    lock_guard<recursive_mutex> guard(access);
    if(caltype.type == Type::None) {
        // no calibration active, nothing to do
        return;
    }
    // formulas from "Multi-Port Calibration Techniques for Differential Parameter Measurements with Network Analyzers", variable names also losely follow this document
    MatrixXcd S(caltype.usedPorts.size(), caltype.usedPorts.size());
    MatrixXcd a(caltype.usedPorts.size(), caltype.usedPorts.size());
    MatrixXcd b(caltype.usedPorts.size(), caltype.usedPorts.size());

    // gab point and interpolate
    Point p;
    if(d.frequency <= points.front().frequency) {
        p = points.front();
    } else if(d.frequency >= points.back().frequency) {
        p = points.back();
    } else {
        // needs to interpolate
        auto lower = lower_bound(points.begin(), points.end(), d.frequency, [](const Point &lhs, double rhs) -> bool {
            return lhs.frequency < rhs;
        });
        auto highPoint = *lower;
        auto lowPoint = *prev(lower);
        double alpha = (d.frequency - lowPoint.frequency) / (highPoint.frequency - lowPoint.frequency);

        p = lowPoint.interpolate(highPoint, alpha);
    }

    // Grab measurements (easier to access by index later)
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            auto pSrc = caltype.usedPorts[i];
            auto pRcv = caltype.usedPorts[j];
            auto name = "S"+QString::number(pRcv)+QString::number(pSrc);
            if(d.measurements.count(name) == 0) {
                qWarning() << "Missing measurement for calibration:" << name;
                S(j,i) = 0.0;
            } else {
                // grab measurement and remove isolation here
                S(j,i) = d.measurements[name];
                if(j != i) {
                    S(j,i) -= p.I[i][j];
                }
            }
        }
    }

    // assemble a (L) and b (K) matrices
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            if(i == j) {
                // calculate incident and reflected wave at the exciting port
                a(j,i) = 1.0 + p.S[i]/p.R[i]*(S(j,i) - p.D[i]*1.0);
                b(j,i) = (1.0 / p.R[i]) * (S(j,i) - p.D[i]*1.0);
            } else {
                // calculate incident and reflected wave at the receiving port
                a(j,i) = p.L[i][j]*S(j,i) / p.T[i][j];
                b(j,i) = S(j,i) / p.T[i][j];
            }
        }
    }
    S = b * a.inverse();

    // extract measurement from matrix and store back into VNAMeasurement
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            auto pSrc = caltype.usedPorts[i];
            auto pRcv = caltype.usedPorts[j];
            auto name = "S"+QString::number(pRcv)+QString::number(pSrc);
            d.measurements[name] = S(j,i);
        }
    }
}

void Calibration::correctTraces(std::map<QString, Trace *> traceSet)
{
    auto points = Trace::assembleDatapoints(traceSet);
    if(points.size()) {
        // succeeded in assembling datapoints
        for(auto &p : points) {
            correctMeasurement(p);
        }
        Trace::fillFromDatapoints(traceSet, points);
    }
}

void Calibration::edit(TraceModel *traceModel)
{
    auto d = new QDialog();
    d->setAttribute(Qt::WA_DeleteOnClose);
    auto ui = new Ui::CalibrationDialog;
    ui->setupUi(d);

    ui->calMinFreq->setUnit("Hz");
    ui->calMinFreq->setPrecision(4);
    ui->calMinFreq->setPrefixes(" kMG");
    ui->calMaxFreq->setUnit("Hz");
    ui->calMaxFreq->setPrecision(4);
    ui->calMaxFreq->setPrefixes(" kMG");

    // generate all possible calibration with the connected device
    vector<CalType> availableCals = getAvailableCalibrations();

    for(auto c : availableCals) {
        ui->calibrationList->addItem(c.getReadableDescription());
    }

    auto updateCalStatistics = [=](){
        lock_guard<recursive_mutex> guard(access);
        ui->activeCalibration->setText(caltype.getReadableDescription());
        ui->calPoints->setValue(points.size());
        if(points.size() > 0) {
            ui->calMinFreq->setValue(points.front().frequency);
            ui->calMaxFreq->setValue(points.back().frequency);
        } else {
            ui->calMinFreq->setValue(0);
            ui->calMaxFreq->setValue(0);
        }
    };

    auto updateCalButtons = [=](){
        lock_guard<recursive_mutex> guard(access);
        auto row = ui->calibrationList->currentRow();
        if(row < 0) {
            ui->activate->setEnabled(false);
            ui->deactivate->setEnabled(false);
        } else {
            if(caltype == availableCals[row]) {
                ui->deactivate->setEnabled(true);
                ui->activate->setEnabled(false);
            } else {
                ui->deactivate->setEnabled(false);
                ui->activate->setEnabled(canCompute(availableCals[row]));
            }
        }
    };

    auto updateCalibrationList = [=](){
        auto style = QApplication::style();
        for(unsigned int i=0;i<availableCals.size();i++) {
            QIcon icon;
            if(canCompute(availableCals[i])) {
                icon = style->standardIcon(QStyle::SP_DialogApplyButton);
            } else {
                icon = style->standardIcon(QStyle::SP_MessageBoxCritical);
            }
            ui->calibrationList->item(i)->setIcon(icon);
        }
        updateCalButtons();
    };

    updateCalibrationList();
    updateCalStatistics();
    updateCalButtons();

    connect(ui->calibrationList, &QListWidget::doubleClicked, [=](const QModelIndex&) {
        ui->activate->clicked();
    });

    connect(ui->calibrationList, &QListWidget::currentRowChanged, [=](){
        updateCalButtons();
    });

    connect(this, &Calibration::activated, d, [=](){
        updateCalibrationList();
        updateCalStatistics();
        updateCalButtons();
    });

    connect(this, &Calibration::deactivated, d, [=](){
        updateCalibrationList();
        updateCalStatistics();
        updateCalButtons();
    });

    connect(ui->activate, &QPushButton::clicked, [=](){
        auto cal = availableCals[ui->calibrationList->currentRow()];
        compute(cal);
    });

    connect(ui->deactivate, &QPushButton::clicked, this, &Calibration::deactivate);

    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto updateTableEditButtons = [=](){
        ui->bDelete->setEnabled(ui->table->currentRow() >= 0);
        ui->bMoveUp->setEnabled(ui->table->currentRow() >= 1);
        ui->bMoveDown->setEnabled(ui->table->currentRow() >= 0 && ui->table->currentRow() < ui->table->rowCount() - 1);
        auto selected = ui->table->selectionModel()->selectedRows();
        ui->measure->setEnabled(selected.size() > 0);
        ui->selectMeasurement->setEnabled(traceModel && selected.size() == 1);
        ui->clearMeasurement->setEnabled(selected.size() > 0);
    };

    auto updateMeasurementTable = [=](){
        int row = ui->table->currentRow();
        ui->table->clear();
        ui->table->setColumnCount(5);
        ui->table->setHorizontalHeaderItem(0, new QTableWidgetItem("Type"));
        ui->table->setHorizontalHeaderItem(1, new QTableWidgetItem("Calkit Standard"));
        ui->table->setHorizontalHeaderItem(2, new QTableWidgetItem("Settings"));
        ui->table->setHorizontalHeaderItem(3, new QTableWidgetItem("Statistics"));
        ui->table->setHorizontalHeaderItem(4, new QTableWidgetItem("Timestamp"));
        ui->table->setRowCount(measurements.size());
        for(unsigned int i=0;i<measurements.size();i++){
            ui->table->setItem(i, 0, new QTableWidgetItem(CalibrationMeasurement::Base::TypeToString(measurements[i]->getType())));
            ui->table->setCellWidget(i, 1, measurements[i]->createStandardWidget());
            ui->table->setCellWidget(i, 2, measurements[i]->createSettingsWidget());
            ui->table->setItem(i, 3, measurements[i]->getStatisticsItem());
            ui->table->setItem(i, 4, new QTableWidgetItem(measurements[i]->getTimestamp().toString()));
        }
        ui->table->selectRow(row);
        updateTableEditButtons();
    };

    ui->createDefault->addItem(" ");
    for(unsigned int i=0;i<(int) DefaultMeasurements::Last;i++) {
        ui->createDefault->addItem(DefaultMeasurementsToString((DefaultMeasurements) i));
    }

    QObject::connect(ui->createDefault, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        if(measurements.size() > 0) {
            if(!InformationBox::AskQuestion("Create default entries?", "Do you want to remove all existing entries and create default calibration measurements instead?", true)) {
                // user aborted
                ui->createDefault->blockSignals(true);
                ui->createDefault->setCurrentIndex(0);
                ui->createDefault->blockSignals(false);
                return;
            }
            deleteMeasurements();
        }
        createDefaultMeasurements((DefaultMeasurements) (ui->createDefault->currentIndex() - 1));
        updateMeasurementTable();
        updateCalibrationList();
        ui->createDefault->blockSignals(true);
        ui->createDefault->setCurrentIndex(0);
        ui->createDefault->blockSignals(false);
    });

    QObject::connect(ui->bDelete, &QPushButton::clicked, [=](){
        auto selected = ui->table->selectionModel()->selectedRows();
        set<CalibrationMeasurement::Base*> toDelete;
        for(auto s : selected) {
            toDelete.insert(measurements[s.row()]);
        }
        while(toDelete.size() > 0) {
            for(unsigned int i=0;i<measurements.size();i++) {
                if(toDelete.count(measurements[i])) {
                    // this measurement should be deleted
                    delete measurements[i];
                    toDelete.erase(measurements[i]);
                    measurements.erase(measurements.begin() + i);
                }
            }
        }
        // update calibration (may have changed due to deleted measurement)
        if(canCompute(caltype)) {
            compute(caltype);
        } else {
            deactivate();
        }
        updateMeasurementTable();
        updateCalibrationList();
    });

    QObject::connect(ui->bMoveUp, &QPushButton::clicked, [=](){
        auto row = ui->table->currentRow();
        if(row >= 1) {
            swap(measurements[row], measurements[row-1]);
            ui->table->selectRow(row-1);
            updateMeasurementTable();
        }
    });

    QObject::connect(ui->bMoveDown, &QPushButton::clicked, [=](){
        auto row = ui->table->currentRow();
        if(row >= 0) {
            swap(measurements[row], measurements[row+1]);
            ui->table->selectRow(row+1);
            updateMeasurementTable();
        }
    });

    connect(ui->measure, &QPushButton::clicked, [=](){
        std::set<CalibrationMeasurement::Base*> m;
        auto selected = ui->table->selectionModel()->selectedRows();
        for(auto s : selected) {
            auto meas = measurements[s.row()];
            if(!meas->readyForMeasurement()) {
                InformationBox::ShowError("Unable to measure", CalibrationMeasurement::Base::TypeToString(meas->getType())+" measurement is not ready, please check that a valid calibration standard is selected");
                return;
            }
            m.insert(measurements[s.row()]);
        }
        if(!CalibrationMeasurement::Base::canMeasureSimultaneously(m)) {
            InformationBox::ShowError("Unable to measure", "Different selected measurements require the same port, unable to perform measurement");
            return;
        }
        emit startMeasurements(m);
    });

    connect(ui->selectMeasurement, &QPushButton::clicked, [=](){
        auto selected = ui->table->selectionModel()->selectedRows();
        if(selected.size() != 1) {
            InformationBox::ShowError("Unable to select measurement", "Exactly one measurement must be selected");
            return;
        }
        // figure out which S parameters we need
        auto meas = measurements[selected[0].row()];
        auto ports = meas->getPorts();
        if(ports.size() == 0) {
            InformationBox::ShowError("Unable to select measurement", "Selecting measurements for this type of calibration measurement is not supported");
            return;
        }
        auto selector = new SParamTraceSelectorDialog(*traceModel, ports);
        connect(selector, &SParamTraceSelectorDialog::tracesSelected, d, [=](std::vector<DeviceDriver::VNAMeasurement> traceMeasurements){
            clearMeasurements({meas});
            for(const auto &tm : traceMeasurements) {
                addMeasurements({meas}, tm);
            }
            updateMeasurementTable();
            updateCalibrationList();
        });
        selector->show();
    });
    if(!traceModel) {
        // can not select a measurement if no trace model is supplied
        ui->selectMeasurement->setEnabled(false);
    }

    connect(this, &Calibration::measurementsUpdated, d, [=](){
        updateMeasurementTable();
        updateCalibrationList();
        if(caltype.type != Type::None) {
            // update the calibration with the new measurement
            if(canCompute(caltype)) {
                compute(caltype);
            } else {
                // can not keep the calibration, disabled
                deactivate();
            }
        }
    });

    connect(ui->clearMeasurement, &QPushButton::clicked, [=](){
        auto selected = ui->table->selectionModel()->selectedRows();
        for(auto s : selected) {
            measurements[s.row()]->clearPoints();
        }
        // update calibration (may have changed due to deleted measurement)
        if(canCompute(caltype)) {
            compute(caltype);
        } else {
            deactivate();
        }
        updateMeasurementTable();
        updateCalibrationList();
    });

    connect(ui->eCal, &QPushButton::clicked, [=](){
        auto d = new LibreCALDialog(this);
        d->show();
    });

    connect(ui->editCalkit, &QPushButton::clicked, [=](){
        kit.edit([=](){
            updateMeasurementTable();
            // update calibration (may have changed due to edited calibration standard)
            if(canCompute(caltype)) {
                compute(caltype);
            } else {
                deactivate();
            }
        });
    });

    QObject::connect(ui->table, &QTableWidget::itemSelectionChanged, updateTableEditButtons);

    auto addMenu = new QMenu();
    for(auto t : CalibrationMeasurement::Base::availableTypes()) {
        auto action = new QAction(CalibrationMeasurement::Base::TypeToString(t));
        QObject::connect(action, &QAction::triggered, [=](){
            auto newMeas = newMeasurement(t);
            if(newMeas) {
                measurements.push_back(newMeas);
                updateMeasurementTable();
            }
        });
        addMenu->addAction(action);
    }

    ui->bAdd->setMenu(addMenu);

    updateMeasurementTable();

    d->show();
}

CalibrationMeasurement::Base *Calibration::newMeasurement(CalibrationMeasurement::Base::Type type)
{
    CalibrationMeasurement::Base *m = nullptr;
    switch(type) {
    case CalibrationMeasurement::Base::Type::Open: m = new CalibrationMeasurement::Open(this); break;
    case CalibrationMeasurement::Base::Type::Short: m = new CalibrationMeasurement::Short(this); break;
    case CalibrationMeasurement::Base::Type::Load: m = new CalibrationMeasurement::Load(this); break;
    case CalibrationMeasurement::Base::Type::SlidingLoad: m = new CalibrationMeasurement::SlidingLoad(this); break;
    case CalibrationMeasurement::Base::Type::Reflect: m = new CalibrationMeasurement::Reflect(this); break;
    case CalibrationMeasurement::Base::Type::Through: m = new CalibrationMeasurement::Through(this); break;
    case CalibrationMeasurement::Base::Type::Isolation: m = new CalibrationMeasurement::Isolation(this); break;
    case CalibrationMeasurement::Base::Type::Line: m = new CalibrationMeasurement::Line(this); break;
    case CalibrationMeasurement::Base::Type::Last: break;
    }
    return m;
}

Calibration::Point Calibration::createInitializedPoint(double f) {
    Point point;
    point.frequency = f;
    // resize vectors
    point.D.resize(caltype.usedPorts.size(), 0.0);
    point.R.resize(caltype.usedPorts.size(), 1.0);
    point.S.resize(caltype.usedPorts.size(), 0.0);

    point.L.resize(caltype.usedPorts.size());
    point.T.resize(caltype.usedPorts.size());
    point.I.resize(caltype.usedPorts.size());
    fill(point.L.begin(), point.L.end(), vector<complex<double>>(caltype.usedPorts.size(), 0.0));
    fill(point.T.begin(), point.T.end(), vector<complex<double>>(caltype.usedPorts.size(), 1.0));
    fill(point.I.begin(), point.I.end(), vector<complex<double>>(caltype.usedPorts.size(), 0.0));
    return point;
}

Calibration::Point Calibration::computeOSL(double f)
{
    Point point = createInitializedPoint(f);

    // Calculate SOL coefficients
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        auto p = caltype.usedPorts[i];
        auto _short = static_cast<CalibrationMeasurement::Short*>(findMeasurement(CalibrationMeasurement::Base::Type::Short, p));
        auto open = static_cast<CalibrationMeasurement::Open*>(findMeasurement(CalibrationMeasurement::Base::Type::Open, p));
        auto s_m = _short->getMeasured(f);
        auto o_m = open->getMeasured(f);
        auto s_c = _short->getActual(f);
        auto o_c = open->getActual(f);
        complex<double> l_c, l_m;
        auto slidingMeasurements = findMeasurements(CalibrationMeasurement::Base::Type::SlidingLoad, p);
        if(slidingMeasurements.size() >= 3) {
            // use sliding load
            vector<complex<double>> slidingMeasured;
            for(auto m : slidingMeasurements) {
                auto slidingLoad = static_cast<CalibrationMeasurement::SlidingLoad*>(m);
                auto value = slidingLoad->getMeasured(f);
                if(isnan(abs(value))) {
                    throw runtime_error("missing sliding load measurement");
                }
                slidingMeasured.push_back(value);
            }
            // use center of measurement for ideal load measurement
            l_m = Util::findCenterOfCircle(slidingMeasured);
            // assume perfect sliding load
            l_c = 0.0;
        } else {
            // use normal load standard
            auto load = static_cast<CalibrationMeasurement::Load*>(findMeasurement(CalibrationMeasurement::Base::Type::Load, p));
            l_c = load->getActual(f);
            l_m = load->getMeasured(f);
        }
        auto denom = l_c * o_c * (o_m - l_m) + l_c * s_c * (l_m - s_m) + o_c * s_c * (s_m - o_m);
        point.D[i] = (l_c * o_m * (s_m * (o_c - s_c) + l_m * s_c) - l_c * o_c * l_m * s_m + o_c * l_m * s_c * (s_m - o_m)) / denom;
        point.S[i] = (l_c * (o_m - s_m) + o_c * (s_m - l_m) + s_c * (l_m - o_m)) / denom;
        auto delta = (l_c * l_m * (o_m - s_m) + o_c * o_m * (s_m - l_m) + s_c * s_m * (l_m - o_m)) / denom;
        point.R[i] = point.D[i] * point.S[i] - delta;
    }
    return point;
}

Calibration::Point Calibration::computeSOLT(double f)
{
    Point point = computeOSL(f);

    // calculate forward match and transmission
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            if(i == j) {
                // this is the exciting port, SOL error box used here
                continue;
            }
            auto p1 = caltype.usedPorts[i];
            auto p2 = caltype.usedPorts[j];
            // grab measurement and calkit through definitions
            auto throughForward = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p1, p2));
            auto throughReverse = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p2, p1));
            complex<double> S11, S21;
            Sparam Sideal;
            if(throughForward) {
                S11 = throughForward->getMeasured(f).get(1,1);
                S21 = throughForward->getMeasured(f).get(2,1);
                Sideal = throughForward->getActual(f);
            } else if(throughReverse) {
                S11 = throughReverse->getMeasured(f).get(2,2);
                S21 = throughReverse->getMeasured(f).get(1,2);
                Sideal = throughReverse->getActual(f);
                Sideal.swapPorts(1,2);
            }
            auto isoMeas = static_cast<CalibrationMeasurement::Isolation*>(findMeasurement(CalibrationMeasurement::Base::Type::Isolation));
            auto isolation = complex<double>(0.0,0.0);
            if(isoMeas) {
                isolation = isoMeas->getMeasured(f, p2, p1);
            }
            auto deltaS = Sideal.get(1,1)*Sideal.get(2,2) - Sideal.get(2,1) * Sideal.get(1,2);
            point.L[i][j] = ((S11 - point.D[i])*(1.0 - point.S[i] * Sideal.get(1,1))-Sideal.get(1,1)*point.R[i])
                    / ((S11 - point.D[i])*(Sideal.get(2,2)-point.S[i]*deltaS)-deltaS*point.R[i]);
            point.T[i][j] = (S21 - isolation)*(1.0 - point.S[i]*Sideal.get(1,1) - point.L[i][j]*Sideal.get(2,2) + point.S[i]*point.L[i][j]*deltaS) / Sideal.get(2,1);
            point.I[i][j] = isolation;
        }
    }
    return point;
}

Calibration::Point Calibration::computeSOLTwithoutRxMatch(double f) {
    // This is very similar to SOLT but it assumes that receiver matching at the VNA is perfect.
    // It can be used if the through calibration standard is very lossy which would result in
    // very noisy values for the receiver match
    auto p = computeSOLT(f);
    for(auto &l : p.L) {
        fill(l.begin(), l.end(), 0.0);
    }
    return p;
}

Calibration::Point Calibration::computeThroughNormalization(double f)
{
    Point point = createInitializedPoint(f);

    // Calculate SOL coefficients
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        // use ideal coefficients
        point.D[i] = 0.0;
        point.S[i] = 0.0;
        point.R[i] = 1.0;
    }
    // calculate forward match and transmission
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            if(i == j) {
                // this is the exciting port, SOL error box used here
                continue;
            }
            auto p1 = caltype.usedPorts[i];
            auto p2 = caltype.usedPorts[j];
            // grab measurement and calkit through definitions
            auto throughForward = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p1, p2));
            auto throughReverse = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p2, p1));
            complex<double> S21 = 0.0;
            Sparam Sideal;
            if(throughForward) {
                S21 = throughForward->getMeasured(f).get(2,1);
                Sideal = throughForward->getActual(f);
            } else if(throughReverse) {
                S21 = throughReverse->getMeasured(f).get(1,2);
                Sideal = throughReverse->getActual(f);
                Sideal.swapPorts(1,2);
            }
            point.L[i][j] = 0.0;
            point.T[i][j] = S21 / Sideal.get(2,1);
            point.I[i][j] = 0.0;
        }
    }
    return point;
}

Calibration::Point Calibration::computeTRL(double freq)
{
    Point point = createInitializedPoint(freq);

    // calculate forward match and transmission
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            if(i == j) {
                // calculation only possible with through measurements
                continue;
            }
            auto p1 = caltype.usedPorts[i];
            auto p2 = caltype.usedPorts[j];
            // grab reflection measurements
            auto S11_reflection = static_cast<CalibrationMeasurement::Reflect*>(findMeasurement(CalibrationMeasurement::Base::Type::Reflect, p1));
            auto S22_reflection = static_cast<CalibrationMeasurement::Reflect*>(findMeasurement(CalibrationMeasurement::Base::Type::Reflect, p2));
            bool S11_short, S22_short;
            {
                auto S11_short_standard = dynamic_cast<CalStandard::Short*>(S11_reflection->getStandard());
                auto S11_open_standard = dynamic_cast<CalStandard::Open*>(S11_reflection->getStandard());
                auto S11_reflect_standard = dynamic_cast<CalStandard::Reflect*>(S11_reflection->getStandard());
                if(S11_short_standard) {
                    S11_short = true;
                } else if(S11_open_standard) {
                    S11_short = false;
                } else if(S11_reflect_standard) {
                    S11_short = S11_reflect_standard->getIsShort();
                } else {
                    // invalid standard
                    throw runtime_error("Invalid standard defined for reflection measurement");
                }
                auto S22_short_standard = dynamic_cast<CalStandard::Short*>(S22_reflection->getStandard());
                auto S22_open_standard = dynamic_cast<CalStandard::Open*>(S22_reflection->getStandard());
                auto S22_reflect_standard = dynamic_cast<CalStandard::Reflect*>(S22_reflection->getStandard());
                if(S22_short_standard) {
                    S22_short = true;
                } else if(S22_open_standard) {
                    S22_short = false;
                } else if(S22_reflect_standard) {
                    S22_short = S22_reflect_standard->getIsShort();
                } else {
                    // invalid standard
                    throw runtime_error("Invalid standard defined for reflection measurement");
                }
            }
            bool reflectionIsNegative;
            if(S11_short && S22_short) {
                reflectionIsNegative = true;
            } else if(!S11_short && !S22_short) {
                reflectionIsNegative = false;
            } else {
                throw runtime_error("Reflection measurements must all use the same standard (either open or short)");
            }
            // grab through measurement
            auto throughForward = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p1, p2));
            auto throughReverse = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p2, p1));
            Sparam Sthrough;
            if(throughForward) {
                Sthrough = throughForward->getMeasured(freq);
            } else if(throughReverse) {
                Sthrough = throughReverse->getMeasured(freq);
                Sthrough.swapPorts(1,2);
            }
            // grab line measurement
            auto forwardLines = findMeasurements(CalibrationMeasurement::Base::Type::Line, p1, p2);
            auto reverseLines = findMeasurements(CalibrationMeasurement::Base::Type::Line, p2, p1);
            // find the closest (geometric) match for the current frequency
            double closestIdealFreqRatio = numeric_limits<double>::max();
            bool closestLineIsReversed = false;
            CalibrationMeasurement::Line* closestLine = nullptr;
            CalStandard::Line* closestStandard = nullptr;
            for(int i=0;i<2;i++) {
                auto list = i ? reverseLines : forwardLines;
                for(auto l : list) {
                    auto line = static_cast<CalibrationMeasurement::Line*>(l);
                    auto standard = static_cast<CalStandard::Line*>(line->getStandard());
                    double idealFreq = (standard->minFrequency() + standard->maxFrequency()) / 2;
                    double mismatch = idealFreq / freq;
                    if(mismatch < 0) {
                        mismatch = 1.0 / mismatch;
                    }
                    if(mismatch < closestIdealFreqRatio) {
                        closestIdealFreqRatio = mismatch;
                        closestLineIsReversed = i > 0;
                        closestLine = line;
                        closestStandard = standard;
                    }
                }
            }
            if(!closestLine) {
                throw runtime_error("Unable to find required line measurement");
            }
            if(freq < closestStandard->minFrequency() || freq > closestStandard->maxFrequency()) {
                throw runtime_error("No line standard supports the required frequency ("+QString::number(freq).toStdString()+")");
            }

            Sparam Sline = closestLine->getMeasured(freq);
            if(closestLineIsReversed) {
                Sline.swapPorts(1,2);
            }

            // got all required measurements
            // calculate TRL calibration
            // variable names and formulas according to http://emlab.uiuc.edu/ece451/notes/new_TRL.pdf
            // page 19
            auto R_T = Tparam(Sthrough);
            auto R_D = Tparam(Sline);
            auto T = R_D*R_T.inverse();
            complex<double> a_over_c, b;
            // page 21-22
            Util::solveQuadratic(T.get(2,1), T.get(2,2) - T.get(1,1), -T.get(1,2), b, a_over_c);
            // ensure correct root selection
            // page 23
            if(abs(b) >= abs(a_over_c)) {
                swap(b, a_over_c);
            }
            // page 24
            auto g = R_T.get(2,2);
            auto d = R_T.get(1,1) / g;
            auto e = R_T.get(1,2) / g;
            auto f = R_T.get(2,1) / g;

            // page 25
            auto r22_rho22 = g * (1.0 - e / a_over_c) / (1.0 - b / a_over_c);
            auto gamma = (f - d / a_over_c) / (1.0 - e / a_over_c);
            auto beta_over_alpha = (e - b) / (d - b * f);
            // page 26
            auto alpha_a = (d - b * f) / (1.0 - e / a_over_c);
            auto w1 = S11_reflection->getMeasured(freq);
            auto w2 = S22_reflection->getMeasured(freq);
            // page 28
            auto a = sqrt((w1 - b) / (w2 + gamma) * (1.0 + w2 * beta_over_alpha) / (1.0 - w1 / a_over_c) * alpha_a);
            // page 29, check sign of a
            auto reflection = (w1 - b) / (a * (1.0 - w1 / a_over_c));
            if((reflection.real() > 0 && reflectionIsNegative) || (reflection.real() < 0 && !reflectionIsNegative)) {
                // wrong sign for a
                a = -a;
            }
            // Revert back from error boxes with T parameters to S paramaters,
            // page 17 + formulas for calculating S parameters from T parameters.
            // Forward coefficients, normalize for S21 = 1.0 -> r22 = 1.0
            auto r22 = complex<double>(1.0);
            auto rho22 = r22_rho22 / r22;
            auto alpha = alpha_a / a;
            auto beta = beta_over_alpha * alpha;
            auto c = a / a_over_c;
            auto Box_A = Tparam(r22 * a, r22 * b, r22 * c, r22);
            auto Box_B = Tparam(rho22 * alpha, rho22 * beta, rho22 * gamma, rho22);
            auto S_A = Sparam(Box_A);
            point.D[i] = S_A.get(1,1);
            point.R[i] = S_A.get(1,2);
            point.S[i] = S_A.get(2,2);
            auto S_B = Sparam(Box_B);
            point.L[i][j] = S_B.get(1,1);
            point.T[i][j] = S_B.get(2,1);
            // no isolation measurement available
            point.I[i][j] = 0.0;

            // Reverse coefficients, will be handled in loop iteration where i=j and j=i
        }
    }
    return point;
}

Calibration::CalType Calibration::getCaltype() const
{
    return caltype;
}

Calibration::InterpolationType Calibration::getInterpolation(double f_start, double f_stop, int npoints)
{
    lock_guard<recursive_mutex> guard(access);
    if(!points.size()) {
        return InterpolationType::NoCalibration;
    }
    if(f_start < points.front().frequency || f_stop > points.back().frequency) {
        return InterpolationType::Extrapolate;
    }
    // Either exact or interpolation, check individual frequencies
    uint32_t f_step;
    if(npoints > 1) {
        f_step = (f_stop - f_start) / (npoints - 1);
    } else {
        f_step = f_stop - f_start;
    }
    uint64_t f = f_start;
    do {
        if(find_if(points.begin(), points.end(), [&f](const Point& p){
            return abs(f - p.frequency) < 100;
        }) == points.end()) {
            return InterpolationType::Interpolate;
        }
        f += f_step;
    } while(f <= f_stop && f_step > std::numeric_limits<double>::epsilon());

    // if we get here all frequency points were matched
    if(points.front().frequency == f_start && points.back().frequency == f_stop) {
        return InterpolationType::Unchanged;
    } else {
        return InterpolationType::Exact;
    }
}

std::vector<Trace *> Calibration::getErrorTermTraces()
{
    lock_guard<recursive_mutex> guard(access);
    vector<Trace*> ret;
    if(points.size() == 0) {
        return ret;
    }
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        auto p = caltype.usedPorts[i];
        auto tDir = new Trace("Directivity_Port"+QString::number(p));
        tDir->setReflection(true);
        tDir->setCalibration();
        auto tSM = new Trace("SourceMatch_Port"+QString::number(p));
        tSM->setReflection(true);
        tSM->setCalibration();
        auto tRT = new Trace("ReflectionTracking_Port"+QString::number(p));
        tRT->setReflection(false);
        tRT->setCalibration();
        for(auto p : points) {
            Trace::Data td;
            td.x = p.frequency;
            td.y = p.D[i];
            tDir->addData(td, Trace::DataType::Frequency);
            td.y = p.S[i];
            tSM->addData(td, Trace::DataType::Frequency);
            td.y = p.R[i];
            tRT->addData(td, Trace::DataType::Frequency);
        }
        ret.push_back(tDir);
        ret.push_back(tSM);
        ret.push_back(tRT);
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            if(i==j) {
                continue;
            }
            auto p2 = caltype.usedPorts[j];
            auto tRM = new Trace("ReceiverMatch_"+QString::number(p)+QString::number(p2));
            tRM->setReflection(true);
            tRM->setCalibration();
            auto tTT = new Trace("TransmissionTracking_"+QString::number(p)+QString::number(p2));
            tTT->setReflection(false);
            tTT->setCalibration();
            auto tTI = new Trace("TransmissionIsolation_"+QString::number(p)+QString::number(p2));
            tTI->setReflection(false);
            tTI->setCalibration();
            for(auto p : points) {
                Trace::Data td;
                td.x = p.frequency;
                td.y = p.L[i][j];
                tRM->addData(td, Trace::DataType::Frequency);
                td.y = p.T[i][j];
                tTT->addData(td, Trace::DataType::Frequency);
                td.y = p.I[i][j];
                tTI->addData(td, Trace::DataType::Frequency);
            }
            ret.push_back(tRM);
            ret.push_back(tTT);
            ret.push_back(tTI);
        }
    }
    return ret;
}

std::vector<Trace *> Calibration::getMeasurementTraces()
{
    lock_guard<recursive_mutex> guard(access);
    vector<Trace*> ret;
    for(auto m : measurements) {
        switch(m->getType()) {
        case CalibrationMeasurement::Base::Type::Open:
        case CalibrationMeasurement::Base::Type::Short:
        case CalibrationMeasurement::Base::Type::Load:
        case CalibrationMeasurement::Base::Type::SlidingLoad:
        case CalibrationMeasurement::Base::Type::Reflect: {
            auto onePort = static_cast<CalibrationMeasurement::OnePort*>(m);
            auto t = new Trace(CalibrationMeasurement::Base::TypeToString(onePort->getType())+"_Port"+QString::number(onePort->getPort()));
            t->setCalibration();
            t->setReflection(true);
            for(auto d : onePort->getPoints()) {
                Trace::Data td;
                td.x = d.frequency;
                td.y = d.S;
                t->addData(td, Trace::DataType::Frequency);
            }
            ret.push_back(t);
        }
            break;
        case CalibrationMeasurement::Base::Type::Through:
        case CalibrationMeasurement::Base::Type::Line: {
            auto twoPort = static_cast<CalibrationMeasurement::TwoPort*>(m);
            auto ts11 = new Trace(CalibrationMeasurement::Base::TypeToString(twoPort->getType())+"_Port"+QString::number(twoPort->getPort1())+QString::number(twoPort->getPort2())+"_S11");
            auto ts12 = new Trace(CalibrationMeasurement::Base::TypeToString(twoPort->getType())+"_Port"+QString::number(twoPort->getPort1())+QString::number(twoPort->getPort2())+"_S12");
            auto ts21 = new Trace(CalibrationMeasurement::Base::TypeToString(twoPort->getType())+"_Port"+QString::number(twoPort->getPort1())+QString::number(twoPort->getPort2())+"_S21");
            auto ts22 = new Trace(CalibrationMeasurement::Base::TypeToString(twoPort->getType())+"_Port"+QString::number(twoPort->getPort1())+QString::number(twoPort->getPort2())+"_S22");
            ts11->setCalibration();
            ts11->setReflection(true);
            ts12->setCalibration();
            ts12->setReflection(false);
            ts21->setCalibration();
            ts21->setReflection(false);
            ts22->setCalibration();
            ts22->setReflection(true);
            for(auto d : twoPort->getPoints()) {
                Trace::Data td;
                td.x = d.frequency;
                td.y = d.S.get(1,1);
                ts11->addData(td, Trace::DataType::Frequency);
                td.y = d.S.get(1,2);
                ts12->addData(td, Trace::DataType::Frequency);
                td.y = d.S.get(2,1);
                ts21->addData(td, Trace::DataType::Frequency);
                td.y = d.S.get(2,2);
                ts22->addData(td, Trace::DataType::Frequency);
            }
            ret.push_back(ts11);
            ret.push_back(ts12);
            ret.push_back(ts21);
            ret.push_back(ts22);
        }
            break;
        case CalibrationMeasurement::Base::Type::Isolation: {
            auto iso = static_cast<CalibrationMeasurement::Isolation*>(m);
            int ports = iso->getPoints()[0].S.size();
            // Create the traces
            vector<vector<Trace*>> traces;
            traces.resize(ports);
            for(int i=0;i<ports;i++) {
                for(int j=0;j<ports;j++) {
                    auto t = new Trace(CalibrationMeasurement::Base::TypeToString(iso->getType())+"_S"+QString::number(i+1)+QString::number(j+1));
                    t->setCalibration();
                    t->setReflection(i==j);
                    traces[i].push_back(t);
                    // also add to main return vector
                    ret.push_back(t);
                }
            }
            // Fill the traces
            for(auto p : iso->getPoints()) {
                Trace::Data td;
                td.x = p.frequency;
                for(unsigned int i=0;i<p.S.size();i++) {
                    for(unsigned int j=0;j<p.S[i].size();j++) {
                        td.y = p.S[i][j];
                        traces[i][j]->addData(td, Trace::DataType::Frequency);
                    }
                }
            }
        }
            break;
        case CalibrationMeasurement::Base::Type::Last:
            break;
        }
    }
    return ret;
}

QString Calibration::getCurrentCalibrationFile()
{
    return currentCalFile;
}

double Calibration::getMinFreq()
{
    if(points.size() > 0) {
        return points.front().frequency;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

double Calibration::getMaxFreq()
{
    if(points.size() > 0) {
        return points.back().frequency;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

int Calibration::getNumPoints()
{
    return points.size();
}

QString Calibration::descriptiveCalName()
{
    if(points.size() == 0) {
        return QString();
    }
    int precision = 3;
    QString lo = Unit::ToString(points.front().frequency, "", " kMG", precision);
    QString hi = Unit::ToString(points.back().frequency, "", " kMG", precision);
    // due to rounding up 123.66M and 123.99M -> we get lo="124M" and hi="124M"
    // so let's add some precision
    if (lo == hi) {
        // Only in case of 123.66M and 123.69M we would need 5 digits, but that kind of narrow cal. is very unlikely.
        precision = 4;
        lo = Unit::ToString(points.front().frequency, "", " kMG", precision);
        hi = Unit::ToString(points.back().frequency, "", " kMG", precision);
    }

    QString tmp =
            caltype.getShortString()
            + "_"
            + lo + "-" + hi
            + "_"
            + QString::number(this->points.size()) + "pt";
    tmp = tmp.replace(" ", "_");
    tmp = tmp.replace("[", "");
    tmp = tmp.replace("]", "");
    tmp = tmp.replace(".", "_");
    tmp = tmp.replace(",", "_");
    return tmp;
}

QString Calibration::getValidDevice() const
{
    return validDevice;
}

bool Calibration::validForDevice(QString serial) const
{
    if(validDevice.isEmpty()) {
        // no device indicated, always assume that the calibration is valid
        return true;
    }
    if(validDevice == serial) {
        return true;
    } else {
        return false;
    }
}

bool Calibration::hasDirectivity(unsigned int port)
{
    unsigned int index = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), port) - caltype.usedPorts.begin();
    if(points.size() == 0 || index >= caltype.usedPorts.size()) {
        // no calibration or it does not contain this port
        return false;
    }
    auto def = std::complex<double>(0.0, 0.0);
    for(const auto &p : points) {
        if(p.D[index] != def) {
            // at least one point does not match the default value -> we have a valid calibration for this
            return true;
        }
    }
    // all points still at default value
    return false;
}

bool Calibration::hasReflectionTracking(unsigned int port)
{
    unsigned int index = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), port) - caltype.usedPorts.begin();
    if(points.size() == 0 || index >= caltype.usedPorts.size()) {
        // no calibration or it does not contain this port
        return false;
    }
    auto def = std::complex<double>(1.0, 0.0);
    for(const auto &p : points) {
        if(p.R[index] != def) {
            // at least one point does not match the default value -> we have a valid calibration for this
            return true;
        }
    }
    // all points still at default value
    return false;
}

bool Calibration::hasSourceMatch(unsigned int port)
{
    unsigned int index = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), port) - caltype.usedPorts.begin();
    if(points.size() == 0 || index >= caltype.usedPorts.size()) {
        // no calibration or it does not contain this port
        return false;
    }
    auto def = std::complex<double>(0.0, 0.0);
    for(const auto &p : points) {
        if(p.S[index] != def) {
            // at least one point does not match the default value -> we have a valid calibration for this
            return true;
        }
    }
    // all points still at default value
    return false;
}

bool Calibration::hasReceiverMatch(unsigned int sourcePort, unsigned int receivePort)
{
    unsigned int indexSrc = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), sourcePort) - caltype.usedPorts.begin();
    unsigned int indexRcv = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), receivePort) - caltype.usedPorts.begin();
    if(points.size() == 0 || indexSrc >= caltype.usedPorts.size() || indexRcv >= caltype.usedPorts.size()) {
        // no calibration or it does not contain this port
        return false;
    }
    auto def = std::complex<double>(0.0, 0.0);
    for(const auto &p : points) {
        if(p.L[indexSrc][indexRcv] != def) {
            // at least one point does not match the default value -> we have a valid calibration for this
            return true;
        }
    }
    // all points still at default value
    return false;
}

bool Calibration::hasTransmissionTracking(unsigned int sourcePort, unsigned int receivePort)
{
    unsigned int indexSrc = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), sourcePort) - caltype.usedPorts.begin();
    unsigned int indexRcv = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), receivePort) - caltype.usedPorts.begin();
    if(points.size() == 0 || indexSrc >= caltype.usedPorts.size() || indexRcv >= caltype.usedPorts.size()) {
        // no calibration or it does not contain this port
        return false;
    }
    auto def = std::complex<double>(1.0, 0.0);
    for(const auto &p : points) {
        if(p.T[indexSrc][indexRcv] != def) {
            // at least one point does not match the default value -> we have a valid calibration for this
            return true;
        }
    }
    // all points still at default value
    return false;
}

bool Calibration::hasIsolation(unsigned int sourcePort, unsigned int receivePort)
{
    unsigned int indexSrc = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), sourcePort) - caltype.usedPorts.begin();
    unsigned int indexRcv = std::find(caltype.usedPorts.begin(), caltype.usedPorts.end(), receivePort) - caltype.usedPorts.begin();
    if(points.size() == 0 || indexSrc >= caltype.usedPorts.size() || indexRcv >= caltype.usedPorts.size()) {
        // no calibration or it does not contain this port
        return false;
    }
    auto def = std::complex<double>(0.0, 0.0);
    for(const auto &p : points) {
        if(p.I[indexSrc][indexRcv] != def) {
            // at least one point does not match the default value -> we have a valid calibration for this
            return true;
        }
    }
    // all points still at default value
    return false;
}

bool Calibration::hasUnsavedChanges() const
{
    return unsavedChanges;
}

Calkit &Calibration::getKit()
{
    return kit;
}

nlohmann::json Calibration::toJSON()
{
    lock_guard<recursive_mutex> guard(access);
    nlohmann::json j;
    j["format"] = 3;
    nlohmann::json jmeasurements;
    for(auto m : measurements) {
        nlohmann::json jmeas;
        jmeas["type"] = CalibrationMeasurement::Base::TypeToString(m->getType()).toStdString();
        jmeas["data"] = m->toJSON();
        jmeasurements.push_back(jmeas);
    }
    j["measurements"] = jmeasurements;
    j["calkit"] = kit.toJSON();
    j["type"] = TypeToString(caltype.type).toStdString();
    nlohmann::json jports;
    for(auto p : caltype.usedPorts) {
        jports.push_back(p);
    }
    j["ports"] = jports;
    j["version"] = qlibrevnaApp->applicationVersion().toStdString();
    j["device"] = validDevice.toStdString();
    return j;
}

void Calibration::fromJSON(nlohmann::json j)
{
    reset();
    lock_guard<recursive_mutex> guard(access);
    if(j.contains("calkit")) {
        kit.fromJSON(j["calkit"]);
    }
    unsigned int format = 0;
    if(j.contains("format")) {
        format = j["format"];
    } else {
        // no clear format indicated, attempt to guess from json content
        if(j.contains("port1StandardMale")) {
            format = 2;
        } else if(j.contains("version")){
            format = 3;
        }
    }
    switch(format) {
    case 3: {
        if(j.contains("measurements")) {
            for(auto jm : j["measurements"]) {
                auto type = CalibrationMeasurement::Base::TypeFromString(QString::fromStdString(jm.value("type", "")));
                auto m = newMeasurement(type);
                if(m && jm.contains("data")) {
                    m->fromJSON(jm["data"]);
                    measurements.push_back(m);
                }
            }
        }

        CalType ct;
        ct.type = TypeFromString(QString::fromStdString(j.value("type", "")));
        if(j.contains("ports")) {
            for(auto jp : j["ports"]) {
                ct.usedPorts.push_back(jp);
            }
        }
        if(ct.type != Type::None) {
            compute(ct);
        }
        validDevice = QString::fromStdString(j.value("device", ""));
    }
        break;
    case 2: {
        // associated calkit should already be loaded
        if(j.contains("measurements")) {
            // grab measurements
            for(auto j_m : j["measurements"]) {
                if(!j_m.contains("name")) {
                    throw runtime_error("Measurement without name given");
                }
                CalibrationMeasurement::Base *m = nullptr;
                auto name = QString::fromStdString(j_m["name"]);
                if(name == "Port 1 Open") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Open);
                    static_cast<CalibrationMeasurement::OnePort*>(m)->setPort(1);
                } else if(name == "Port 1 Short") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Short);
                    static_cast<CalibrationMeasurement::OnePort*>(m)->setPort(1);
                } else if(name == "Port 1 Load") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Load);
                    static_cast<CalibrationMeasurement::OnePort*>(m)->setPort(1);
                } else if(name == "Port 2 Open") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Open);
                    static_cast<CalibrationMeasurement::OnePort*>(m)->setPort(2);
                } else if(name == "Port 2 Short") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Short);
                    static_cast<CalibrationMeasurement::OnePort*>(m)->setPort(2);
                } else if(name == "Port 2 Load") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Load);
                    static_cast<CalibrationMeasurement::OnePort*>(m)->setPort(2);
                } else if(name == "Through") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Through);
                    static_cast<CalibrationMeasurement::TwoPort*>(m)->setPort1(1);
                    static_cast<CalibrationMeasurement::TwoPort*>(m)->setPort2(2);
                } else if(name == "Isolation") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Isolation);
                } else if(name == "Line") {
                    m = newMeasurement(CalibrationMeasurement::Base::Type::Line);
                    static_cast<CalibrationMeasurement::TwoPort*>(m)->setPort1(1);
                    static_cast<CalibrationMeasurement::TwoPort*>(m)->setPort2(2);
                }
                if(!m) {
                    // unknown measurement name
                    throw runtime_error("Measurement name unknown: "+std::string(j_m["name"]));
                }
                // attempt to select the correct standard
                bool p1male = j.value("port1StandardMale", true);
                bool p2male = j.value("port2StandardMale", true);
                bool throughZeroLength = j.value("throughZeroLength", true);

                auto onePort = dynamic_cast<CalibrationMeasurement::OnePort*>(m);
                if(onePort) {
                    bool male = onePort->getPort() == 1 ? p1male : p2male;
                    QString name = male ? "Default male standard" : "Default female standard";
                    for(auto s : m->supportedStandards()) {
                        if(s->getName() == name) {
                            m->setStandard(s);
                            break;
                        }
                    }
                }
                auto through = dynamic_cast<CalibrationMeasurement::Through*>(m);
                if(through) {
                    if(throughZeroLength) {
                        // needs to create a zero length through standard in the calkit in addition to the already existing through standard
                        auto zerolength = new CalStandard::Through();
                        zerolength->setName("Zero length");
                        kit.addStandard(zerolength);
                        // use this standard
                        m->setStandard(zerolength);
                    }
                }
                if(!m->getStandard()) {
                    // failed to find specific standard, use the first available
                    m->setFirstSupportedStandard();
                }
                // extract points
                if(!j_m.contains("points")) {
                    throw runtime_error("Measurement "+name.toStdString()+" does not contain any points");
                }
                for(auto j_p : j_m["points"]) {
                    DeviceDriver::VNAMeasurement p;
                    p.frequency = j_p.value("frequency", 0.0);
                    p.Z0 = 50.0;
                    p.measurements["S11"] = complex<double>(j_p.value("S11_real", 0.0), j_p.value("S11_imag", 0.0));
                    p.measurements["S12"] = complex<double>(j_p.value("S12_real", 0.0), j_p.value("S12_imag", 0.0));
                    p.measurements["S21"] = complex<double>(j_p.value("S21_real", 0.0), j_p.value("S21_imag", 0.0));
                    p.measurements["S22"] = complex<double>(j_p.value("S22_real", 0.0), j_p.value("S22_imag", 0.0));
                    m->addPoint(p);
                }
                measurements.push_back(m);
            }
        }
        // got all measurements, construct calibration according to type
        if(j.contains("type")) {
            auto type = QString::fromStdString(j["type"]);
            CalType ct;
            ct.type = Calibration::Type::None;
            if(type == "Port 1") {
                ct.type = Calibration::Type::SOLT;
                ct.usedPorts = {1};
            } else if(type == "Port 2") {
                ct.type = Calibration::Type::SOLT;
                ct.usedPorts = {2};
            } else if(type == "SOLT") {
                ct.type = Calibration::Type::SOLT;
                ct.usedPorts = {1,2};
            } else if(type == "Normalize") {
                ct.type = Calibration::Type::ThroughNormalization;
                ct.usedPorts = {1,2};
            } else if(type == "TRL") {
                ct.type = Calibration::Type::TRL;
                ct.usedPorts = {1,2};
            }
            if(ct.type != Type::None) {
                compute(ct);
            }
        }
        InformationBox::ShowMessage("Old calibration format", "The selected calibration file was saved in an old format. Please check the imported result "
                                                              "carefully and save the calibration to migrate to the new format");
    }
        break;
    default:
        InformationBox::ShowError("Failed to load calibration", "Unable to load calibration, unknown json format: "+QString::number(format));
        break;
    }
}

bool Calibration::toFile(QString filename)
{
    if(filename.isEmpty()) {
        QString fn = descriptiveCalName();
        filename = QFileDialog::getSaveFileName(nullptr, "Save calibration data", Preferences::getInstance().UISettings.Paths.cal + "/" + fn, "Calibration files (*.cal)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return false;
        }
        Preferences::getInstance().UISettings.Paths.cal = QFileInfo(filename).path();
    }

    if(filename.toLower().endsWith(".cal")) {
        filename.chop(4);
    }
    auto calibration_file = filename + ".cal";
    ofstream file;
    file.open(calibration_file.toStdString());
    file << setw(1) << toJSON();

    this->currentCalFile = calibration_file;    // if all ok, remember this

    unsavedChanges = false;
    return true;
}

bool Calibration::fromFile(QString filename)
{
    if(filename.isEmpty()) {
        filename = QFileDialog::getOpenFileName(nullptr, "Load calibration data", Preferences::getInstance().UISettings.Paths.cal, "Calibration files (*.cal)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return false;
        }
        Preferences::getInstance().UISettings.Paths.cal = QFileInfo(filename).path();
    }

    // force correct file ending
    if(filename.toLower().endsWith(".cal")) {
        filename.chop(4);
        filename += ".cal";
    }

    qDebug() << "Attempting to open calibration from file" << filename;

    // attempt to load associated calibration kit first. It needs to be available when using the old calibration format. If the
    // newer calibration format is used, the calibration kit is overwritten by the calibration file anyway
    auto calkit_file = filename;
    auto dotPos = calkit_file.lastIndexOf('.');
    if(dotPos >= 0) {
        calkit_file.truncate(dotPos);
    }
    calkit_file.append(".calkit");
    qDebug() << "Associated calibration kit expected in" << calkit_file;
    try {
        kit = Calkit::fromFile(calkit_file);
    } catch (runtime_error &e) {
        qDebug() << "Parsing of calibration kit failed while opening calibration file: " << e.what() << " (ignore for calibration format >= 3)";
    }

    ifstream file;

    file.open(filename.toStdString());
    if(!file.good()) {
        QString msg = "Unable to open file: "+filename;
        InformationBox::ShowError("Error", msg);
        qWarning() << msg;
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        currentCalFile = filename;    // if all ok, remember this
        fromJSON(j);
    } catch(exception &e) {
        currentCalFile.clear();
        InformationBox::ShowError("File parsing error", e.what());
        qWarning() << "Calibration file parsing failed: " << e.what();
        return false;
    }

    unsavedChanges = false;
    return true;
}

std::vector<Calibration::CalType> Calibration::getAvailableCalibrations()
{
    unsigned int ports = DeviceDriver::getInfo(DeviceDriver::getActiveDriver()).Limits.VNA.ports;
    vector<CalType> ret;
    for(auto t : getTypes()) {
        CalType cal;
        cal.type = t;
        auto minPorts = minimumPorts(t);
        for(unsigned int pnum = minPorts;pnum <= ports;pnum++) {
            std::string bitmask(pnum, 1);
            bitmask.resize(ports, 0);
            // assemble selected ports and permute bitmask
            do {
                vector<unsigned int> usedPorts;
                for (unsigned int i = 0; i < ports; ++i) {
                    if (bitmask[i]) {
                        usedPorts.push_back(i+1);
                    }
                }
                cal.usedPorts = usedPorts;
                ret.push_back(cal);
            } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
        }
    }
    return ret;
}

std::vector<Calibration::Type> Calibration::getTypes()
{
    vector<Type> types;
    // Start at index 1, skip Type::None
    for(int i=1;i<(int) Type::Last;i++) {
        types.push_back((Type) i);
    }
    return types;
}

bool Calibration::canCompute(Calibration::CalType type, double *startFreq, double *stopFreq, int *points, bool *isLog)
{
    using RequiredMeasurements = struct {
        CalibrationMeasurement::Base::Type type;
        unsigned int port1, port2;
    };
    vector<RequiredMeasurements> required;
    switch(type.type) {
    case Type::None:
        return true; // Always possible to reset the calibration
    case Type::SOLT:
    case Type::SOLTwithoutRxMatch:
        // through measurements between all ports
        for(unsigned int i=1;i<=type.usedPorts.size();i++) {
            for(unsigned int j=i+1;j<=type.usedPorts.size();j++) {
                required.push_back({.type = CalibrationMeasurement::Base::Type::Through, .port1 = i, .port2 = j});
            }
        }
        [[fallthrough]];
    case Type::OSL:
        // SOL measurements for every port
        for(auto p : type.usedPorts) {
            required.push_back({.type = CalibrationMeasurement::Base::Type::Short, .port1 = p, .port2 = 0});
            required.push_back({.type = CalibrationMeasurement::Base::Type::Open, .port1 = p, .port2 = 0});
            if(findMeasurements(CalibrationMeasurement::Base::Type::SlidingLoad, p).size() >= 3) {
                // got enough sliding load measurements, use these
                required.push_back({.type = CalibrationMeasurement::Base::Type::SlidingLoad, .port1 = p, .port2 = 0});
            } else {
                // not enough sliding load measurement, use normal load
                required.push_back({.type = CalibrationMeasurement::Base::Type::Load, .port1 = p, .port2 = 0});
            }
        }
        break;
    case Type::ThroughNormalization:
        // through measurements between all ports
        for(unsigned int i=1;i<=type.usedPorts.size();i++) {
            for(unsigned int j=i+1;j<=type.usedPorts.size();j++) {
                required.push_back({.type = CalibrationMeasurement::Base::Type::Through, .port1 = i, .port2 = j});
            }
        }
        break;
    case Type::TRL:
        // Reflect measurement for every port
        for(auto p : type.usedPorts) {
            required.push_back({.type = CalibrationMeasurement::Base::Type::Reflect, .port1 = p, .port2 = 0});
        }
        // through and line measurements between all ports
        for(unsigned int i=1;i<=type.usedPorts.size();i++) {
            for(unsigned int j=i+1;j<=type.usedPorts.size();j++) {
                required.push_back({.type = CalibrationMeasurement::Base::Type::Through, .port1 = i, .port2 = j});
                required.push_back({.type = CalibrationMeasurement::Base::Type::Line, .port1 = i, .port2 = j});
            }
        }
        break;
    case Type::Last:
        // Invalid selection
        return false;

    }
    if(required.size() > 0) {
        vector<CalibrationMeasurement::Base*> foundMeasurements;
        for(auto m : required) {
            auto meas = findMeasurement(m.type, m.port1, m.port2);
            if(!meas) {
                // missing measurement
                return false;
            } else if (!meas->readyForCalculation()){
                // measurement not ready (either not calkit standard definded or no measurements)
                return false;
            } else {
                foundMeasurements.push_back(meas);
            }
        }
        return hasFrequencyOverlap(foundMeasurements, startFreq, stopFreq, points, isLog);
    }
    return false;
}

bool Calibration::compute(Calibration::CalType type)
{
    lock_guard<recursive_mutex> guard(access);
    if(type.type == Type::None) {
        deactivate();
        return true;
    }
    double start, stop;
    int numPoints;
    bool isLog;
    if(!canCompute(type, &start, &stop, &numPoints, &isLog)) {
        return false;
    }
    caltype = type;
    try {
        points.clear();
        for(int i=0;i<numPoints;i++) {
            double f;
            if(!isLog) {
                f = start + (stop - start) * i / (numPoints - 1);
            } else {
                f = start * pow(10.0, i * log10(stop / start) / (numPoints - 1));
            }
            Point p;
            switch(type.type) {
            case Type::OSL: p = computeOSL(f); break;
            case Type::SOLT: p = computeSOLT(f); break;
            case Type::SOLTwithoutRxMatch: p = computeSOLTwithoutRxMatch(f); break;
            case Type::ThroughNormalization: p = computeThroughNormalization(f); break;
            case Type::TRL: p = computeTRL(f); break;
            case Type::None:
            case Type::Last:
                // nothing to do, should never get here
                break;
            }
            points.push_back(p);
        }
    } catch (exception &e) {
        points.clear();
        caltype.usedPorts.clear();
    }
    emit activated(caltype);
    unsavedChanges = true;
    return true;
}

void Calibration::reset()
{
    deleteMeasurements();
    deactivate();
}

int Calibration::minimumPorts(Calibration::Type type)
{
    switch(type) {
    case Type::OSL: return 1;
    case Type::SOLT: return 1;
    case Type::SOLTwithoutRxMatch: return 2;
    case Type::ThroughNormalization: return 2;
    case Type::TRL: return 2;
    case Type::None:
    case Type::Last:
        return -1;
    }
    return -1;
}

void Calibration::addMeasurements(std::set<CalibrationMeasurement::Base *> m, const DeviceDriver::VNAMeasurement &data)
{
    for(auto meas : m) {
        meas->addPoint(data);
    }
    unsavedChanges = true;
    if(DeviceDriver::getActiveDriver()) {
        validDevice = DeviceDriver::getActiveDriver()->getSerial();
    }
}

void Calibration::clearMeasurements(std::set<CalibrationMeasurement::Base *> m)
{
    for(auto meas : m) {
        meas->clearPoints();
    }
    unsavedChanges = true;
}

void Calibration::measurementsComplete()
{
    emit measurementsUpdated();
}

void Calibration::measurementsAbort()
{
    emit measurementsAborted();
}

void Calibration::deactivate()
{
    lock_guard<recursive_mutex> guard(access);
    points.clear();
    caltype.type = Type::None;
    caltype.usedPorts.clear();
    unsavedChanges = true;
    emit deactivated();
}

QString Calibration::DefaultMeasurementsToString(Calibration::DefaultMeasurements dm)
{
    switch(dm) {
    case DefaultMeasurements::SOL1Port: return "1 Port SOL";
    case DefaultMeasurements::SOLT2Port: return "2 Port SOLT";
    case DefaultMeasurements::SOLT3Port: return "3 Port SOLT";
    case DefaultMeasurements::SOLT4Port: return "4 Port SOLT";
    case DefaultMeasurements::Last: return "Invalid";
    }
    return "";
}

void Calibration::createDefaultMeasurements(Calibration::DefaultMeasurements dm)
{
    lock_guard<recursive_mutex> guard(access);
    auto createSOL = [=](int port) {
        auto _short = new CalibrationMeasurement::Short(this);
        _short->setPort(port);
        measurements.push_back(_short);
        auto open = new CalibrationMeasurement::Open(this);
        open->setPort(port);
        measurements.push_back(open);
        auto load = new CalibrationMeasurement::Load(this);
        load->setPort(port);
        measurements.push_back(load);
    };
    auto createThrough = [=](int port1, int port2) {
        auto through = new CalibrationMeasurement::Through(this);
        through->setPort1(port1);
        through->setPort2(port2);
        measurements.push_back(through);
    };
    switch(dm) {
    case DefaultMeasurements::SOL1Port:
        createSOL(1);
        break;
    case DefaultMeasurements::SOLT2Port:
        createSOL(1);
        createSOL(2);
        createThrough(1, 2);
        break;
    case DefaultMeasurements::SOLT3Port:
        createSOL(1);
        createSOL(2);
        createSOL(3);
        createThrough(1, 2);
        createThrough(1, 3);
        createThrough(2, 3);
        break;
    case DefaultMeasurements::SOLT4Port:
        createSOL(1);
        createSOL(2);
        createSOL(3);
        createSOL(4);
        createThrough(1, 2);
        createThrough(1, 3);
        createThrough(1, 4);
        createThrough(2, 3);
        createThrough(2, 4);
        createThrough(3, 4);
        break;
    case DefaultMeasurements::Last:
        break;
    }
}

void Calibration::deleteMeasurements()
{
    lock_guard<recursive_mutex> guard(access);
    for(auto m : measurements) {
        delete m;
    }
    measurements.clear();
}

bool Calibration::hasFrequencyOverlap(std::vector<CalibrationMeasurement::Base *> m, double *startFreq, double *stopFreq, int *points, bool *isLog)
{
    double minResolution = std::numeric_limits<double>::max();
    double minFreq = 0;
    double maxFreq = std::numeric_limits<double>::max();
    unsigned int logCount = 0;
    unsigned int linCount = 0;
    for(auto meas : m) {
        if(meas->numPoints() < 2) {
            return false;
        }
        auto resolution = (meas->maxUsableFreq() - meas->minUsableFreq()) / (meas->numPoints() - 1);
        if(meas->maxUsableFreq() < maxFreq) {
            maxFreq = meas->maxUsableFreq();
        }
        if(meas->minUsableFreq() > minFreq) {
            minFreq = meas->minUsableFreq();
        }
        if(resolution < minResolution) {
            minResolution = resolution;
        }
        // check whether the frequency points are more linear or more logarithmic
        double minDiff = std::numeric_limits<double>::max();
        double maxDiff = 0;
        double minRatio = std::numeric_limits<double>::max();
        double maxRatio = 0;
        for(unsigned int i=1;i<meas->numPoints();i++) {
            double f_prev = meas->getPointFreq(i-1);
            double f_next = meas->getPointFreq(i);
            double diff = f_next - f_prev;
            double ratio = f_next / f_prev;
            if(diff < minDiff) {
                minDiff = diff;
            }
            if(diff > maxDiff) {
                maxDiff = diff;
            }
            if(ratio < minRatio) {
                minRatio = ratio;
            }
            if(ratio > maxRatio) {
                maxRatio = ratio;
            }
        }
        double diffVariationNormalized = (maxDiff - minDiff) / maxDiff;
        double ratioVariationNormalized = (maxRatio - minRatio) / maxRatio;
        if(abs(diffVariationNormalized) < abs(ratioVariationNormalized)) {
            // more linear
            linCount++;
        } else {
            // more logarithmic
            logCount++;
        }
    }
    if(startFreq) {
        *startFreq = minFreq;
    }
    if(stopFreq) {
        *stopFreq = maxFreq;
    }
    if(points) {
        *points = (maxFreq - minFreq) / minResolution + 1;
    }
    if(isLog) {
        *isLog = logCount > linCount;
    }
    if(maxFreq > minFreq) {
        return true;
    } else {
        return false;
    }
}

std::vector<CalibrationMeasurement::Base *> Calibration::findMeasurements(CalibrationMeasurement::Base::Type type, int port1, int port2)
{
    vector<CalibrationMeasurement::Base*> ret;
    for(auto m : measurements) {
        if(m->getType() != type) {
            continue;
        }
        auto onePort = dynamic_cast<CalibrationMeasurement::OnePort*>(m);
        if(onePort) {
            if(onePort->getPort() != port1) {
                continue;
            }
        }
        auto twoPort = dynamic_cast<CalibrationMeasurement::TwoPort*>(m);
        if(twoPort) {
            if(twoPort->getPort1() != port1 || twoPort->getPort2() != port2) {
                continue;
            }
        }
        // if we get here, we have a match
        ret.push_back(m);
    }
    return ret;
}

CalibrationMeasurement::Base *Calibration::findMeasurement(CalibrationMeasurement::Base::Type type, int port1, int port2)
{
    auto meas = findMeasurements(type, port1, port2);
    if(meas.size() > 0) {
        return meas[0];
    } else {
        return nullptr;
    }
}

QString Calibration::CalType::getReadableDescription()
{
    QString ret = TypeToString(this->type);
    if(usedPorts.size() == 1) {
        ret += ", Port: "+QString::number(usedPorts[0]);
    } else if(usedPorts.size() > 0) {
        ret += ", Ports: [";
        for(auto p : usedPorts) {
            ret += QString::number(p)+",";
        }
        // remove the last trailing comma
        ret.chop(1);
        ret += "]";
    }
    return ret;
}

QString Calibration::CalType::getShortString()
{
    QString ret = TypeToString(this->type);
    if(usedPorts.size() > 0) {
        ret += "_";
    }
    for(auto p : usedPorts) {
        ret += QString::number(p);
    }
    return ret;
}

Calibration::CalType Calibration::CalType::fromShortString(QString s)
{
    CalType ret;
    auto list = s.split("_");
    if(list.size() != 2) {
        ret.type = Type::None;
    } else {
        ret.type = TypeFromString(list[0]);
        for(auto c : list[1]) {
            ret.usedPorts.push_back(QString(c).toInt());
        }
    }
    return ret;
}

Calibration::Point Calibration::Point::interpolate(const Calibration::Point &to, double alpha)
{
    Point ret;
    ret.frequency = frequency * (1.0-alpha) + to.frequency * alpha;
    ret.D.resize(D.size(), 0.0);
    for(unsigned int i=0;i<D.size();i++) {
        ret.D[i] = Util::interpolateMagPhase(D[i], to.D[i], alpha);
    }
    ret.R.resize(R.size(), 0.0);
    for(unsigned int i=0;i<R.size();i++) {
        ret.R[i] = Util::interpolateMagPhase(R[i], to.R[i], alpha);
    }
    ret.S.resize(S.size(), 0.0);
    for(unsigned int i=0;i<S.size();i++) {
        ret.S[i] = Util::interpolateMagPhase(S[i], to.S[i], alpha);
    }
    ret.T.resize(T.size());
    for(unsigned int i=0;i<T.size();i++) {
        ret.T[i].resize(T[i].size(), 0.0);
        for(unsigned int j=0;j<T[i].size();j++) {
            ret.T[i][j] = Util::interpolateMagPhase(T[i][j], to.T[i][j], alpha);
        }
    }
    ret.L.resize(L.size());
    for(unsigned int i=0;i<L.size();i++) {
        ret.L[i].resize(L[i].size(), 0.0);
        for(unsigned int j=0;j<L[i].size();j++) {
            ret.L[i][j] = Util::interpolateMagPhase(L[i][j], to.L[i][j], alpha);
        }
    }
    ret.I.resize(I.size());
    for(unsigned int i=0;i<I.size();i++) {
        ret.I[i].resize(I[i].size(), 0.0);
        for(unsigned int j=0;j<I[i].size();j++) {
            ret.I[i][j] = Util::interpolateMagPhase(I[i][j], to.I[i][j], alpha);
        }
    }
    return ret;
}
