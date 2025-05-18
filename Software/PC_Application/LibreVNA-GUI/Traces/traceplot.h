#ifndef TRACEPLOT_H
#define TRACEPLOT_H

#include "tracemodel.h"
#include "savable.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QTime>
#include <QLabel>
#include <QWidget>

class TileWidget;

class TracePlot : public QWidget, public Savable
{
    Q_OBJECT
public:
    enum class Type {
        SmithChart,
        XYPlot,
        Waterfall,
        PolarChart,
        EyeDiagram,
    };

    TracePlot(TraceModel &model, QWidget *parent = nullptr);
    virtual ~TracePlot();

    void setParentTile(TileWidget *tile);

    virtual void enableTrace(Trace *t, bool enabled);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void updateSpan(double min, double max);
    virtual Type getType() = 0;
    static QString TypeToString(Type t);
    static Type TypeFromString(QString s);
    static TracePlot* createFromType(TraceModel &model, Type t);
    static TracePlot *createDefaultPlotForTrace(TraceModel &model, Trace *t);

    static std::set<TracePlot *> getPlots();

    TraceModel &getModel() const;

    bool getLimitPassing() const;

public slots:
    void updateGraphColors();

signals:
    void doubleClicked(QWidget *w);
    void deleted(TracePlot*);

protected:
    static constexpr int MinUpdateInterval = 100;
    static constexpr int MaxUpdateInterval = 2000;
    // need to be called in derived class constructor
    void initializeTraceInfo();
    std::vector<Trace*> activeTraces();
    // returns a list of all traces in alphabetical order
    std::vector<Trace*> orderedTraces();
    // changes the graph settings to make it possible to display a specific trace. The trace is not aded yet
    virtual bool configureForTrace(Trace *t) { Q_UNUSED(t) return false; } // default implementation fails for all traces
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void updateContextMenu(){}
    // adds common entries at bottom of context menu. Should be called at the end of derived udpateContextMenu functions
    void finishContextMenu();
    virtual void move(const QPoint &vect) { Q_UNUSED(vect) }
    virtual void zoom(const QPoint &center, double factor, bool horizontally, bool vertically) {Q_UNUSED(center)Q_UNUSED(factor)Q_UNUSED(horizontally)Q_UNUSED(vertically)}
    virtual void setAuto(bool horizontally, bool vertically) {Q_UNUSED(horizontally)Q_UNUSED(vertically)}
    virtual void replot(){update();}
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
    void wheelEvent(QWheelEvent *event) override;

    Marker *markerAtPosition(QPoint p, bool onlyMovable = false);
    virtual bool positionWithinGraphArea(const QPoint &p) {Q_UNUSED(p) return false;}

    void createMarkerAtPosition(QPoint p);

    // handle trace drops
    virtual bool dropSupported(Trace *t);
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    virtual void traceDropped(Trace *t, QPoint position);
    virtual QString mouseText(QPoint pos) {Q_UNUSED(pos) return QString();}
    QRect getDropRect();

    // save/load base class members. Should be called by derived classes in the toJSON/fromJSON functions
    nlohmann::json getBaseJSON();
    void parseBaseJSON(nlohmann::json j);

protected slots:
    void newTraceAvailable(Trace *t);
    void traceDeleted(Trace *t);
    void triggerReplot();
    void checkIfStillSupported(Trace *t);
    virtual void markerAdded(Marker *m);
    virtual void markerRemoved(Marker *m);
    virtual bool markerVisible(double x) = 0;
protected:
    static constexpr unsigned int marginBottom = 0;
    static constexpr unsigned int marginLeft = 0;
    static constexpr unsigned int marginRight = 0;

    static constexpr double dropOpacity = 0.9;
    static constexpr auto dropBackgroundColor = Qt::darkGray;
    static constexpr auto dropForegroundColor = Qt::white;
    static constexpr auto dropHighlightColor = Qt::red;

    double sweep_fmin, sweep_fmax;
    double xSweep; // current position in the sweep (NaN if no live traces are active on the plot)
    TraceModel &model;
    Marker *selectedMarker;
    bool movingGraph;
    QPoint lastMousePoint;
    TileWidget *parentTile;

    // graph settings have been changed, check and possibly remove incompatible traces before next paint event
    bool traceRemovalPending;

    bool dropPending;
    QPoint dropPosition;
    enum class DropSection {
        Above,
        Below,
        ToTheLeft,
        ToTheRight,
        OnPlot,
    };
    DropSection dropSection;

    Trace *dropTrace;

    QLabel *cursorLabel;

    unsigned int marginTop;

    bool limitPassing;

private:
    QString title;
};

#endif // TRACEPLOT_H
