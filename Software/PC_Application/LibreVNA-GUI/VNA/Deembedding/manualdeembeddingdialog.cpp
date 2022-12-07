#include "manualdeembeddingdialog.h"

#include "ui_manualdeembeddingdialog.h"
#include "Traces/sparamtraceselector.h"

ManualDeembeddingDialog::ManualDeembeddingDialog(const TraceModel &model, Deembedding *deemb) :
    ui(new Ui::ManualDeembeddingDialog)
{
    ui->setupUi(this);
    auto traceSelector = new SparamTraceSelector(model, deemb->getAffectedPorts(), true, 8);
    ui->verticalLayout->insertWidget(1, traceSelector, 1.0);
    ui->buttonBox->setEnabled(false);
    connect(traceSelector, &SparamTraceSelector::selectionValid, ui->buttonBox, &QDialogButtonBox::setEnabled);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
        deemb->Deembed(traceSelector->getTraces());
        accept();
    });
}

ManualDeembeddingDialog::~ManualDeembeddingDialog()
{
    delete ui;
}
