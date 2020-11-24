#ifndef TRACEWIDGETVNA_H
#define TRACEWIDGETVNA_H

#include "Traces/tracewidget.h"

class TraceWidgetVNA : public TraceWidget
{
public:
    TraceWidgetVNA(TraceModel &model, QWidget *parent = nullptr);
protected slots:
    virtual void exportDialog() override;
    virtual void importDialog() override;

protected:
    virtual Trace::LiveParameter defaultParameter() override {return Trace::LiveParameter::S11;};
};

#endif // TRACEWIDGETVNA_H
