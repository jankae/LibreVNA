#ifndef TRACEWIDGETVNA_H
#define TRACEWIDGETVNA_H

#include "Traces/tracewidget.h"
#include "Calibration/calibration.h"
#include "Deembedding/deembedding.h"

class TraceWidgetVNA : public TraceWidget
{
public:
    TraceWidgetVNA(TraceModel &model, Calibration *cal, Deembedding *deembed, QWidget *parent = nullptr);
public slots:
    void exportCSV();
    void exportTouchstone();
    virtual void exportDialog() override {}
    virtual QStringList supportsImportFileFormats() override {return {"csv", "s1p", "s2p", "s3p", "s4p"};}

protected:
    virtual QString defaultParameter() override {return "S11";}
};

#endif // TRACEWIDGETVNA_H
