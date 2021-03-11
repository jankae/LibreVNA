#include "manualdeembeddingdialog.h"
#include "ui_manualdeembeddingdialog.h"
#include "Traces/sparamtraceselector.h"

ManualDeembeddingDialog::ManualDeembeddingDialog(const TraceModel &model, Deembedding *deemb) :
    ui(new Ui::ManualDeembeddingDialog)
{
    ui->setupUi(this);
    auto traceSelector = new SparamTraceSelector(model, 2);
    ui->verticalLayout->insertWidget(1, traceSelector, 1.0);
    ui->buttonBox->setEnabled(false);
    connect(traceSelector, &SparamTraceSelector::selectionValid, ui->buttonBox, &QDialogButtonBox::setEnabled);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
        auto t = traceSelector->getTraces();
        deemb->Deembed(*t[0], *t[1], *t[2], *t[3]);
        accept();
    });
}

ManualDeembeddingDialog::~ManualDeembeddingDialog()
{
    delete ui;
}
