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

    virtual Type getType() override { return Type::SmithChart;};

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void wheelEvent(QWheelEvent *event) override;
public slots:
    void axisSetupDialog();

protected:
    static constexpr double ReferenceImpedance = 50.0;
    static constexpr double screenUsage = 0.9;
    static constexpr double smithCoordMax = 4096;

    class Arc
    {
    public:
        Arc(QPointF center, double radius, double startAngle, double spanAngle);
        void constrainToCircle(QPointF center, double radius);

        QPointF center;
        double radius;
        double startAngle, spanAngle;
    };

    QPoint dataToPixel(std::complex<double> d);
    QPoint dataToPixel(Trace::Data d);
    std::complex<double> pixelToData(QPoint p);
    QPoint markerToPixel(Marker *m) override;
    double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override;
    virtual bool xCoordinateVisible(double x);

    //void paintEvent(QPaintEvent *event) override;
    virtual void updateContextMenu() override;
    bool supported(Trace *t) override;
    bool dropSupported(Trace *t) override;
    virtual void draw(QPainter& painter) override;
    virtual void traceDropped(Trace *t, QPoint position) override;
    QString mouseText(QPoint pos) override;
    bool limitToSpan;
    bool limitToEdge;
    double edgeReflection; // magnitude of reflection coefficient at the edge of the smith chart (zoom factor)
    QTransform transform;
};

#endif // TRACESMITHCHART_H
