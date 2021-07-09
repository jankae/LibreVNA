#include "calibrationtracedialog.h"
#include "ui_calibrationtracedialog.h"
#include "measurementmodel.h"
#include <QStyle>
#include "CustomWidgets/informationbox.h"

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
    auto measurement = measurements[ui->tableView->currentIndex().row()];
    emit triggerMeasurement(measurement);
}

void CalibrationTraceDialog::on_bApply_clicked()
{
    emit applyCalibration(requestedType);
    accept();
}
