#ifndef TRACEXYPLOT_H
#define TRACEXYPLOT_H

#include "traceplot.h"
#include <set>
//#include <qwt_plot.h>
//#include <qwt_plot_curve.h>
//#include <qwt_series_data.h>
//#include <qwt_plot_marker.h>
//#include <qwt_plot_grid.h>
//#include <qwt_plot_picker.h>

//// Derived plotpicker, exposing transformation functions
//class XYplotPicker : public QwtPlotPicker {
//    Q_OBJECT
//public:
//    XYplotPicker(int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QWidget *w)
//        : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, w) {};
//    QPoint plotToPixel(const QPointF &pos) {
//        return transform(pos);
//    }
//    QPointF pixelToPlot(const QPoint &pos) {
//        return invTransform(pos);
//    }
//};

class TraceXYPlot : public TracePlot
{
    friend class XYplotAxisDialog;
//    friend class QwtTraceSeries;
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
    enum class XAxisMode {
        UseSpan,
        FitTraces,
        Manual,
    };

    void setYAxis(int axis, YAxisType type, bool log, bool autorange, double min, double max, double div);
    void setXAxis(XAxisType type, XAxisMode mode, double min, double max, double div);
    void enableTrace(Trace *t, bool enabled) override;
    void updateSpan(double min, double max) override;

    // Applies potentially changed colors to all XY-plots
//    static void updateGraphColors();
    bool isTDRtype(YAxisType type);

public slots:
    void axisSetupDialog();

protected:
    virtual void updateContextMenu() override;
    virtual bool supported(Trace *t) override;
    virtual void draw(QPainter &p) override;

private slots:
    void updateAxisTicks();
//    void traceColorChanged(Trace *t);
//    void markerAdded(TraceMarker *m) override;
//    void markerRemoved(TraceMarker *m) override;
//    void markerDataChanged(TraceMarker *m);
//    void markerSymbolChanged(TraceMarker *m);

//    void clicked(const QPointF pos);
//    void moved(const QPointF pos);
private:
    static constexpr int AxisLabelSize = 10;
//    void setColorFromPreferences();
    QString AxisTypeToName(YAxisType type);
    void enableTraceAxis(Trace *t, int axis, bool enabled);
    bool supported(Trace *t, YAxisType type);
    void updateXAxis();
    double transformFrequencyY(std::complex<double> data, YAxisType type);
//    QwtSeriesData<QPointF> *createQwtSeriesData(Trace &t, int axis);

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

//    using CurveData = struct {
//        QwtPlotCurve *curve;
//        QwtSeriesData<QPointF> *data;
//    };

//    std::map<Trace*, CurveData> curves[2];
//    std::map<TraceMarker*, QwtPlotMarker*> markers;
//    QwtPlot *plot;
//    QwtPlotGrid *grid;
    TraceMarker *selectedMarker;
//    QwtPlotCurve *selectedCurve;

//    XYplotPicker *drawPicker;

    // keep track of all created plots for changing colors
//    static std::set<TraceXYPlot*> allPlots;
};

#endif // TRACEXYPLOT_H
