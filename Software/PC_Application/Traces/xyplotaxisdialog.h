#ifndef XYPLOTAXISDIALOG_H
#define XYPLOTAXISDIALOG_H

#include "tracexyplot.h"

#include <QDialog>

namespace Ui {
class XYplotAxisDialog;
}

class XYplotAxisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit XYplotAxisDialog(TraceXYPlot *plot);
    ~XYplotAxisDialog();

private slots:
    void on_buttonBox_accepted();
    void XAxisTypeChanged(int XAxisIndex);

private:
    std::set<YAxis::Type> supportedYAxis(XAxis::Type type);
    bool isSupported(XAxis::Type type);
    Ui::XYplotAxisDialog *ui;
    TraceXYPlot *plot;
    void removeLine(int index);
};

#endif // XYPLOTAXISDIALOG_H
