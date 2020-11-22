#ifndef TRACESMITHCHART_H
#define TRACESMITHCHART_H

#include "traceplot.h"
#include <QPen>
#include <QPainterPath>
#include <QTransform>

class TraceSmithChart : public TracePlot
{
    Q_OBJECT
public:
    TraceSmithChart(TraceModel &model, QWidget *parent = 0);
public slots:
    void axisSetupDialog();
protected:
    static constexpr double ReferenceImpedance = 50.0;
    static constexpr double screenUsage = 0.9;
    static constexpr double smithCoordMax = 4096;

    QPoint dataToPixel(Trace::Data d) override;

    //void paintEvent(QPaintEvent *event) override;
    virtual void updateContextMenu() override;
    bool supported(Trace *t) override;
    virtual void draw(QPainter& painter) override;
    virtual void traceDropped(Trace *t, QPoint position) override;
    QPen textPen;
    QPen chartLinesPen;
    QPen thinPen;
    QPen pointDataPen;
    QPen lineDataPen;
    bool limitToSpan;

    /// Path for the thin arcs
    QPainterPath thinArcsPath;
    /// Path for the thick arcs
    QPainterPath thickArcsPath;

    QTransform transform;
};

#endif // TRACESMITHCHART_H
