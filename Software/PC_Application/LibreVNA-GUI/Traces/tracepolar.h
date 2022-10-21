#ifndef TRACEPOLAR_H
#define TRACEPOLAR_H

#include "traceplot.h"

class PolarArc {
public:
    PolarArc(QPointF center, double radius, double startAngle = 0.0, double spanAngle = 2*M_PI);
    void constrainToCircle(QPointF center, double radius);
    QPointF center;
    double radius;
    double startAngle, spanAngle;
};

class TracePolar : public TracePlot
{
    Q_OBJECT
public:
    TracePolar(TraceModel &model, QWidget *parent = 0);

    virtual nlohmann::json toJSON() override; // derived classes must call TracePolar::joJSON before doing anything
    virtual void fromJSON(nlohmann::json j) override; // derived classes must call TracePolar::joJSON before doing anything

    void wheelEvent(QWheelEvent *event) override;

public slots:
    virtual void axisSetupDialog() {}

protected:
    static constexpr double polarCoordMax = 4096;

    virtual std::complex<double> dataAddDx(std::complex<double> d);
    virtual Trace::Data dataAddDx(Trace::Data d);

    QPoint dataToPixel(std::complex<double> d);
    QPoint dataToPixel(Trace::Data d);
    std::complex<double> pixelToData(QPoint p);

    QPoint markerToPixel(Marker *m) override;
    double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override;
    virtual bool markerVisible(double x) override;

    virtual void updateContextMenu() override;
    virtual bool supported(Trace *t) override {Q_UNUSED(t) return false;}

    // given two points and a circle, the two points are adjusted in such a way that the line they describe
    // is constrained within the circle. Returns true if there is a remaining line segment in the circle, false
    // if the line lies completely outside of the circle (or is tangent to the circle)
    static bool constrainLineToCircle(QPointF &a, QPointF &b, QPointF center, double radius);

    bool limitToSpan;
    bool limitToEdge;
    bool manualFrequencyRange;
    double fmin, fmax; // frequency range when manual range is selected

    double edgeReflection; // magnitude of reflection coefficient at the edge of the polar chart (zoom factor)
    double dx;
    QTransform transform;
};

#endif // TRACEPOLAR_H
