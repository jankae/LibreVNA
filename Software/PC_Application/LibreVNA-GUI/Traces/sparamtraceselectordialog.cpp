#include "sparamtraceselectordialog.h"
#include "ui_sparamtraceselectordialog.h"

#include "sparamtraceselector.h"
#include <QPushButton>

SParamTraceSelectorDialog::SParamTraceSelectorDialog(const TraceModel &model, std::vector<unsigned int> used_ports, bool empty_allowed)
    : QDialog(nullptr)
    , ui(new Ui::SParamTraceSelectorDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    auto selector = new SparamTraceSelector(model, used_ports, empty_allowed);
    ui->verticalLayout->replaceWidget(ui->placeholder, selector);

    auto okButton = ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
    connect(selector, &SparamTraceSelector::selectionValid, okButton, &QPushButton::setEnabled);
    okButton->setEnabled(selector->isValid());

    connect(okButton, &QPushButton::clicked, [=](){
        auto traces = selector->getTraces();
        if(traces.size() == 0) {
            // should not happen
            reject();
        }
        emit tracesSelected(Trace::assembleDatapoints(traces));
        accept();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SParamTraceSelectorDialog::reject);
}

SParamTraceSelectorDialog::~SParamTraceSelectorDialog()
{
    delete ui;
}
