#ifndef TRACEEDITDIALOG_H
#define TRACEEDITDIALOG_H

#include <QDialog>
#include "trace.h"

namespace Ui {
class TraceEditDialog;
}

class TraceEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TraceEditDialog(Trace &t, QWidget *parent = nullptr);
    ~TraceEditDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::TraceEditDialog *ui;
    Trace &trace;
    bool VNAtrace;
};

#endif // TRACEEDITDIALOG_H
