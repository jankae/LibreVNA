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

    virtual void move(const QPoint &vect) override;
    virtual void zoom(const QPoint &center, double factor, bool horizontally, bool vertically) override;
    virtual void setAuto(bool horizontally, bool vertically) override;

//    void wheelEvent(QWheelEvent *event) override;

public slots:
    virtual void axisSetupDialog() {}

protected:
    static constexpr double polarCoordMax = 4096;

    virtual bool positionWithinGraphArea(const QPoint &p) override;
    virtual std::complex<double> dataAddOffset(std::complex<double> d);
    virtual Trace::Data dataAddOffset(Trace::Data d);

    QPoint dataToPixel(QPointF d);
    QPoint dataToPixel(std::complex<double> d);
    QPoint dataToPixel(Trace::Data d);
    std::complex<double> pixelToData(QPoint p);

    QPoint markerToPixel(Marker *m) override;
    double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override;
    virtual bool markerVisible(double x) override;

    virtual void updateContextMenu() override;
    virtual bool supported(Trace *t) override {Q_UNUSED(t) return false;}

    double minimumVisibleFrequency();
    double maximumVisibleFrequency();

    // given two points and a circle, the two points are adjusted in such a way that the line they describe
    // is constrained within the circle. Returns true if there is a remaining line segment in the circle, false
    // if the line lies completely outside of the circle (or is tangent to the circle)
    static bool constrainLineToCircle(QPointF &a, QPointF &b, QPointF center, double radius);

    bool limitToSpan;
    bool limitToEdge;
    bool manualFrequencyRange;
    double fmin, fmax; // frequency range when manual range is selected

    double edgeReflection; // magnitude of reflection coefficient at the edge of the polar chart (zoom factor)
    QPointF offset;
    QTransform transform;
};

#endif // TRACEPOLAR_H
