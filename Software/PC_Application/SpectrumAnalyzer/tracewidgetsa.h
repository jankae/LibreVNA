#ifndef TRACEWIDGETSA_H
#define TRACEWIDGETSA_H

#include "Traces/tracewidget.h"

class TraceWidgetSA : public TraceWidget
{
public:
    TraceWidgetSA(TraceModel &model, QWidget *parent = nullptr);
protected slots:
    virtual void exportDialog() override;
    virtual void importDialog() override;

protected:
    virtual Trace::LiveParameter defaultParameter() override {return Trace::LiveParameter::Port1;};
};

#endif // TRACEWIDGETSA_H
