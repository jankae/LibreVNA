#ifndef SPARAMTRACESELECTORDIALOG_H
#define SPARAMTRACESELECTORDIALOG_H

#include <QDialog>

#include "tracemodel.h"

namespace Ui {
class SParamTraceSelectorDialog;
}

class SParamTraceSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SParamTraceSelectorDialog(const TraceModel &model, std::vector<unsigned int> used_ports, bool empty_allowed = false);
    ~SParamTraceSelectorDialog();

signals:
    void tracesSelected(std::vector<DeviceDriver::VNAMeasurement> traceMeasurements);

private:
    Ui::SParamTraceSelectorDialog *ui;
};

#endif // SPARAMTRACESELECTORDIALOG_H
