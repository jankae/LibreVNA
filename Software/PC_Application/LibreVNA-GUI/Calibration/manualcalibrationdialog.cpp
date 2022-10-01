#include "manualcalibrationdialog.h"

#include "ui_manualcalibrationdialog.h"
#include "Traces/sparamtraceselector.h"

ManualCalibrationDialog::ManualCalibrationDialog(const TraceModel &model, Calibration *cal) :
    ui(new Ui::ManualCalibrationDialog)
{
    ui->setupUi(this);
    auto traceSelector = new SparamTraceSelector(model, cal->getCaltype().usedPorts);
    ui->verticalLayout->insertWidget(1, traceSelector, 1.0);
    ui->buttonBox->setEnabled(false);
    connect(traceSelector, &SparamTraceSelector::selectionValid, ui->buttonBox, &QDialogButtonBox::setEnabled);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
        cal->correctTraces(traceSelector->getTraces());
        accept();
    });
}

ManualCalibrationDialog::~ManualCalibrationDialog()
{
    delete ui;
}
