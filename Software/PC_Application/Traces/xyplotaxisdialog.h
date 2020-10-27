#ifndef XYPLOTAXISDIALOG_H
#define XYPLOTAXISDIALOG_H

#include <QDialog>
#include "tracexyplot.h"

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
    QString YAxisUnit(TraceXYPlot::YAxisType type);
    std::set<TraceXYPlot::YAxisType> supportedYAxis(TraceXYPlot::XAxisType type);
    Ui::XYplotAxisDialog *ui;
    TraceXYPlot *plot;
};

#endif // XYPLOTAXISDIALOG_H
