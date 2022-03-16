#ifndef TRACEXYPLOT_H
#define TRACEXYPLOT_H

#include "traceplot.h"
#include "traceaxis.h"

#include <set>

class TraceXYPlot : public TracePlot
{
    friend class XYplotAxisDialog;
    Q_OBJECT
public:
    TraceXYPlot(TraceModel &model, QWidget *parent = nullptr);

    enum class XAxisMode {
        UseSpan,
        FitTraces,
        Manual,
        Last,
    };

    void setYAxis(int axis, YAxis::Type type, bool log, bool autorange, double min, double max, double div);
    void setXAxis(XAxis::Type type, XAxisMode mode, bool log, double min, double max, double div);
    void enableTrace(Trace *t, bool enabled) override;
    void updateSpan(double min, double max) override;
    void replot() override;

    virtual Type getType() override { return Type::XYPlot;};
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    bool isTDRtype(YAxis::Type type);

    static int sideMargin(bool YAxisEnabled);

public slots:
    void axisSetupDialog();

protected:
    virtual bool configureForTrace(Trace *t) override;
    virtual void updateContextMenu() override;
    virtual bool dropSupported(Trace *t) override;
    virtual void draw(QPainter &p) override;

private slots:
    void updateAxisTicks();
private:
    static constexpr int AxisLabelSize = 10;
    static constexpr int yAxisSpace = 55;
    static constexpr int yAxisDisabledSpace = 10;
    static constexpr int xAxisSpace = 30;
    static QString AxisModeToName(XAxisMode mode);
    static XAxisMode AxisModeFromName(QString name);
    void enableTraceAxis(Trace *t, int axis, bool enabled);
    bool domainMatch(Trace *t);
    bool supported(Trace *t) override;
    bool supported(Trace *t, YAxis::Type type);
    QPointF traceToCoordinate(Trace *t, unsigned int sample, YAxis &yaxis);
    QPoint plotValueToPixel(QPointF plotValue, int Yaxis);
    QPointF pixelToPlotValue(QPoint pixel, int YAxis);
    QPoint markerToPixel(Marker *m) override;
    double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override;
    virtual bool markerVisible(double x) override;
    void traceDropped(Trace *t, QPoint position) override;
    QString mouseText(QPoint pos) override;

    std::set<Trace*> tracesAxis[2];

    YAxis yAxis[2];
    XAxis xAxis;
    XAxisMode xAxisMode;

    int plotAreaLeft, plotAreaWidth, plotAreaBottom, plotAreaTop;
};

#endif // TRACEXYPLOT_H
