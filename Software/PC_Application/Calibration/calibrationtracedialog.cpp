#include "calibrationtracedialog.h"

#include "ui_calibrationtracedialog.h"
#include "measurementmodel.h"
#include "CustomWidgets/informationbox.h"

#include <QStyle>

CalibrationTraceDialog::CalibrationTraceDialog(Calibration *cal, double f_min, double f_max, Calibration::Type type) :
    QDialog(nullptr),
    ui(new Ui::CalibrationTraceDialog),
    cal(cal),
    requestedType(type)
{
    ui->setupUi(this);
    ui->bApply->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    measurements = cal->Measurements(type);
    if(requestedType == Calibration::Type::None) {
        ui->bApply->setVisible(false);
    }
    model = new MeasurementModel(cal, measurements);
    ui->tableView->setModel(model);
    ui->tableView->setColumnWidth(0, 100);
    ui->tableView->setColumnWidth(1, 80);
    ui->tableView->setColumnWidth(2, 350);
    ui->tableView->setColumnWidth(3, 320);
    ui->tableView->setColumnWidth(4, 160);
    UpdateCalibrationStatus();

    auto updateThroughStandardUI = [=](){
        if(cal->getPortStandard(1) == cal->getPortStandard(2)) {
            // same gender on both ports, can't use zero length through
            ui->throughCalkit->click();
            ui->throughZero->setEnabled(false);
            ui->throughCalkit->setEnabled(false);
        } else {
            // user may select option for through
            ui->throughZero->setEnabled(true);
            ui->throughCalkit->setEnabled(true);
        }
        model->genderUpdated();
    };

    connect(ui->port1Group, qOverload<int>(&QButtonGroup::buttonClicked), [=](){
        if(ui->port1Male->isChecked()) {
            cal->setPortStandard(1, Calibration::PortStandard::Male);
        } else {
            cal->setPortStandard(1, Calibration::PortStandard::Female);
        }
        updateThroughStandardUI();
        UpdateCalibrationStatus();
    });

    connect(ui->port2Group, qOverload<int>(&QButtonGroup::buttonClicked), [=](){
        if(ui->port2Male->isChecked()) {
            cal->setPortStandard(2, Calibration::PortStandard::Male);
        } else {
            cal->setPortStandard(2, Calibration::PortStandard::Female);
        }
        updateThroughStandardUI();
        UpdateCalibrationStatus();
    });

    connect(ui->throughGroup, qOverload<int>(&QButtonGroup::buttonClicked), [=](){
        if(ui->throughZero->isChecked()) {
            cal->setThroughZeroLength(true);
        } else {
            cal->setThroughZeroLength(false);
        }
        UpdateCalibrationStatus();
    });

    // hide selector if calkit does not have separate male/female standards
    if(!cal->getCalibrationKit().hasSeparateMaleFemaleStandards()) {
        ui->port1Standards->hide();
        ui->port2Standards->hide();
        ui->throughStandard->hide();
        ui->tableView->hideColumn((int) MeasurementModel::ColIndex::Gender);
        // default selection is male
        ui->port1Male->click();
        ui->port2Male->click();
        ui->throughCalkit->click();
    } else {
        // separate standards defined
        if(cal->getPortStandard(1) == Calibration::PortStandard::Male) {
            ui->port1Male->setChecked(true);
        } else {
            ui->port1Female->setChecked(true);
        }
        if(cal->getPortStandard(2) == Calibration::PortStandard::Male) {
            ui->port2Male->setChecked(true);
        } else {
            ui->port2Female->setChecked(true);
        }
        if(cal->getThroughZeroLength()) {
            ui->throughZero->setChecked(true);
        } else {
            ui->throughCalkit->setChecked(true);
        }
        updateThroughStandardUI();
    }

    // Check calibration kit span
    if(type != Calibration::Type::None) {
        auto kit = cal->getCalibrationKit();
        auto isTRL = type == Calibration::Type::TRL;
        if(isTRL && (kit.minFreqTRL() > f_min || kit.maxFreqTRL() < f_max)) {
            // TODO check SOLT frequency range depending on selected male/female kit
            InformationBox::ShowMessage("Warning", "The calibration kit does not completely cover the currently selected span. "
                                        "Applying a calibration will not be possible for any measurements taken with these settings.");
        }
    }
}

CalibrationTraceDialog::~CalibrationTraceDialog()
{
    delete ui;
}

void CalibrationTraceDialog::measurementsComplete(std::set<Calibration::Measurement> m)
{
    for(auto t : m) {
        measurementComplete(t);
    }
}

void CalibrationTraceDialog::measurementComplete(Calibration::Measurement m)
{
    model->measurementUpdated(m);
    UpdateCalibrationStatus();
}

void CalibrationTraceDialog::UpdateCalibrationStatus()
{
    if(!cal->calculationPossible(cal->getType())) {
        // some trace for the current calibration was deleted
        cal->resetErrorTerms();
        emit calibrationInvalidated();
    } else {
        // update error terms as a measurement might have changed
        cal->constructErrorTerms(cal->getType());
    }
    ui->bApply->setEnabled(cal->calculationPossible(requestedType));
}

void CalibrationTraceDialog::on_bDelete_clicked()
{
    auto selected = ui->tableView->selectionModel()->selectedRows();
    for(auto s : selected) {
        cal->clearMeasurement(measurements[s.row()]);
        model->measurementUpdated(measurements[s.row()]);
    }
    UpdateCalibrationStatus();
}

void CalibrationTraceDialog::on_bMeasure_clicked()
{
    std::set<Calibration::Measurement> m;
    auto selected = ui->tableView->selectionModel()->selectedRows();
    for(auto s : selected) {
        m.insert(measurements[s.row()]);
    }

    // check if incompatible measurements are selected
    auto p1Standard = Calibration::Standard::Any;
    auto p2Standard = Calibration::Standard::Any;

    bool okay = true;
    for(auto type : m) {
        auto p1Required = Calibration::getPort1Standard(type);
        auto p2Required = Calibration::getPort2Standard(type);
        if(p1Required != Calibration::Standard::Any) {
            if(p1Standard == Calibration::Standard::Any) {
                // first calibration measurement type that needs a specific standard
                p1Standard = p1Required;
            } else if(p1Required != p1Standard) {
                // needs different standard than other measurement that has also been selected
                okay = false;
                break;
            }
        }
        // same check for port 2
        if(p2Required != Calibration::Standard::Any) {
            if(p2Standard == Calibration::Standard::Any) {
                // first calibration measurement type that needs a specific standard
                p2Standard = p2Required;
            } else if(p2Required != p2Standard) {
                // needs different standard than other measurement that has also been selected
                okay = false;
                break;
            }
        }
    }
    if(!okay) {
        // these measurements should not be taken at once, get user confirmation before continuing
        okay = InformationBox::AskQuestion("Confirm selection", "The selected calibration measurements require different standards. Are you sure you want to measure them at the same time?", false);
    }
    if(okay) {
        emit triggerMeasurements(m);
    }
}

void CalibrationTraceDialog::on_bApply_clicked()
{
    emit applyCalibration(requestedType);
    accept();
}
