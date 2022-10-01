#ifndef MANUALDEEMBEDDINGDIALOG_H
#define MANUALDEEMBEDDINGDIALOG_H

#include "Traces/tracemodel.h"
#include "deembedding.h"

#include <QDialog>

namespace Ui {
class ManualDeembeddingDialog;
}

class ManualDeembeddingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualDeembeddingDialog(const TraceModel &model, Deembedding *deemb);
    ~ManualDeembeddingDialog();

private:
    Ui::ManualDeembeddingDialog *ui;
};

#endif // MANUALDEEMBEDDINGDIALOG_H
