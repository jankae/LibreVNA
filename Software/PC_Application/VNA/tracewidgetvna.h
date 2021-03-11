#ifndef TRACEWIDGETVNA_H
#define TRACEWIDGETVNA_H

#include "Traces/tracewidget.h"
#include "Calibration/calibration.h"
#include "Deembedding/deembedding.h"

class TraceWidgetVNA : public TraceWidget
{
public:
    TraceWidgetVNA(TraceModel &model, Calibration &cal, Deembedding &deembed, QWidget *parent = nullptr);
protected slots:
    virtual void exportDialog() override {};
    virtual void importDialog() override;

protected:
    virtual Trace::LiveParameter defaultParameter() override {return Trace::LiveParameter::S11;};
    // These can optionally be applied when importing an s2p file
    Calibration &cal;
    Deembedding &deembed;
};

#endif // TRACEWIDGETVNA_H
