#ifndef TRACEEXPORTDIALOG_H
#define TRACEEXPORTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include "tracemodel.h"
#include <QSignalMapper>

namespace Ui {
class TraceExportDialog;
}

class TraceExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TraceExportDialog(TraceModel &model, QWidget *parent = nullptr);
    ~TraceExportDialog();
    bool setTrace(int portFrom, int portTo, Trace *t);
    bool setPortNum(int ports);

private slots:
    void on_buttonBox_accepted();
    void on_sbPorts_valueChanged(int ports);
    void selectionChanged(QComboBox *w);

private:
    Ui::TraceExportDialog *ui;
    TraceModel &model;
    std::vector<std::vector<QComboBox*>> cTraces;

    unsigned int points;
    double lowerFreq, upperFreq;
    bool freqsSet;
};

#endif // TRACEEXPORTDIALOG_H
