#ifndef TRACEPOLARCHART_H
#define TRACEPOLARCHART_H

#include "traceplot.h"

class PolarChartCircle
{
public:
    PolarChartCircle(QPointF center, double radius, double startAngle = 0.0, double spanAngle = 2*M_PI);
    QPointF center;
    double radius;
    double startAngle, spanAngle;
};


class TracePolarChart : public TracePlot
{
    Q_OBJECT
public:
    TracePolarChart(TraceModel &model, QWidget *parent = 0);

    virtual Type getType() override { return Type::PolarChart;};

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void wheelEvent(QWheelEvent *event) override;
public slots:
    void axisSetupDialog();

private:
    static constexpr double polarCoordMax = 4096;

    std::complex<double> dataAddDx(std::complex<double> d);
    Trace::Data dataAddDx(Trace::Data d);

    QPoint dataToPixel(std::complex<double> d);
    QPoint dataToPixel(Trace::Data d);
    std::complex<double> pixelToData(QPoint p);
    QPoint markerToPixel(Marker *m) override;
    double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override;
    virtual bool markerVisible(double x);

    virtual void updateContextMenu() override;
    bool supported(Trace *t) override;
    virtual void draw(QPainter& painter) override;
    virtual bool dropSupported(Trace *t) override;
    QString mouseText(QPoint pos) override;
    bool limitToSpan;
    bool limitToEdge;
    double edgeReflection;
    double dx;
    QTransform transform;
};

#endif // TRACEPOLARCHART_H
