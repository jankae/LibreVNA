#include "manualcalibrationdialog.h"
#include "ui_manualcalibrationdialog.h"
#include "Traces/sparamtraceselector.h"

ManualCalibrationDialog::ManualCalibrationDialog(const TraceModel &model, Calibration *cal) :
    ui(new Ui::ManualCalibrationDialog)
{
    ui->setupUi(this);
    auto traceSelector = new SparamTraceSelector(model, 2);
    ui->verticalLayout->insertWidget(1, traceSelector, 1.0);
    ui->buttonBox->setEnabled(false);
    connect(traceSelector, &SparamTraceSelector::selectionValid, ui->buttonBox, &QDialogButtonBox::setEnabled);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
        auto t = traceSelector->getTraces();
        cal->correctTraces(*t[0], *t[1], *t[2], *t[3]);
        accept();
    });
}

ManualCalibrationDialog::~ManualCalibrationDialog()
{
    delete ui;
}
