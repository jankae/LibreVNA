#include "traceplot.h"

#include "Marker/marker.h"
#include "unit.h"
#include "Marker/markermodel.h"
#include "preferences.h"
#include "Util/util.h"
#include "CustomWidgets/tilewidget.h"
#include "tracexyplot.h"
#include "tracesmithchart.h"
#include "eyediagramplot.h"
#include "tracewaterfall.h"
#include "tracepolarchart.h"

#include <QPainter>
#include <QPainterPath>
#include <QMimeData>
#include <QDebug>
#include <QApplication>
#include <QInputDialog>

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
            connect(t, &Trace::deembeddingChanged, this, &TracePlot::checkIfStillSupported);
            connect(t, &Trace::colorChanged, this, &TracePlot::triggerReplot);
        } else {
            // disconnect from notifications
            disconnect(t, &Trace::dataChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::visibilityChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::markerFormatChanged, this, &TracePlot::triggerReplot);
            disconnect(t, &Trace::markerAdded, this, &TracePlot::markerAdded);
            disconnect(t, &Trace::markerRemoved, this, &TracePlot::markerRemoved);
            disconnect(t, &Trace::typeChanged, this, &TracePlot::checkIfStillSupported);
            disconnect(t, &Trace::deembeddingChanged, this, &TracePlot::checkIfStillSupported);
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

QString TracePlot::TypeToString(Type t)
{
    switch(t) {
    case Type::EyeDiagram: return "Eye Diagram";
    case Type::PolarChart: return "Polar Chart";
    case Type::SmithChart: return "Smith Chart";
    case Type::Waterfall: return "Waterfall";
    case Type::XYPlot: return "XY Plot";
    default: return "Invalid";
    }
}

TracePlot::Type TracePlot::TypeFromString(QString s)
{
    for(unsigned int i=0;i<=(int) Type::EyeDiagram;i++) {
        if(TypeToString((Type) i) == s) {
            return (Type) i;
        }
    }
    // use default
    return Type::XYPlot;
}

TracePlot *TracePlot::createFromType(TraceModel &model, Type t)
{
    switch(t) {
    case Type::EyeDiagram: return new EyeDiagramPlot(model);
    case Type::PolarChart: return new TracePolarChart(model);
    case Type::SmithChart: return new TraceSmithChart(model);
    case Type::Waterfall: return new TraceWaterfall(model);
    case Type::XYPlot: return new TraceXYPlot(model);
    default:
        return nullptr;
    }
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

    marginTop = 0;
    // draw title
    if(!title.isEmpty()) {
        QFont font = p.font();
        font.setPixelSize(pref.Graphs.fontSizeTitle);
        p.setFont(font);
        p.setPen(Util::getFontColorFromBackground(pref.Graphs.Color.background));
        p.drawText(QRect(0, 0, width(), pref.Graphs.fontSizeTitle), Qt::AlignCenter, title);
        marginTop += pref.Graphs.fontSizeTitle;
    }

    // show names of active traces and marker data (if enabled)
    bool hasMarkerData = false;
    auto marginMarkerData = pref.Graphs.fontSizeMarkerData * 12.5;
    auto traceNameTop = marginTop + 5;
    marginTop += pref.Graphs.fontSizeTraceNames + 8;
    int x = 1; // xcoordinate for the next trace name
    int y = marginTop; // ycoordinate for the next marker data
    auto labelMarginRight = 4;
    auto borderRadius = 5;
    for(auto t : traces) {
        if(!t.second || !t.first->isVisible()) {
            continue;
        }

        // Trace name
        auto textArea = QRect(x, traceNameTop, width() - x, pref.Graphs.fontSizeTraceNames + 8);
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

    if(dropPending) {
        p.setOpacity(dropOpacity);
        p.setBrush(dropBackgroundColor);
        p.setPen(dropForegroundColor);

        auto dropRect = getDropRect();

        p.fillRect(0, 0, dropRect.left(), h-1, p.brush());
        p.fillRect(dropRect.left(), 0, dropRect.width()-1, dropRect.top(), p.brush());
        p.fillRect(dropRect.left(), dropRect.bottom(), dropRect.width()-1, h-1, p.brush());
        p.fillRect(dropRect.right(), 0, w-1, h-1, p.brush());

        p.setOpacity(1.0);
        p.drawLine(QPoint(0, 0), dropRect.topLeft());
        p.drawLine(QPoint(0, h-1), dropRect.bottomLeft());
        p.drawLine(QPoint(w-1, 0), dropRect.topRight());
        p.drawLine(QPoint(w-1, h-1), dropRect.bottomRight());
        p.drawLine(QPoint(0, 0), QPoint(0, h-1));
        p.drawLine(QPoint(0, h-1), QPoint(w-1, h-1));
        p.drawLine(QPoint(w-1, h-1), QPoint(w-1, 0));
        p.drawLine(QPoint(w-1, 0), QPoint(0, 0));
        p.drawLine(dropRect.topLeft(), dropRect.topRight());
        p.drawLine(dropRect.topRight(), dropRect.bottomRight());
        p.drawLine(dropRect.bottomRight(), dropRect.bottomLeft());
        p.drawLine(dropRect.bottomLeft(), dropRect.topLeft());

        auto font = p.font();
        font.setPixelSize(20);
        p.setFont(font);
        p.setPen(dropSection == DropSection::Above ? dropHighlightColor : dropForegroundColor);
        p.drawText(QRect(0, 0, w, dropRect.top()), Qt::AlignCenter, "Insert above");
        p.setPen(dropSection == DropSection::Below ? dropHighlightColor : dropForegroundColor);
        p.drawText(QRect(0, dropRect.bottom(), w, dropRect.top()), Qt::AlignCenter, "Insert below");
        p.setPen(dropSection == DropSection::ToTheLeft ? dropHighlightColor : dropForegroundColor);
        p.drawText(QRect(0, 0, dropRect.left(), h), Qt::AlignCenter, "Insert to\nthe left");
        p.setPen(dropSection == DropSection::ToTheRight ? dropHighlightColor : dropForegroundColor);
        p.drawText(QRect(dropRect.right(), 0, dropRect.left(), h), Qt::AlignCenter, "Insert to\nthe right");
    }

    replotTimer.start(MaxUpdateInterval);
}

void TracePlot::finishContextMenu()
{
    contextmenu->addSeparator();
    auto setTitle = new QAction("Set Title", contextmenu);
    contextmenu->addAction(setTitle);
    connect(setTitle, &QAction::triggered, [=](){
        title = QInputDialog::getText(contextmenu, "Set new graph title", "Enter new title:", QLineEdit::Normal, title);
    });
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

    auto removeTile = new QAction("Remove Tile", contextmenu);
    contextmenu->addAction(removeTile);
    connect(removeTile, &QAction::triggered, [=]() {
        markedForDeletion = true;
        // 'this' object will be deleted when returning function but the following lambda
        // might be executed after that and we still need to know which tile to close.
        // Capture parentTile explicitly by value
        QTimer::singleShot(0, [p=this->parentTile](){
            p->closeTile();
        });
    });

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
        triggerReplot();
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
        triggerReplot();
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
    QPoint position(event->position().x(), event->position().y());
    if(pref.Graphs.enablePanAndZoom && positionWithinGraphArea(position)) {
        bool horizontally = !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
        bool vertically = !(QApplication::keyboardModifiers() & Qt::ControlModifier);
        double factor = pow(pref.Graphs.zoomFactor, (double) event->angleDelta().y() / 120.0);
        zoom(position, factor, horizontally, vertically);
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
        for(Marker* m : markers) {
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
                    if(closestMarker->getType() == Marker::Type::Flatness) {
                        closestMarker = m;
                    }
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
//        if(dropSupported(trace)) {
            event->acceptProposedAction();
            dropPending = true;
            dropTrace = trace;
//        }
    }
    triggerReplot();
}

void TracePlot::dragMoveEvent(QDragMoveEvent *event)
{
    if(!dropPending) {
        return;
    }
    auto dropRect = getDropRect();
    auto pos = event->position().toPoint() - QPoint(marginLeft, marginTop);
    if(dropRect.contains(pos)) {
        dropSection = DropSection::OnPlot;
    } else {
        // transform to relative coordinates from 0 to 1
        auto x = (double) pos.x() / (width() - marginLeft - marginRight);
        auto y = (double) pos.y() / (height() - marginTop - marginBottom);
        if(y < 0.5) {
            if(x < y) {
                dropSection = DropSection::ToTheLeft;
            } else if(x > (1.0 - y)) {
                dropSection = DropSection::ToTheRight;
            } else {
                dropSection = DropSection::Above;
            }
        } else {
            if(x < (1.0 - y)) {
                dropSection = DropSection::ToTheLeft;
            } else if(x > y) {
                dropSection = DropSection::ToTheRight;
            } else {
                dropSection = DropSection::Below;
            }
        }
    }
    dropPosition = pos;
    replot();
}

void TracePlot::dropEvent(QDropEvent *event)
{
    if(dropTrace) {
        if(dropSection == DropSection::OnPlot) {
            traceDropped(dropTrace, event->position().toPoint() -  - QPoint(marginLeft, marginTop));
        } else {
            TileWidget *newTile = nullptr;
            // parentTile will be modified by the split, save here
            TileWidget *oldParent = parentTile;
            switch(dropSection) {
            case DropSection::Above:
                parentTile->splitVertically(true);
                newTile = oldParent->Child1();
                break;
            case DropSection::Below:
                parentTile->splitVertically(false);
                newTile = oldParent->Child2();
                break;
            case DropSection::ToTheLeft:
                parentTile->splitHorizontally(true);
                newTile = oldParent->Child1();
                break;
            case DropSection::ToTheRight:
                parentTile->splitHorizontally(false);
                newTile = oldParent->Child2();
                break;
            case DropSection::OnPlot:
                // already handled above
                break;
            }
            TracePlot *graph = createDefaultPlotForTrace(model, dropTrace);
            if(!graph->configureForTrace(dropTrace)) {
                // can't be used for the configuration the trace is in, fall back to XY-Plot
                delete graph;
                graph = new TraceXYPlot(model);
                graph->configureForTrace(dropTrace);
            }
            newTile->setPlot(graph);
            graph->enableTrace(dropTrace, true);
        }
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

QRect TracePlot::getDropRect()
{
    constexpr double dropBorders = 0.2;
    auto w = width() - marginLeft - marginRight;
    auto h = height() - marginTop - marginBottom;
    return QRect(QPoint(w*dropBorders, h*dropBorders), QSize(w*(1.0-2*dropBorders), h*(1.0-2*dropBorders)));
}

nlohmann::json TracePlot::getBaseJSON()
{
    nlohmann::json j;
    j["title"] = title.toStdString();
    return j;
}

void TracePlot::parseBaseJSON(nlohmann::json j)
{
    title = QString::fromStdString(j.value("title", ""));
}

std::set<TracePlot *> TracePlot::getPlots()
{
    return plots;
}

TracePlot *TracePlot::createDefaultPlotForTrace(TraceModel &model, Trace *t)
{
    auto &p = Preferences::getInstance();

    TracePlot *ret = nullptr;
    if(t->isReflection()) {
        ret = createFromType(model, TypeFromString(p.Graphs.defaultGraphs.reflection));
    } else {
        ret = createFromType(model, TypeFromString(p.Graphs.defaultGraphs.transmission));
    }
    return ret;
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
    if (lastUpdate.msecsTo(now) >= MinUpdateInterval // last update was a sufficiently long time ago
            || lastUpdate.msecsTo(now) < 0) { // or the time rolled over at midnight
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
            } else {
                // other trace may need to be removed because the graph no longer supports them. However, they (the traces)
                // may change soon as well (e.g. because multiple traces changed at the same time and we are only handling
                // the first changed trace right now). Postpone the deletion of other traces until all currently executing
                // slots have completed
                QTimer::singleShot(0, [this]{
                    for(auto t : activeTraces()) {
                        if(!supported(t)) {
                            enableTrace(t, false);
                        }
                    }
                });
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
