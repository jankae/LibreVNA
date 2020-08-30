#ifndef TRACEBODEPLOT_H
#define TRACEBODEPLOT_H

#include "traceplot.h"
#include <set>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_plot_marker.h>

class TraceBodePlot : public TracePlot
{
    friend class BodeplotAxisDialog;
    Q_OBJECT
public:
    TraceBodePlot(TraceModel &model, QWidget *parent = nullptr);
    ~TraceBodePlot();

    enum class YAxisType {
        Disabled = 0,
        Magnitude = 1,
        Phase = 2,
        VSWR = 3,
        Last,
    };

    virtual void setXAxis(double min, double max) override;
    void setYAxis(int axis, YAxisType type, bool log, bool autorange, double min, double max, double div);
    void setXAxis(bool autorange, double min, double max, double div);
    void enableTrace(Trace *t, bool enabled) override;

protected:
    virtual void updateContextMenu();
    virtual bool supported(Trace *t);
    void replot() override;

private slots:
    void traceColorChanged(Trace *t);
    void markerAdded(TraceMarker *m) override;
    void markerRemoved(TraceMarker *m) override;
    void markerDataChanged(TraceMarker *m);
private:
    QString AxisTypeToName(YAxisType type);
    void enableTraceAxis(Trace *t, int axis, bool enabled);
    bool supported(Trace *t, YAxisType type);
    void updateXAxis();
    QwtSeriesData<QPointF> *createQwtSeriesData(Trace &t, int axis);

    std::set<Trace*> tracesAxis[2];

    class Axis {
    public:
        YAxisType type;
        bool log;
        bool autorange;
        double rangeMin;
        double rangeMax;
        double rangeDiv;
    };
    Axis YAxis[2];
    Axis XAxis;
    double sweep_fmin, sweep_fmax;

    using CurveData = struct {
        QwtPlotCurve *curve;
        QwtSeriesData<QPointF> *data;
    };

    std::map<Trace*, CurveData> curves[2];
    std::map<TraceMarker*, QwtPlotMarker*> markers;
    QwtPlot *plot;
    TraceMarker *selectedMarker;
    QwtPlotCurve *selectedCurve;
};

#endif // TRACEBODEPLOT_H
