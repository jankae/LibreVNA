#include "manualdeembeddingdialog.h"

#include "ui_manualdeembeddingdialog.h"
#include "Traces/sparamtraceselector.h"
#include "CustomWidgets/informationbox.h"

ManualDeembeddingDialog::ManualDeembeddingDialog(const TraceModel &model, Deembedding *deemb) :
    ui(new Ui::ManualDeembeddingDialog)
{
    ui->setupUi(this);
    auto traceSelector = new SparamTraceSelector(model, deemb->getAffectedPorts(), true, 8);
    ui->verticalLayout->insertWidget(1, traceSelector, 1.0);
    ui->buttonBox->setEnabled(false);
    connect(traceSelector, &SparamTraceSelector::selectionValid, ui->buttonBox, &QDialogButtonBox::setEnabled);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
        auto traces = traceSelector->getTraces();
        bool clearDeembedding = false;
        for(auto t : traces) {
            if(t.second->deembeddingAvailable()) {
                clearDeembedding = InformationBox::AskQuestion("Clear previous de-embedding data?", "At least one of the selected traces "
                                   "has already been de-embedded. Do you want to clear the old de-embedding data before applying the new de-embedding?", true);
                break;
            }
        }
        if(clearDeembedding) {
            for(auto t : traces) {
                if(t.second->deembeddingAvailable()) {
                    t.second->clearDeembedding();
                }
            }
        }
        deemb->Deembed(traceSelector->getTraces());
        accept();
    });
}

ManualDeembeddingDialog::~ManualDeembeddingDialog()
{
    delete ui;
}
