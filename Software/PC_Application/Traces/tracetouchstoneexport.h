#ifndef TRACETOUCHSTONEEXPORT_H
#define TRACETOUCHSTONEEXPORT_H

#include <QDialog>
#include <QComboBox>
#include "tracemodel.h"
#include <QSignalMapper>

namespace Ui {
class TraceTouchstoneExport;
}

class TraceTouchstoneExport : public QDialog
{
    Q_OBJECT

public:
    explicit TraceTouchstoneExport(TraceModel &model, QWidget *parent = nullptr);
    ~TraceTouchstoneExport();
    bool setTrace(int portFrom, int portTo, Trace *t);
    bool setPortNum(int ports);

private slots:
    void on_buttonBox_accepted();
    void on_sbPorts_valueChanged(int ports);
    void selectionChanged(QComboBox *w);

private:
    Ui::TraceTouchstoneExport *ui;
    TraceModel &model;
    std::vector<std::vector<QComboBox*>> cTraces;

    unsigned int ports;
    unsigned int points;
    double lowerFreq, upperFreq;
    bool freqsSet;
};

#endif // TRACEEXPORTDIALOG_H
