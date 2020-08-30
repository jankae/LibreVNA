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
    void on_color_clicked();
    void on_buttonBox_accepted();

private:
    void setColor(QColor c);
    Ui::TraceEditDialog *ui;
    Trace &trace;
};

#endif // TRACEEDITDIALOG_H
