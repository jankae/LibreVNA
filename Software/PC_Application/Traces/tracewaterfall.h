#ifndef TRACEWATERFALL_H
#define TRACEWATERFALL_H

#include "traceplot.h"

class TraceWaterfall : public TracePlot
{
    Q_OBJECT
public:
    TraceWaterfall(TraceModel &model, QWidget *parent = 0);;

    virtual void enableTrace(Trace *t, bool enabled) override;

    void updateSpan(double min, double max) override;
    void replot() override;
    virtual Type getType() override { return Type::Waterfall;};

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

    enum class XAxisType {
        Frequency,
        Time,
        Distance,
        Power,
        Last,
    };
    enum class XAxisMode {
        UseSpan,
        FitTraces,
        Manual,
        Last,
    };

    void setXAxis(XAxisType type, XAxisMode mode, bool log, double min, double max, double div);


public slots:
    void axisSetupDialog();
    void resetWaterfall();

protected:
    virtual bool configureForTrace(Trace *t) override;
    virtual void updateContextMenu() override;
    virtual void draw(QPainter& p) override;
    virtual bool supported(Trace *t) override;

    virtual QPoint markerToPixel(Marker *m) override { Q_UNUSED(m) return QPoint(0,0);};
    virtual double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override;

    virtual QString mouseText(QPoint pos) override;

protected slots:
    virtual bool xCoordinateVisible(double x) override;
private slots:
    void updateAxisTicks();
private:
    static constexpr int AxisLabelSize = 10;
    static QString AxisTypeToName(XAxisType type);
    static QString AxisModeToName(XAxisMode mode);
    static XAxisType XAxisTypeFromName(QString name);
    static XAxisMode AxisModeFromName(QString name);

    static QString AxisUnit(XAxisType type);

    class XAxis {
    public:
        XAxisType type;
        XAxisMode mode;
        bool log;
        double rangeMin;
        double rangeMax;
        double rangeDiv;
        std::vector<double> ticks;
    };

    XAxis XAxis;

    std::vector<std::vector<Trace::Data>> data;
    unsigned int pixelsPerLine;
    int plotAreaLeft, plotAreaWidth, plotAreaBottom, plotAreaTop;
};

#endif // TRACEWATERFALL_H
