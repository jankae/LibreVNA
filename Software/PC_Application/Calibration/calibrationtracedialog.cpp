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
    ui->tableView->setColumnWidth(1, 350);
    ui->tableView->setColumnWidth(2, 320);
    ui->tableView->setColumnWidth(3, 160);
    UpdateCalibrationStatus();

    // Check calibration kit span
    if(type != Calibration::Type::None) {
        auto kit = cal->getCalibrationKit();
        auto isTRL = type == Calibration::Type::TRL;
        if(kit.minFreq(isTRL) > f_min || kit.maxFreq(isTRL) < f_max) {
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
    auto measurement = measurements[ui->tableView->currentIndex().row()];
    cal->clearMeasurement(measurement);
    model->measurementUpdated(measurement);
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
