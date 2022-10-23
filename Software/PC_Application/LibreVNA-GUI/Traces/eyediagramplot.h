#ifndef EYEDIAGRAMPLOT_H
#define EYEDIAGRAMPLOT_H

#include "traceplot.h"
#include "traceaxis.h"
#include "Traces/Math/tdr.h"

#include <mutex>
#include <QThread>
#include <QSemaphore>

#include <QObject>

class EyeDiagramPlot;

class EyeThread : public QThread
{
    Q_OBJECT
public:
    EyeThread(EyeDiagramPlot &eye) : eye(eye) {}
    ~EyeThread(){}
private:
    void run() override;
    EyeDiagramPlot &eye;
};

class EyeDiagramPlot : public TracePlot
{
    friend class EyeThread;
    Q_OBJECT
public:
    EyeDiagramPlot(TraceModel &model, QWidget *parent = 0);
    ~EyeDiagramPlot();

    virtual void enableTrace(Trace *t, bool enabled) override;
    void replot() override;

    virtual void move(const QPoint &vect) override;
    virtual void zoom(const QPoint &center, double factor, bool horizontally, bool vertically) override;
    virtual void setAuto(bool horizontally, bool vertically) override;

    virtual Type getType() override { return Type::EyeDiagram;}

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

public slots:
    void axisSetupDialog();
signals:
    void statusChanged(QString);

protected:
    virtual void updateContextMenu() override;
    virtual bool positionWithinGraphArea(const QPoint &p) override;
    virtual void draw(QPainter& p) override;
    virtual bool supported(Trace *t) override;

    virtual QPoint markerToPixel(Marker *m) override { Q_UNUSED(m) return QPoint(0,0);} // never used but most be implemented
    virtual double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) override {Q_UNUSED(t)Q_UNUSED(pixel)Q_UNUSED(distance)return 0.0;} // never used but most be implemented

    virtual QString mouseText(QPoint pos) override;

protected slots:
    virtual bool markerVisible(double x) override {Q_UNUSED(x)return false;} // never show markers

private slots:
    void triggerUpdate();
private:
    static constexpr double yOverrange = 0.2;
    QPoint plotValueToPixel(QPointF plotValue);
    QPointF pixelToPlotValue(QPoint pixel);
    void setStatus(QString s);
    double calculatedTime();
    double minDisplayVoltage();
    double maxDisplayVoltage();

    Math::TDR *tdr;

    Trace *trace;

    XAxis xAxis;
    YAxis yAxis;

    class Xdata {
    public:
        double x;
        std::vector<double> y;
    };

    std::vector<Xdata> data[2];
    std::vector<Xdata> *displayData;
    std::vector<Xdata> *calcData;

    unsigned int xSamples;
    double datarate;
    double highlevel;
    double lowlevel;
    unsigned int bitsPerSymbol;
    double risetime;
    double falltime;
    double noise;
    double jitter;
    bool linearEdge;
    unsigned int patternbits;
    unsigned int cycles;
    int traceBlurring;

    int plotAreaLeft, plotAreaWidth, plotAreaBottom, plotAreaTop;

    QString status;

    std::mutex bufferSwitchMutex;
    std::mutex calcMutex;

    EyeThread *thread;
    bool destructing;
    QSemaphore semphr;
};

#endif // EYEDIAGRAMPLOT_H
