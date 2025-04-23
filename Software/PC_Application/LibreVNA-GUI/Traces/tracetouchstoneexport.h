#ifndef TRACETOUCHSTONEEXPORT_H
#define TRACETOUCHSTONEEXPORT_H

#include "tracemodel.h"

#include <QDialog>
#include <QComboBox>
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
    bool setPortNum(unsigned int ports);

private slots:
    void on_buttonBox_accepted();
    void selectionChanged();

private:
    Ui::TraceTouchstoneExport *ui;
};

#endif // TRACEEXPORTDIALOG_H
