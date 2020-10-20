#include "tracebodeplot.h"
#include <QGridLayout>
#include <qwt_plot_grid.h>
#include "qwtplotpiecewisecurve.h"
#include "qwt_series_data.h"
#include "trace.h"
#include <cmath>
#include <QFrame>
#include <qwt_plot_canvas.h>
#include <qwt_scale_div.h>
#include <qwt_plot_layout.h>
#include "tracemarker.h"
#include <qwt_symbol.h>
#include <qwt_picker_machine.h>
#include "bodeplotaxisdialog.h"

using namespace std;

static double AxisTransformation(TraceBodePlot::YAxisType type, complex<double> data) {
    switch(type) {
    case TraceBodePlot::YAxisType::Magnitude: return 20*log10(abs(data)); break;
    case TraceBodePlot::YAxisType::Phase: return arg(data) * 180.0 / M_PI; break;
    case TraceBodePlot::YAxisType::VSWR:
        if(abs(data) < 1.0) {
            return (1+abs(data)) / (1-abs(data));
        }
        break;
    default: break;
    }
    return numeric_limits<double>::quiet_NaN();
}

template<TraceBodePlot::YAxisType E> class QwtTraceSeries : public QwtSeriesData<QPointF> {
public:
    QwtTraceSeries(Trace &t)
        : QwtSeriesData<QPointF>(),
          t(t){};
    size_t size() const override {
        return t.size();
    }
    QPointF sample(size_t i) const override {
        Trace::Data d = t.sample(i);
        QPointF p;
        p.setX(d.frequency);
        p.setY(AxisTransformation(E, d.S));
        return p;
    }
    QRectF boundingRect() const override {
        return qwtBoundingRect(*this);
    }

private:
    Trace &t;
};

TraceBodePlot::TraceBodePlot(TraceModel &model, QWidget *parent)
    : TracePlot(parent),
      selectedMarker(nullptr)
{
    plot = new QwtPlot(this);
    plot->setCanvasBackground(Background);
    auto pal = plot->palette();
    pal.setColor(QPalette::Window, Background);
    pal.setColor(QPalette::WindowText, Border);
    pal.setColor(QPalette::Text, Border);
    auto canvas = new QwtPlotCanvas(plot);
    canvas->setFrameStyle(QFrame::Plain);
    plot->setCanvas(canvas);
    plot->setPalette(pal);
    plot->setAutoFillBackground(true);

    auto selectPicker = new BodeplotPicker(plot->xBottom, plot->yLeft, QwtPicker::NoRubberBand, QwtPicker::ActiveOnly, plot->canvas());
    selectPicker->setStateMachine(new QwtPickerClickPointMachine);

    drawPicker = new BodeplotPicker(plot->xBottom, plot->yLeft, QwtPicker::NoRubberBand, QwtPicker::ActiveOnly, plot->canvas());
    drawPicker->setStateMachine(new QwtPickerDragPointMachine);
    drawPicker->setTrackerPen(QPen(Qt::white));

    // Marker selection
    connect(selectPicker, SIGNAL(selected(QPointF)), this, SLOT(clicked(QPointF)));;
    // Marker movement
    connect(drawPicker, SIGNAL(moved(QPointF)), this, SLOT(moved(QPointF)));

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Divisions, 1.0, Qt::DotLine));
    grid->attach(plot);
    auto layout = new QGridLayout;
    layout->addWidget(plot);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    plot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    plot->plotLayout()->setAlignCanvasToScales(true);
    initializeTraceInfo(model);
    setAutoFillBackground(true);

    // Setup default axis
    setYAxis(0, YAxisType::Magnitude, false, false, -120, 20, 10);
    setYAxis(1, YAxisType::Phase, false, false, -180, 180, 30);
    // enable autoscaling and set for full span (no information about actual span available yet)
    setXAxis(0, 6000000000);
    setXAxis(true, 0, 6000000000, 600000000);
    // get notified when the span changes
    connect(&model, &TraceModel::SpanChanged, this, qOverload<double, double>(&TraceBodePlot::setXAxis));
}

TraceBodePlot::~TraceBodePlot()
{
    for(int axis = 0;axis < 2;axis++) {
        for(auto pd : curves[axis]) {
            delete pd.second.curve;
        }
    }
    delete drawPicker;
}

void TraceBodePlot::setXAxis(double min, double max)
{
    sweep_fmin = min;
    sweep_fmax = max;
    updateXAxis();
}

void TraceBodePlot::setYAxis(int axis, TraceBodePlot::YAxisType type, bool log, bool autorange, double min, double max, double div)
{
    if(YAxis[axis].type != type) {
        YAxis[axis].type = type;
        // remove traces that are active but not supported with the new axis type
        bool erased = false;
        do {
            erased = false;
            for(auto t : tracesAxis[axis]) {
                if(!supported(t, type)) {
                    enableTraceAxis(t, axis, false);
                    erased = true;
                    break;
                }
            }
        } while(erased);

        for(auto t : tracesAxis[axis]) {
            // supported but needs an adjusted QwtSeriesData
            auto td = curves[axis][t];
            td.data = createQwtSeriesData(*t, axis);
            // call to setSamples deletes old QwtSeriesData
            td.curve->setSamples(td.data);
            if(axis == 0) {
                // update marker data
                auto marker = t->getMarkers();
                for(auto m : marker) {
                    markerDataChanged(m);
                }
            }
        }
    }
    YAxis[axis].log = log;
    YAxis[axis].autorange = autorange;
    YAxis[axis].rangeMin = min;
    YAxis[axis].rangeMax = max;
    YAxis[axis].rangeDiv = div;
    // enable/disable y axis
    auto qwtaxis = axis == 0 ? QwtPlot::yLeft : QwtPlot::yRight;
    plot->enableAxis(qwtaxis, type != YAxisType::Disabled);
    if(autorange) {
        plot->setAxisAutoScale(qwtaxis, true);
    } else {
        plot->setAxisScale(qwtaxis, min, max, div);
    }
    updateContextMenu();
    replot();
}

void TraceBodePlot::setXAxis(bool autorange, double min, double max, double div)
{
    XAxis.autorange = autorange;
    XAxis.rangeMin = min;
    XAxis.rangeMax = max;
    XAxis.rangeDiv = div;
    updateXAxis();
}

void TraceBodePlot::enableTrace(Trace *t, bool enabled)
{
    for(int axis = 0;axis < 2;axis++) {
        if(supported(t, YAxis[axis].type)) {
            enableTraceAxis(t, axis, enabled);
        }
    }
}

void TraceBodePlot::updateContextMenu()
{
    contextmenu->clear();
//    for(int axis = 0;axis < 2;axis++) {
//        QMenu *axisMenu;
//        if(axis == 0) {
//            axisMenu = contextmenu->addMenu("Primary Axis");
//        } else {
//            axisMenu = contextmenu->addMenu("Secondary Axis");
//        }
//        auto group = new QActionGroup(this);
//        for(int i=0;i<(int) YAxisType::Last;i++) {
//            auto action = new QAction(AxisTypeToName((YAxisType) i));
//            action->setCheckable(true);
//            group->addAction(action);
//            if(YAxis[axis].type == (YAxisType) i) {
//                action->setChecked(true);
//            }
//            connect(action, &QAction::triggered, [=](bool active) {
//                if(active) {
//                    setYAxisType(axis, (YAxisType) i);
//                }
//            });
//        }
//        axisMenu->addActions(group->actions());
//    }
    auto setup = new QAction("Axis setup...");
    connect(setup, &QAction::triggered, [this]() {
        auto setup = new BodeplotAxisDialog(this);
        setup->show();
    });
    contextmenu->addAction(setup);
    for(int axis = 0;axis < 2;axis++) {
        if(YAxis[axis].type == YAxisType::Disabled) {
            continue;
        }
        if(axis == 0) {
            contextmenu->addSection("Primary Traces");
        } else {
            contextmenu->addSection("Secondary Traces");
        }
        for(auto t : traces) {
            // Skip traces that are not applicable for the selected axis type
            if(!supported(t.first, YAxis[axis].type)) {
                continue;
            }

            auto action = new QAction(t.first->name());
            action->setCheckable(true);
            if(tracesAxis[axis].find(t.first) != tracesAxis[axis].end()) {
                action->setChecked(true);
            }
            connect(action, &QAction::toggled, [=](bool active) {
                enableTraceAxis(t.first, axis, active);
            });
            contextmenu->addAction(action);
        }
    }
    contextmenu->addSeparator();
    auto close = new QAction("Close");
    contextmenu->addAction(close);
    connect(close, &QAction::triggered, [=]() {
        markedForDeletion = true;
    });
}

bool TraceBodePlot::supported(Trace *)
{
    // potentially possible to add every kind of trace (depends on axis)
    return true;
}

void TraceBodePlot::replot()
{
    plot->replot();
}

QString TraceBodePlot::AxisTypeToName(TraceBodePlot::YAxisType type)
{
    switch(type) {
    case YAxisType::Disabled: return "Disabled"; break;
    case YAxisType::Magnitude: return "Magnitude"; break;
    case YAxisType::Phase: return "Phase"; break;
    case YAxisType::VSWR: return "VSWR"; break;
    default: return "Unknown"; break;
    }
}

void TraceBodePlot::enableTraceAxis(Trace *t, int axis, bool enabled)
{
    bool alreadyEnabled = tracesAxis[axis].find(t) != tracesAxis[axis].end();
    if(alreadyEnabled != enabled) {
        if(enabled) {
            tracesAxis[axis].insert(t);
            CurveData cd;
            cd.data = createQwtSeriesData(*t, axis);
            cd.curve = new QwtPlotPiecewiseCurve();
            cd.curve->attach(plot);
            cd.curve->setYAxis(axis == 0 ? QwtPlot::yLeft : QwtPlot::yRight);
            cd.curve->setSamples(cd.data);
            curves[axis][t] = cd;
            // connect signals
            connect(t, &Trace::dataChanged, this, &TraceBodePlot::triggerReplot);
            connect(t, &Trace::colorChanged, this, &TraceBodePlot::traceColorChanged);
            connect(t, &Trace::visibilityChanged, this, &TraceBodePlot::traceColorChanged);
            connect(t, &Trace::visibilityChanged, this, &TraceBodePlot::triggerReplot);
            if(axis == 0) {
                connect(t, &Trace::markerAdded, this, &TraceBodePlot::markerAdded);
                connect(t, &Trace::markerRemoved, this, &TraceBodePlot::markerRemoved);
                auto tracemarkers = t->getMarkers();
                for(auto m : tracemarkers) {
                    markerAdded(m);
                }
            }
            traceColorChanged(t);
        } else {
            tracesAxis[axis].erase(t);
            // clean up and delete
            if(curves[axis].find(t) != curves[axis].end()) {
                delete curves[axis][t].curve;
                curves[axis].erase(t);
            }
            int otherAxis = axis == 0 ? 1 : 0;
            if(curves[otherAxis].find(t) == curves[otherAxis].end()) {
                // this trace is not used anymore, disconnect from notifications
                disconnect(t, &Trace::dataChanged, this, &TraceBodePlot::triggerReplot);
                disconnect(t, &Trace::colorChanged, this, &TraceBodePlot::traceColorChanged);
                disconnect(t, &Trace::visibilityChanged, this, &TraceBodePlot::traceColorChanged);
                disconnect(t, &Trace::visibilityChanged, this, &TraceBodePlot::triggerReplot);
            }
            if(axis == 0) {
                disconnect(t, &Trace::markerAdded, this, &TraceBodePlot::markerAdded);
                disconnect(t, &Trace::markerRemoved, this, &TraceBodePlot::markerRemoved);
                auto tracemarkers = t->getMarkers();
                for(auto m : tracemarkers) {
                    markerRemoved(m);
                }
            }
        }

        updateContextMenu();
        replot();
    }
}

bool TraceBodePlot::supported(Trace *t, TraceBodePlot::YAxisType type)
{
    switch(type) {
    case YAxisType::Disabled:
        return false;
    case YAxisType::VSWR:
        if(!t->isReflection()) {
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}

void TraceBodePlot::updateXAxis()
{
    if(XAxis.autorange && sweep_fmax-sweep_fmin > 0) {
        QList<double> tickList;
        for(double tick = sweep_fmin;tick <= sweep_fmax;tick+= (sweep_fmax-sweep_fmin)/10) {
            tickList.append(tick);
        }
        QwtScaleDiv scalediv(sweep_fmin, sweep_fmax, QList<double>(), QList<double>(), tickList);
        plot->setAxisScaleDiv(QwtPlot::xBottom, scalediv);
    } else {
        plot->setAxisScale(QwtPlot::xBottom, XAxis.rangeMin, XAxis.rangeMax, XAxis.rangeDiv);
    }
    triggerReplot();
}

QwtSeriesData<QPointF> *TraceBodePlot::createQwtSeriesData(Trace &t, int axis)
{
    switch(YAxis[axis].type) {
    case YAxisType::Magnitude:
        return new QwtTraceSeries<YAxisType::Magnitude>(t);
    case YAxisType::Phase:
        return new QwtTraceSeries<YAxisType::Phase>(t);
    case YAxisType::VSWR:
        return new QwtTraceSeries<YAxisType::VSWR>(t);
    default:
        return nullptr;
    }
}

void TraceBodePlot::traceColorChanged(Trace *t)
{
    for(int axis = 0;axis < 2;axis++) {
        if(curves[axis].find(t) != curves[axis].end()) {
            // trace active, change the pen color
            if(t->isVisible()) {
                if(axis == 0) {
                    curves[axis][t].curve->setPen(t->color());
                } else {
                    curves[axis][t].curve->setPen(t->color(), 1.0, Qt::DashLine);
                }
                for(auto m : t->getMarkers()) {
                    if(markers.count(m)) {
                        markers[m]->attach(plot);
                    }
                }
            } else {
                curves[axis][t].curve->setPen(t->color(), 0.0, Qt::NoPen);
                for(auto m : t->getMarkers()) {
                    if(markers.count(m)) {
                        markers[m]->detach();
                    }
                }
            }
        }
    }
}

void TraceBodePlot::markerAdded(TraceMarker *m)
{
    if(markers.count(m)) {
        return;
    }
    auto qwtMarker = new QwtPlotMarker;
    markers[m] = qwtMarker;
    markerSymbolChanged(m);
    connect(m, &TraceMarker::symbolChanged, this, &TraceBodePlot::markerSymbolChanged);
    connect(m, &TraceMarker::dataChanged, this, &TraceBodePlot::markerDataChanged);
    markerDataChanged(m);
    qwtMarker->attach(plot);
    triggerReplot();
}

void TraceBodePlot::markerRemoved(TraceMarker *m)
{
    disconnect(m, &TraceMarker::symbolChanged, this, &TraceBodePlot::markerSymbolChanged);
    disconnect(m, &TraceMarker::dataChanged, this, &TraceBodePlot::markerDataChanged);
    if(markers.count(m)) {
        markers[m]->detach();
        delete markers[m];
        markers.erase(m);
    }
    triggerReplot();
}

void TraceBodePlot::markerDataChanged(TraceMarker *m)
{
    auto qwtMarker = markers[m];
    qwtMarker->setXValue(m->getFrequency());
    qwtMarker->setYValue(AxisTransformation(YAxis[0].type, m->getData()));
    triggerReplot();
}

void TraceBodePlot::markerSymbolChanged(TraceMarker *m)
{
    auto qwtMarker = markers[m];
    auto old_sym = qwtMarker->symbol();
    qwtMarker->setSymbol(nullptr);
    delete old_sym;

    QwtSymbol *sym=new QwtSymbol;
    sym->setPixmap(m->getSymbol());
    sym->setPinPoint(QPointF(m->getSymbol().width()/2, m->getSymbol().height()));
    qwtMarker->setSymbol(sym);
    triggerReplot();
}

void TraceBodePlot::clicked(const QPointF pos)
{
    auto clickPoint = drawPicker->plotToPixel(pos);
    unsigned int closestDistance = numeric_limits<unsigned int>::max();
    TraceMarker *closestMarker = nullptr;
    for(auto m : markers) {
        auto markerPoint = drawPicker->plotToPixel(m.second->value());
        auto yDiff = abs(markerPoint.y() - clickPoint.y());
        auto xDiff = abs(markerPoint.x() - clickPoint.x());
        unsigned int distance = xDiff * xDiff + yDiff * yDiff;
        if(distance < closestDistance) {
            closestDistance = distance;
            closestMarker = m.first;
        }
    }
    if(closestDistance <= 400) {
        selectedMarker = closestMarker;
        selectedCurve = curves[0][selectedMarker->trace()].curve;
    } else {
        selectedMarker = nullptr;
        selectedCurve = nullptr;
    }
}

void TraceBodePlot::moved(const QPointF pos)
{
    if(!selectedMarker || !selectedCurve) {
        return;
    }
//        int index = selectedCurve->closestPoint(pos.toPoint());
//        qDebug() << index;
//        if(index < 0) {
//            // unable to find closest point
//            return;
//        }
//        selectedMarker->setFrequency(selectedCurve->sample(index).x());
    selectedMarker->setFrequency(pos.x());
}

