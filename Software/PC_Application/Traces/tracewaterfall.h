#ifndef TRACEWATERFALL_H
#define TRACEWATERFALL_H

#include "traceplot.h"

#include "traceaxis.h"

#include <deque>

class TraceWaterfall : public TracePlot
{
    friend class WaterfallAxisDialog;
    Q_OBJECT
public:
    TraceWaterfall(TraceModel &model, QWidget *parent = 0);;

    virtual void enableTrace(Trace *t, bool enabled) override;

    void replot() override;
    virtual Type getType() override { return Type::Waterfall;};

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

public slots:
    void axisSetupDialog();
    void resetWaterfall();

protected:
    virtual bool configureForTrace(Trace *t) override;
    virtual void updateContextMenu() override;
    virtual void draw(QPainter& p) override;
    bool domainMatch(Trace *t);
    virtual bool supported(Trace *t) override;

    virtual QPoint markerToPixel(Marker *m) override { Q_UNUSED(m) return QPoint(0,0);};
    virtual double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override;

    virtual QString mouseText(QPoint pos) override;

protected slots:
    virtual bool markerVisible(double x) override;
    void traceDataChanged(unsigned int begin, unsigned int end);
private slots:
    void updateYAxis();
private:
    // color scale, input value from 0.0 to 1.0
    QColor getColor(double scale);

    enum class Direction {
        TopToBottom,
        BottomToTop,
    };

    // match alignment of XYplot with either one or both Y axes enabled
    enum class Alignment {
        PrimaryOnly = 0,
        SecondaryOnly = 1,
        BothAxes = 2,
        Last,
    };

    static QString AlignmentToString(Alignment a);
    static Alignment AlignmentFromString(QString s);

    Direction dir;
    Alignment align;

    Trace *trace;

    XAxis xAxis;
    YAxis yAxis;

    std::deque<std::vector<Trace::Data>> data;
    unsigned int pixelsPerLine;
    int plotAreaLeft, plotAreaWidth, plotAreaBottom, plotAreaTop;
    bool keepDataBeyondPlotSize;
    unsigned int maxDataSweeps;
};

#endif // TRACEWATERFALL_H
