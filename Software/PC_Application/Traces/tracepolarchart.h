#ifndef TRACEPOLARCHART_H
#define TRACEPOLARCHART_H

#include "tracepolar.h"

class TracePolarChart : public TracePolar
{
    Q_OBJECT
public:
    TracePolarChart(TraceModel &model, QWidget *parent = 0);

    virtual Type getType() override { return Type::PolarChart;};

public slots:
    virtual void axisSetupDialog() override;

private:
    bool supported(Trace *t) override;
    virtual void draw(QPainter& painter) override;
    virtual bool dropSupported(Trace *t) override;
    QString mouseText(QPoint pos) override;
};

#endif // TRACEPOLARCHART_H
