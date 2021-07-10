#ifndef TRACEPLOT_H
#define TRACEPLOT_H

#include <QWidget>
#include "tracemodel.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QTime>
#include <QLabel>
#include "savable.h"

class TracePlot : public QWidget, public Savable
{
    Q_OBJECT
public:
    enum class Type {
        SmithChart,
        XYPlot,
    };

    TracePlot(TraceModel &model, QWidget *parent = nullptr);
    ~TracePlot();

    virtual void enableTrace(Trace *t, bool enabled);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void updateSpan(double min, double max);
    virtual Type getType() = 0;

    static std::set<TracePlot *> getPlots();

public slots:
    void updateGraphColors();

signals:
    void doubleClicked(QWidget *w);
    void deleted(TracePlot*);

protected:
    static constexpr int MinUpdateInterval = 100;
    // need to be called in derived class constructor
    void initializeTraceInfo();
    std::vector<Trace*> activeTraces();
    // changes the graph settings to make it possible to display a specific trace. The trace is not aded yet
    virtual bool configureForTrace(Trace *t) { Q_UNUSED(t) return false; }; // default implementation fails for all traces
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void updateContextMenu(){};
    virtual void replot(){update();};
    virtual void draw(QPainter& p) = 0;
    virtual bool supported(Trace *t) = 0;
    std::map<Trace*, bool> traces;
    QMenu *contextmenu;
    QPoint contextmenuClickpoint; // mouse coordinates when the contextmenu was invoked
    QTime lastUpdate;
    QTimer replotTimer;
    bool markedForDeletion;
    static std::set<TracePlot*> plots;

    virtual QPoint markerToPixel(Marker *m) = 0;
    virtual double nearestTracePoint(Trace *t, QPoint pixel, double *distance = nullptr) = 0;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    Marker *markerAtPosition(QPoint p, bool onlyMovable = false);

    void createMarkerAtPosition(QPoint p);

    // handle trace drops
    virtual bool dropSupported(Trace *t) = 0;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    virtual void traceDropped(Trace *t, QPoint position){  Q_UNUSED(t) Q_UNUSED(position)};
    virtual QString mouseText(QPoint pos) {Q_UNUSED(pos) return QString();};

protected slots:
    void newTraceAvailable(Trace *t);
    void traceDeleted(Trace *t);
    void triggerReplot();
    void checkIfStillSupported(Trace *t);
    virtual void markerAdded(Marker *m);
    virtual void markerRemoved(Marker *m);
    virtual bool xCoordinateVisible(double x) = 0;
protected:
    static constexpr unsigned int marginTop = 20;
    static constexpr unsigned int marginBottom = 0;
    static constexpr unsigned int marginLeft = 0;
    static constexpr unsigned int marginRight = 0;

    static constexpr unsigned int marginMarkerData = 150;

    double sweep_fmin, sweep_fmax;
    TraceModel &model;
    Marker *selectedMarker;

    // graph settings have been changed, check and possibly remove incompatible traces before next paint event
    bool traceRemovalPending;

    bool dropPending;
    QPoint dropPosition;
    Trace *dropTrace;

    QLabel *cursorLabel;

};

#endif // TRACEPLOT_H
