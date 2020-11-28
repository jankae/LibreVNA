#ifndef TRACEXYPLOT_H
#define TRACEXYPLOT_H

#include "traceplot.h"
#include <set>

class TraceXYPlot : public TracePlot
{
    friend class XYplotAxisDialog;
    Q_OBJECT
public:
    TraceXYPlot(TraceModel &model, QWidget *parent = nullptr);

    enum class YAxisType {
        Disabled = 0,
        // S parameter options
        Magnitude = 1,
        Phase = 2,
        VSWR = 3,
        // TDR options
        Impulse = 4,
        Step = 5,
        Impedance = 6,
        Last,
    };
    static const std::set<YAxisType> YAxisTypes;
    enum class XAxisType {
        Frequency,
        Time,
        Distance,
    };
    enum class XAxisMode {
        UseSpan,
        FitTraces,
        Manual,
    };

    void setYAxis(int axis, YAxisType type, bool log, bool autorange, double min, double max, double div);
    void setXAxis(XAxisType type, XAxisMode mode, double min, double max, double div);
    void enableTrace(Trace *t, bool enabled) override;
    void updateSpan(double min, double max) override;
    void replot() override;

    bool isTDRtype(YAxisType type);

public slots:
    void axisSetupDialog();

protected:
    virtual void updateContextMenu() override;
    virtual bool supported(Trace *t) override;
    virtual void draw(QPainter &p) override;

private slots:
    void updateAxisTicks();
private:
    static constexpr int AxisLabelSize = 10;
    QString AxisTypeToName(YAxisType type);
    void enableTraceAxis(Trace *t, int axis, bool enabled);
    bool supported(Trace *t, YAxisType type);
    void removeUnsupportedTraces();
    QPointF traceToCoordinate(Trace *t, unsigned int sample, YAxisType type);
//    QPoint dataToPixel(Trace::Data d);
    QPoint plotValueToPixel(QPointF plotValue, int Yaxis);
    QPointF pixelToPlotValue(QPoint pixel, int YAxis);
    QPoint markerToPixel(TraceMarker *m) override;
    double nearestTracePoint(Trace *t, QPoint pixel) override;
    void traceDropped(Trace *t, QPoint position) override;
    QString mouseText(QPoint pos) override;

    static QString AxisUnit(YAxisType type);
    static QString AxisUnit(XAxisType type);

    std::set<Trace*> tracesAxis[2];

    class YAxis {
    public:
        YAxisType type;
        bool log; // not used yet
        bool autorange;
        double rangeMin;
        double rangeMax;
        double rangeDiv;
        std::vector<double> ticks;
    };
    class XAxis {
    public:
        XAxisType type;
        XAxisMode mode;
        bool log; // not used yet
        double rangeMin;
        double rangeMax;
        double rangeDiv;
        std::vector<double> ticks;
    };

    YAxis YAxis[2];
    XAxis XAxis;

    int plotAreaLeft, plotAreaWidth, plotAreaBottom;
};

#endif // TRACEXYPLOT_H
