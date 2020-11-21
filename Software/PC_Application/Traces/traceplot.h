#ifndef TRACEPLOT_H
#define TRACEPLOT_H

#include <QWidget>
#include "tracemodel.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QTime>

class TracePlot : public QWidget
{
    Q_OBJECT
public:
    TracePlot(TraceModel &model, QWidget *parent = nullptr);
    ~TracePlot();

    virtual void enableTrace(Trace *t, bool enabled);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void updateSpan(double min, double max);

    static std::set<TracePlot *> getPlots();

signals:
    void doubleClicked(QWidget *w);
    void deleted(TracePlot*);

protected:
    static constexpr int MinUpdateInterval = 100;
    // need to be called in derived class constructor
    void initializeTraceInfo();
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void updateContextMenu(){};
    virtual bool supported(Trace *t) = 0;
    virtual void replot(){update();};
    virtual void draw(QPainter& p) = 0;
    std::map<Trace*, bool> traces;
    QMenu *contextmenu;
    QTime lastUpdate;
    bool markedForDeletion;
    static std::set<TracePlot*> plots;

    // handle trace drops
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

protected slots:
    void newTraceAvailable(Trace *t);
    void traceDeleted(Trace *t);
    void triggerReplot();
    virtual void markerAdded(TraceMarker *m);
    virtual void markerRemoved(TraceMarker *m);
protected:
    static constexpr unsigned int marginTop = 20;
    static constexpr unsigned int marginBottom = 0;
    static constexpr unsigned int marginLeft = 0;
    static constexpr unsigned int marginRight = 0;
    double sweep_fmin, sweep_fmax;
    TraceModel &model;

};

#endif // TRACEPLOT_H
