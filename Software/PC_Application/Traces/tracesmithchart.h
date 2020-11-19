#ifndef TRACESMITHCHART_H
#define TRACESMITHCHART_H

#include "traceplot.h"
#include <QPen>
#include <QPainterPath>

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
    static constexpr int smithCoordMax = 4096;

    QPoint plotToPixel(std::complex<double> S);
    std::complex<double> pixelToPlot(const QPoint &pos);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void updateContextMenu() override;
    bool supported(Trace *t) override;
    void draw(QPainter * painter, double width_factor);
    void replot() override;
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

    double plotToPixelXOffset, plotToPixelXScale;
    double plotToPixelYOffset, plotToPixelYScale;
    TraceMarker *selectedMarker;
};

#endif // TRACESMITHCHART_H
