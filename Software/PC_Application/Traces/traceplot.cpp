#include "traceplot.h"
#include "tracemarker.h"

std::set<TracePlot*> TracePlot::plots;

TracePlot::TracePlot(TraceModel &model, QWidget *parent)
    : QWidget(parent),
      model(model)
{
    contextmenu = new QMenu();
    markedForDeletion = false;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lastUpdate = QTime::currentTime();
    sweep_fmin = std::numeric_limits<double>::lowest();
    sweep_fmax = std::numeric_limits<double>::max();
    // get notified when the span changes
    connect(&model, &TraceModel::SpanChanged, this, qOverload<double, double>(&TracePlot::updateSpan));
    plots.insert(this);
}

TracePlot::~TracePlot()
{
    plots.erase(this);
    delete contextmenu;
}

void TracePlot::enableTrace(Trace *t, bool enabled)
{
    if(traces[t] != enabled) {
        traces[t] = enabled;
        if(enabled) {
            // connect signals
            connect(t, &Trace::dataChanged, this, &TracePlot::triggerReplot);
            connect(t, &Trace::visibilityChanged, this, &TracePlot::triggerReplot);
            connect(t, &Trace::markerAdded, this, &TracePlot::markerAdded);
            connect(t, &Trace::markerRemoved, this, &TracePlot::markerRemoved);
        } else {
            // disconnect from notifications
            disconnect(t, &Trace::dataChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::visibilityChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::markerAdded, this, &TracePlot::markerAdded);
            disconnect(t, &Trace::markerRemoved, this, &TracePlot::markerRemoved);
        }
        updateContextMenu();
        triggerReplot();
    }
}

void TracePlot::mouseDoubleClickEvent(QMouseEvent *) {
    emit doubleClicked(this);
}

void TracePlot::updateSpan(double min, double max)
{
    sweep_fmin = min;
    sweep_fmax = max;
    triggerReplot();
}

void TracePlot::initializeTraceInfo()
{
    // Populate already present traces
    auto tvect = model.getTraces();
    for(auto t : tvect) {
        newTraceAvailable(t);
    }

    // connect notification of traces added at later point
    connect(&model, &TraceModel::traceAdded, this, &TracePlot::newTraceAvailable);
}

void TracePlot::contextMenuEvent(QContextMenuEvent *event)
{
    contextmenu->exec(event->globalPos());
    if(markedForDeletion) {
        emit deleted(this);
        delete this;
    }
}

std::set<TracePlot *> TracePlot::getPlots()
{
    return plots;
}

void TracePlot::newTraceAvailable(Trace *t)
{
    if(supported(t)) {
        traces[t] = false;
        connect(t, &Trace::deleted, this, &TracePlot::traceDeleted);
        connect(t, &Trace::nameChanged, this, &TracePlot::updateContextMenu);
        connect(t, &Trace::typeChanged, this, &TracePlot::updateContextMenu);
    }
    updateContextMenu();
}

void TracePlot::traceDeleted(Trace *t)
{
    enableTrace(t, false);
    traces.erase(t);
    updateContextMenu();
    triggerReplot();
}

void TracePlot::triggerReplot()
{
    auto now = QTime::currentTime();
    if (lastUpdate.msecsTo(now) >= MinUpdateInterval) {
        lastUpdate = now;
        replot();
    }
}

void TracePlot::markerAdded(TraceMarker *m)
{
    connect(m, &TraceMarker::dataChanged, this, &TracePlot::triggerReplot);
    connect(m, &TraceMarker::symbolChanged, this, &TracePlot::triggerReplot);
    triggerReplot();
}

void TracePlot::markerRemoved(TraceMarker *m)
{
    disconnect(m, &TraceMarker::dataChanged, this, &TracePlot::triggerReplot);
    disconnect(m, &TraceMarker::symbolChanged, this, &TracePlot::triggerReplot);
    triggerReplot();
}
