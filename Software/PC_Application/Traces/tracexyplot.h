#ifndef TRACEXYPLOT_H
#define TRACEXYPLOT_H

#include "traceplot.h"
#include <set>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>

// Derived plotpicker, exposing transformation functions
class XYplotPicker : public QwtPlotPicker {
    Q_OBJECT
public:
    XYplotPicker(int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QWidget *w)
        : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, w) {};
    QPoint plotToPixel(const QPointF &pos) {
        return transform(pos);
    }
    QPointF pixelToPlot(const QPoint &pos) {
        return invTransform(pos);
    }
};

class TraceXYPlot : public TracePlot
{
    friend class XYplotAxisDialog;
    Q_OBJECT
public:
    TraceXYPlot(TraceModel &model, QWidget *parent = nullptr);
    ~TraceXYPlot();

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

    virtual void setXAxis(double min, double max) override;
    void setYAxis(int axis, YAxisType type, bool log, bool autorange, double min, double max, double div);
    void setXAxis(XAxisType type, bool autorange, double min, double max, double div);
    void enableTrace(Trace *t, bool enabled) override;

    // Applies potentially changed colors to all XY-plots
    static void updateGraphColors();

    bool isTDRtype(YAxisType type);

protected:
    virtual void updateContextMenu() override;
    virtual bool supported(Trace *t) override;
    void replot() override;

private slots:
    void traceColorChanged(Trace *t);
    void markerAdded(TraceMarker *m) override;
    void markerRemoved(TraceMarker *m) override;
    void markerDataChanged(TraceMarker *m);
    void markerSymbolChanged(TraceMarker *m);

    void clicked(const QPointF pos);
    void moved(const QPointF pos);
private:
    void setColorFromPreferences();
    QString AxisTypeToName(YAxisType type);
    void enableTraceAxis(Trace *t, int axis, bool enabled);
    bool supported(Trace *t, YAxisType type);
    void updateXAxis();
    QwtSeriesData<QPointF> *createQwtSeriesData(Trace &t, int axis);

    std::set<Trace*> tracesAxis[2];

    class Axis {
    public:
        union {
            YAxisType Ytype;
            XAxisType Xtype;
        };
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
    QwtPlotGrid *grid;
    TraceMarker *selectedMarker;
    QwtPlotCurve *selectedCurve;

    XYplotPicker *drawPicker;

    // keep track of all created plots for changing colors
    static std::set<TraceXYPlot*> allPlots;
};

#endif // TRACEXYPLOT_H
