#include "traceplot.h"
#include "Marker/marker.h"
#include "preferences.h"
#include <QPainter>
#include <QMimeData>
#include <QDebug>
#include "unit.h"
#include "Marker/markermodel.h"

std::set<TracePlot*> TracePlot::plots;

using namespace std;

TracePlot::TracePlot(TraceModel &model, QWidget *parent)
    : QWidget(parent),
      model(model),
      selectedMarker(nullptr),
      traceRemovalPending(false),
      dropPending(false),
      dropTrace(nullptr)
{
    contextmenu = new QMenu();
    markedForDeletion = false;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lastUpdate = QTime::currentTime();
    replotTimer.setSingleShot(true);
    connect(&replotTimer, &QTimer::timeout, this, qOverload<>(&TracePlot::update));
    sweep_fmin = std::numeric_limits<double>::lowest();
    sweep_fmax = std::numeric_limits<double>::max();
    // get notified when the span changes
    connect(&model, &TraceModel::SpanChanged, this, qOverload<double, double>(&TracePlot::updateSpan));
    plots.insert(this);

    cursorLabel = new QLabel("Test", this);
    cursorLabel->setStyleSheet("color: white;");
    auto font = cursorLabel->font();
    font.setPixelSize(12);
    cursorLabel->setFont(font);
    cursorLabel->hide();
    setMouseTracking(true);
    setAcceptDrops(true);
}

TracePlot::~TracePlot()
{
    plots.erase(this);
    delete contextmenu;
    delete cursorLabel;
}

void TracePlot::enableTrace(Trace *t, bool enabled)
{
    if(traces[t] != enabled) {
        traces[t] = enabled;
        if(enabled) {
            // connect signals
            connect(t, &Trace::dataChanged, this, &TracePlot::triggerReplot);
            connect(t, &Trace::visibilityChanged, this, &TracePlot::triggerReplot);
            connect(t, &Trace::markerFormatChanged, this, &TracePlot::triggerReplot);
            connect(t, &Trace::markerAdded, this, &TracePlot::markerAdded);
            connect(t, &Trace::markerRemoved, this, &TracePlot::markerRemoved);
            connect(t, &Trace::typeChanged, this, &TracePlot::checkIfStillSupported);
        } else {
            // disconnect from notifications
            disconnect(t, &Trace::dataChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::visibilityChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::markerFormatChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::markerAdded, this, &TracePlot::markerAdded);
            disconnect(t, &Trace::markerRemoved, this, &TracePlot::markerRemoved);
            disconnect(t, &Trace::typeChanged, this, &TracePlot::checkIfStillSupported);
        }
        updateContextMenu();
        replot();
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

std::vector<Trace *> TracePlot::activeTraces()
{
    std::vector<Trace*> ret;
    for(auto t : traces) {
        if(t.second) {
            ret.push_back(t.first);
        }
    }
    return ret;
}

void TracePlot::contextMenuEvent(QContextMenuEvent *event)
{
    auto m = markerAtPosition(event->pos());
    QMenu *menu;
    if(m) {
        // right click on marker, execute its contextmenu
        menu = m->getContextMenu();
    } else {
        // no marker, contextmenu of graph
        menu = contextmenu;
        contextmenuClickpoint = event->pos();
    }
    menu->exec(event->globalPos());
    if(markedForDeletion) {
        emit deleted(this);
        delete this;
    }
}

void TracePlot::paintEvent(QPaintEvent *event)
{
    if(traceRemovalPending) {
        for(auto t : traces) {
            if(!t.second) {
                // trace already disabled
            }
            if(!supported(t.first)) {
                enableTrace(t.first, false);
            }
        }
        traceRemovalPending = false;
    }

    Q_UNUSED(event)
    auto pref = Preferences::getInstance();
    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);
    // fill background
    p.setBackground(QBrush(pref.Graphs.Color.background));
    p.fillRect(0, 0, width(), height(), QBrush(pref.Graphs.Color.background));

    // show names of active traces and marker data (if enabled)
    bool hasMarkerData = false;
    int x = 1; // xcoordinate for the next trace name
    int y = marginTop; // ycoordinate for the next marker data
    for(auto t : traces) {
        if(!t.second || !t.first->isVisible()) {
            continue;
        }
        auto textArea = QRect(x, 0, width() - x, marginTop);
        QRect usedArea;
        QFont font = p.font();
        font.setPixelSize(12);
        p.setFont(font);
        p.setPen(t.first->color());
        p.drawText(textArea, 0, t.first->name() + " ", &usedArea);
        x += usedArea.width();

        auto tmarkers = t.first->getMarkers();

        font.setPixelSize(12);
        p.setFont(font);
        for(auto m : tmarkers) {
            if(!xCoordinateVisible(m->getPosition())) {
                // marker not visible with current plot settings
                continue;
            }
            if(m->getGraphDisplayFormats().size() == 0) {
                // this marker has nothing to display
                continue;
            }
            hasMarkerData = true;

            auto textArea = QRect(width() - marginRight - marginMarkerData, y, width() - marginRight, y + 100);
            p.drawText(textArea, 0, "Marker "+QString::number(m->getNumber())+m->getSuffix()+": "+m->readablePosition(), &usedArea);
            y += usedArea.height();

            for(auto f : m->getGraphDisplayFormats()) {
                auto textArea = QRect(width() - marginRight - marginMarkerData, y, width() - marginRight, y + 100);
                p.drawText(textArea, 0, m->readableData(f), &usedArea);
                y += usedArea.height();
            }
            // leave one row empty between markers
            y += usedArea.height();
        }
    }

    unsigned int l = marginLeft;
    unsigned int t = marginTop;
    unsigned int w = width() - marginLeft - marginRight;
    unsigned int h = height() - marginTop - marginBottom;

    if(hasMarkerData) {
        w -= marginMarkerData;
    }

    p.setViewport(l, t, w, h);
    p.setWindow(0, 0, w, h);

    draw(p);
}


void TracePlot::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton) {
        selectedMarker = markerAtPosition(event->pos(), true);
    } else {
        selectedMarker = nullptr;
    }
}

void TracePlot::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    selectedMarker = nullptr;
}

void TracePlot::mouseMoveEvent(QMouseEvent *event)
{
    auto clickPoint = event->pos() - QPoint(marginLeft, marginTop);
    if(selectedMarker) {
        auto trace = selectedMarker->getTrace();
        selectedMarker->setPosition(nearestTracePoint(trace, clickPoint));
        cursorLabel->hide();
    } else {
        auto text = mouseText(clickPoint);
        if(!text.isEmpty()) {
            cursorLabel->setText(text);
            cursorLabel->adjustSize();
            cursorLabel->move(event->pos() + QPoint(15, 0));
            cursorLabel->show();
        } else {
            cursorLabel->hide();
        }
    }
}

void TracePlot::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    cursorLabel->hide();
    selectedMarker = nullptr;
}

Marker *TracePlot::markerAtPosition(QPoint p, bool onlyMovable)
{
    auto clickPoint = p - QPoint(marginLeft, marginTop);
    // check if click was near a marker
    unsigned int closestDistance = numeric_limits<unsigned int>::max();
    Marker *closestMarker = nullptr;
    for(auto t : traces) {
        if(!t.second) {
            // this trace is disabled, skip
            continue;
        }
        auto markers = t.first->getMarkers();
        for(auto m : markers) {
            if(!m->isMovable() && onlyMovable) {
                continue;
            }
            auto markerPoint = markerToPixel(m);
            if(markerPoint.isNull()) {
                // invalid, skip
                continue;
            }
            auto diff = markerPoint - clickPoint;
            unsigned int distance = diff.x() * diff.x() + diff.y() * diff.y();
            if(distance < closestDistance) {
                closestDistance = distance;
                if(m->getParent()) {
                    closestMarker = m->getParent();
                } else {
                    closestMarker = m;
                }
            }
        }
    }
    if(closestDistance <= 400) {
        return closestMarker;
    } else {
        return nullptr;
    }
}

void TracePlot::createMarkerAtPosition(QPoint p)
{
    // transate from point in absolute coordinates to this widget
    p -= QPoint(marginLeft, marginTop);

    double closestDistance = std::numeric_limits<double>::max();
    Trace *trace = nullptr;
    double xpos;
    for(auto t : traces) {
        if(!t.second) {
            // trace not enabled, skip
            continue;
        }
        double distance;
        auto x = nearestTracePoint(t.first, p, &distance);
        if(distance < closestDistance) {
            trace = t.first;
            xpos = x;
            closestDistance = distance;
        }
    }
    if(!trace) {
        // failed to find trace (should not happen)
        return;
    }

    auto markerModel = model.getMarkerModel();
    auto marker = markerModel->createDefaultMarker();
    marker->assignTrace(trace);
    marker->setPosition(xpos);
    markerModel->addMarker(marker);
}

void TracePlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("trace/pointer")) {
        auto data = event->mimeData()->data("trace/pointer");
        QDataStream stream(&data, QIODevice::ReadOnly);
        quintptr dropPtr;
        stream >> dropPtr;
        auto trace = (Trace*) dropPtr;
        if(dropSupported(trace)) {
            event->acceptProposedAction();
            dropPending = true;
            dropTrace = trace;
        }
    }
    triggerReplot();
}

void TracePlot::dropEvent(QDropEvent *event)
{
    if(dropTrace) {
        traceDropped(dropTrace, event->pos() -  - QPoint(marginLeft, marginTop));
    }
    dropPending = false;
    dropTrace = nullptr;
    replot();
}

void TracePlot::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    dropPending = false;
    dropTrace = nullptr;
    replot();
}

std::set<TracePlot *> TracePlot::getPlots()
{
    return plots;
}

void TracePlot::newTraceAvailable(Trace *t)
{
    traces[t] = false;
    connect(t, &Trace::deleted, this, &TracePlot::traceDeleted);
    connect(t, &Trace::nameChanged, this, &TracePlot::updateContextMenu);
    connect(t, &Trace::typeChanged, this, &TracePlot::updateContextMenu);
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
    } else {
        replotTimer.start(MinUpdateInterval);
    }
}

void TracePlot::checkIfStillSupported(Trace *t)
{
    if(!supported(t)) {
        // something with this trace changed and it can no longer be displayed on this graph
        // behavior depends on preferences
        switch(Preferences::getInstance().Graphs.domainChangeBehavior) {
        case GraphDomainChangeBehavior::RemoveChangedTraces:
            // simply remove the changed trace
            enableTrace(t, false);
            break;
        case GraphDomainChangeBehavior::AdjustGrahpsIfOnlyTrace:
            // remove trace if other traces are present, otherwise try to adjust graph
            if(activeTraces().size() > 1) {
                enableTrace(t, false);
                break;
            }
            [[fallthrough]];
        case GraphDomainChangeBehavior::AdjustGraphs:
            // attempt to configure the graph for the changed trace, remove only if this fails
            if(!configureForTrace(t)) {
                enableTrace(t, false);
            }
            break;
        }

    }
}

void TracePlot::markerAdded(Marker *m)
{
    connect(m, &Marker::dataChanged, this, &TracePlot::triggerReplot);
    connect(m, &Marker::symbolChanged, this, &TracePlot::triggerReplot);
    triggerReplot();
}

void TracePlot::markerRemoved(Marker *m)
{
    disconnect(m, &Marker::dataChanged, this, &TracePlot::triggerReplot);
    disconnect(m, &Marker::symbolChanged, this, &TracePlot::triggerReplot);
    triggerReplot();
}

void TracePlot::updateGraphColors()
{
    replot();
}
