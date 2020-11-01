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
    TracePlot( QWidget *parent = nullptr);
    ~TracePlot();

    virtual void enableTrace(Trace *t, bool enabled);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void updateSpan(double min, double max){Q_UNUSED(min);Q_UNUSED(max)};

    static std::set<TracePlot *> getPlots();

signals:
    void doubleClicked(QWidget *w);
    void deleted(TracePlot*);

protected:
//    static const QColor Background;// = QColor(0,0,0);
//    static const QColor Border;// = QColor(255,255,255);
//    static const QColor Divisions;// = QColor(255,255,255);
    static constexpr int MinUpdateInterval = 100;
    // need to be called in derived class constructor
    void initializeTraceInfo(TraceModel &model);
    void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void updateContextMenu();
    virtual bool supported(Trace *t) = 0;
    virtual void replot(){};
    std::map<Trace*, bool> traces;
    QMenu *contextmenu;
    QTime lastUpdate;
    bool markedForDeletion;

    static std::set<TracePlot*> plots;

protected slots:
    void newTraceAvailable(Trace *t);
    void traceDeleted(Trace *t);
    void triggerReplot();
    virtual void markerAdded(TraceMarker *m);
    virtual void markerRemoved(TraceMarker *m);

};

#endif // TRACEPLOT_H
