#include "traceplot.h"

#include "Marker/marker.h"
#include "unit.h"
#include "Marker/markermodel.h"
#include "preferences.h"
#include "Util/util.h"
#include "CustomWidgets/tilewidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMimeData>
#include <QDebug>
#include <QApplication>

std::set<TracePlot*> TracePlot::plots;

using namespace std;

TracePlot::TracePlot(TraceModel &model, QWidget *parent)
    : QWidget(parent),
      model(model),
      selectedMarker(nullptr),
      movingGraph(false),
      traceRemovalPending(false),
      dropPending(false),
      dropTrace(nullptr),
      marginTop(20),
      limitPassing(true)
{
    parentTile = nullptr;

    contextmenu = new QMenu();
    markedForDeletion = false;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lastUpdate = QTime::currentTime();
    replotTimer.setSingleShot(true);
    connect(&replotTimer, &QTimer::timeout, this, qOverload<>(&TracePlot::update));
    sweep_fmin = std::numeric_limits<double>::lowest();
    sweep_fmax = std::numeric_limits<double>::max();
    xSweep = std::numeric_limits<double>::quiet_NaN();
    // get notified when the span changes
    connect(&model, &TraceModel::SpanChanged, this, qOverload<double, double>(&TracePlot::updateSpan));
    plots.insert(this);

    cursorLabel = new QLabel("Test", this);
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

void TracePlot::setParentTile(TileWidget *tile)
{
    parentTile = tile;
    updateContextMenu();
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
            connect(t, &Trace::colorChanged, this, &TracePlot::triggerReplot);
        } else {
            // disconnect from notifications
            disconnect(t, &Trace::dataChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::visibilityChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::markerFormatChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::markerAdded, this, &TracePlot::markerAdded);
            disconnect(t, &Trace::markerRemoved, this, &TracePlot::markerRemoved);
            disconnect(t, &Trace::typeChanged, this, &TracePlot::checkIfStillSupported);
            disconnect(t, &Trace::colorChanged, this, &TracePlot::triggerReplot);
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

std::vector<Trace *> TracePlot::orderedTraces()
{
    std::vector<Trace*> ordered;
    for(auto t : traces) {
        ordered.push_back(t.first);
    }
    sort(ordered.begin(), ordered.end(), [](Trace *l, Trace *r) -> bool {
        return l->name() < r->name();
    });
    return ordered;
}

void TracePlot::contextMenuEvent(QContextMenuEvent *event)
{
    auto position = event->pos() - QPoint(marginLeft, marginTop);
    auto m = markerAtPosition(position);
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

    xSweep = std::numeric_limits<double>::quiet_NaN();
    for(auto t : traces) {
        if(!t.second) {
            continue;
        }
        Trace* tr = t.first;
        if(tr->getSource() == Trace::Source::Live && tr->isVisible() && !tr->isPaused()) {
            xSweep = model.getSweepPosition();
            break;
        }
    }

    Q_UNUSED(event)
    auto& pref = Preferences::getInstance();
    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);
    // fill background
    p.setBackground(QBrush(pref.Graphs.Color.background));
    p.fillRect(0, 0, width(), height(), QBrush(pref.Graphs.Color.background));

    // show names of active traces and marker data (if enabled)
    bool hasMarkerData = false;
    auto marginMarkerData = pref.Graphs.fontSizeMarkerData * 12.5;
    marginTop = pref.Graphs.fontSizeTraceNames + 8;
    int x = 1; // xcoordinate for the next trace name
    int y = marginTop; // ycoordinate for the next marker data
    auto areaTextTop = 5;
    auto labelMarginRight = 4;
    auto borderRadius = 5;
    for(auto t : traces) {
        if(!t.second || !t.first->isVisible()) {
            continue;
        }

        // Trace name
        auto textArea = QRect(x, areaTextTop, width() - x, marginTop);
        QFont font = p.font();
        font.setPixelSize(pref.Graphs.fontSizeTraceNames);
        p.setFont(font);
        p.setPen(t.first->color());
        auto space = " ";
        auto label = space + t.first->name() + space;
        QRectF usedLabelArea = p.boundingRect(textArea, 0, label);
        QPainterPath path;
        path.addRoundedRect(usedLabelArea, borderRadius, borderRadius);
        p.fillPath(path, t.first->color());
        p.drawPath(path);
        p.setPen(Util::getFontColorFromBackground(t.first->color()));
        p.drawText(textArea, 0, label);
        p.setPen(t.first->color());
        x += usedLabelArea.width()+labelMarginRight;

        auto tmarkers = t.first->getMarkers();
        std::vector<Marker*> vmarkers(tmarkers.begin(), tmarkers.end());
        sort(vmarkers.begin(), vmarkers.end(), [](Marker *l, Marker *r) -> bool {
            switch(Preferences::getInstance().Marker.sortOrder) {
            case PrefMarkerSortXCoord:
                return l->getPosition() < r->getPosition();
            case PrefMarkerSortNumber:
                return l->getNumber() < r->getNumber();
            case PrefMarkerSortTimestamp:
                return l->getCreationTimestamp() < r->getCreationTimestamp();
            }
            return false;
        });
        for(auto m : vmarkers) {
            if(!m->isVisible()) {
                continue;
            }
            if(!markerVisible(m->getPosition())) {
                // marker not visible with current plot settings
                continue;
            }
            if(m->getGraphDisplayFormats().size() == 0) {
                // this marker has nothing to display
                continue;
            }
            hasMarkerData = true;
            QFont font = p.font();
            font.setPixelSize(pref.Graphs.fontSizeMarkerData);
            p.setFont(font);

            // Rounded box
            auto space = "  ";
            auto textArea = QRect(width() - marginRight - marginMarkerData, y, width() - marginRight, y + 100);
            auto label = space + QString::number(m->getNumber()) + space;
            QRectF textAreaConsumed = p.boundingRect(textArea, 0, label);
            QPainterPath pathM;
            pathM.addRoundedRect(textAreaConsumed, borderRadius, borderRadius);
            p.fillPath(pathM, t.first->color());
            p.drawPath(pathM);

            // Over box
            p.setPen(Util::getFontColorFromBackground(t.first->color()));
            p.drawText(textArea, 0, label);

            // Non-rounded description
            auto description = m->getSuffix() + space + m->readablePosition();
            p.setPen(t.first->color());
            p.drawText(width() - marginRight - marginMarkerData + textAreaConsumed.width() + 5, textAreaConsumed.y(), width() - marginRight, textArea.height(), 0, description);
            y += textAreaConsumed.height();

            for(auto f : m->getGraphDisplayFormats()) {
                auto textArea = QRect(width() - marginRight - marginMarkerData, y, width() - marginRight, y + 100);
                p.drawText(textArea, 0, m->readableData(f), &textAreaConsumed);
                y += textAreaConsumed.height();
            }
            // leave one row empty between markers
            y += textAreaConsumed.height();
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
    replotTimer.start(MaxUpdateInterval);
}

void TracePlot::finishContextMenu()
{
    contextmenu->addSeparator();
    if(parentTile) {
        auto add = new QMenu("Add tile...", contextmenu);
        auto left = new QAction("to the left", contextmenu);
        connect(left, &QAction::triggered, [=](){
            // split, keep current graph on the right
            parentTile->splitHorizontally(true);
        });
        add->addAction(left);
        auto right = new QAction("to the right", contextmenu);
        connect(right, &QAction::triggered, [=](){
            // split, keep current graph on the left
            parentTile->splitHorizontally(false);
        });
        add->addAction(right);
        auto above = new QAction("above", contextmenu);
        connect(above, &QAction::triggered, [=](){
            // split, keep current graph on the bottom
            parentTile->splitVertically(true);
        });
        add->addAction(above);
        auto below = new QAction("below", contextmenu);
        connect(below, &QAction::triggered, [=](){
            // split, keep current graph on the top
            parentTile->splitVertically(false);
        });
        add->addAction(below);
        contextmenu->addMenu(add);
    }

    auto close = new QAction("Close", contextmenu);
    contextmenu->addAction(close);
    connect(close, &QAction::triggered, [=]() {
        markedForDeletion = true;
    });
}


void TracePlot::mousePressEvent(QMouseEvent *event)
{
    auto &pref = Preferences::getInstance();
    auto position = event->pos() - QPoint(marginLeft, marginTop);
    if(event->buttons() == Qt::LeftButton) {
        selectedMarker = markerAtPosition(position, true);
        if(!selectedMarker && pref.Graphs.enablePanAndZoom && positionWithinGraphArea(position)) {
            // no marker at the position, enter trace moving mode
            movingGraph = true;
            lastMousePoint = position;
            cursorLabel->hide();
        }
    } else {
        selectedMarker = nullptr;
    }
    if(pref.Graphs.enablePanAndZoom && event->button() == Qt::MiddleButton) {
        bool horizontally = !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
        bool vertically = !(QApplication::keyboardModifiers() & Qt::ControlModifier);
        setAuto(horizontally, vertically);
    }
    event->accept();
}

void TracePlot::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    selectedMarker = nullptr;
    movingGraph = false;
    event->accept();
}

void TracePlot::mouseMoveEvent(QMouseEvent *event)
{
    auto clickPoint = event->pos() - QPoint(marginLeft, marginTop);
    if(selectedMarker) {
        auto trace = selectedMarker->getTrace();
        selectedMarker->setPosition(nearestTracePoint(trace, clickPoint));
        cursorLabel->hide();
    } else if(movingGraph) {
        move(clickPoint - lastMousePoint);
        lastMousePoint = clickPoint;
    } else {
        auto text = mouseText(clickPoint);
        if(!text.isEmpty()) {
            cursorLabel->setText(text);
            cursorLabel->adjustSize();
            cursorLabel->move(event->pos() + QPoint(15, 0));
            auto font = cursorLabel->font();
            font.setPixelSize(Preferences::getInstance().Graphs.fontSizeCursorOverlay);
            if(Util::getFontColorFromBackground(Preferences::getInstance().Graphs.Color.background) == Qt::black) {
                cursorLabel->setStyleSheet("color: black;");
            } else {
                cursorLabel->setStyleSheet("color: white;");
            }
            cursorLabel->setFont(font);
            cursorLabel->show();
        } else {
            cursorLabel->hide();
        }
    }
    event->accept();
}

void TracePlot::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    cursorLabel->hide();
    selectedMarker = nullptr;
    movingGraph = false;
    event->accept();
}

void TracePlot::wheelEvent(QWheelEvent *event)
{
    auto &pref = Preferences::getInstance();
    if(pref.Graphs.enablePanAndZoom && positionWithinGraphArea(event->pos())) {
        bool horizontally = !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
        bool vertically = !(QApplication::keyboardModifiers() & Qt::ControlModifier);
        double factor = pow(pref.Graphs.zoomFactor, (double) event->angleDelta().y() / 120.0);
        zoom(event->pos(), factor, horizontally, vertically);
        event->accept();
    }
}

Marker *TracePlot::markerAtPosition(QPoint p, bool onlyMovable)
{
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
            auto diff = markerPoint - p;
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

bool TracePlot::dropSupported(Trace *t)
{
    return supported(t);
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

void TracePlot::traceDropped(Trace *t, QPoint position)
{
    Q_UNUSED(t)
    Q_UNUSED(position);
    if(supported(t)) {
        enableTrace(t, true);
    }
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
            // remove non-supported traces after graph has been adjusted
            for(auto t : activeTraces()) {
                if(!supported(t)) {
                    enableTrace(t, false);
                }
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

bool TracePlot::getLimitPassing() const
{
    return limitPassing;
}

TraceModel &TracePlot::getModel() const
{
    return model;
}

void TracePlot::updateGraphColors()
{
    replot();
}
