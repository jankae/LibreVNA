#include "tracexyplot.h"
#include <QGridLayout>
//#include "qwtplotpiecewisecurve.h"
//#include "qwt_series_data.h"
#include "trace.h"
#include <cmath>
#include <QFrame>
//#include <qwt_plot_canvas.h>
//#include <qwt_scale_div.h>
//#include <qwt_plot_layout.h>
#include "tracemarker.h"
//#include <qwt_symbol.h>
//#include <qwt_picker_machine.h>
#include "xyplotaxisdialog.h"
#include <preferences.h>
#include <QPainter>
#include "Util/util.h"
#include "unit.h"

using namespace std;

//set<TraceXYPlot*> TraceXYPlot::allPlots;

const set<TraceXYPlot::YAxisType> TraceXYPlot::YAxisTypes = {TraceXYPlot::YAxisType::Disabled,
                                           TraceXYPlot::YAxisType::Magnitude,
                                           TraceXYPlot::YAxisType::Phase,
                                           TraceXYPlot::YAxisType::VSWR,
                                           TraceXYPlot::YAxisType::Impulse,
                                           TraceXYPlot::YAxisType::Step,
                                           TraceXYPlot::YAxisType::Impedance};

static double FrequencyAxisTransformation(TraceXYPlot::YAxisType type, complex<double> data) {
    switch(type) {
    case TraceXYPlot::YAxisType::Magnitude: return 20*log10(abs(data)); break;
    case TraceXYPlot::YAxisType::Phase: return arg(data) * 180.0 / M_PI; break;
    case TraceXYPlot::YAxisType::VSWR:
        if(abs(data) < 1.0) {
            return (1+abs(data)) / (1-abs(data));
        }
        break;
    default: break;
    }
    return numeric_limits<double>::quiet_NaN();
}
static double TimeAxisTransformation(TraceXYPlot::YAxisType type, Trace *t, int index) {
    auto timeData = t->getTDR()[index];
    switch(type) {
    case TraceXYPlot::YAxisType::Impulse: return timeData.impulseResponse; break;
    case TraceXYPlot::YAxisType::Step: return timeData.stepResponse; break;
    case TraceXYPlot::YAxisType::Impedance:
        if(abs(timeData.stepResponse) < 1.0) {
            return 50 * (1+timeData.stepResponse) / (1-timeData.stepResponse);
        }
        break;
    default: break;
    }
    return numeric_limits<double>::quiet_NaN();
}

//class QwtTraceSeries : public QwtSeriesData<QPointF> {
//public:
//    QwtTraceSeries(Trace &t, TraceXYPlot::YAxisType Ytype, const TraceXYPlot *plot)
//        : QwtSeriesData<QPointF>(),
//          Ytype(Ytype),
//          plot(plot),
//          t(t){}
//    size_t size() const override {
//        switch(Ytype) {
//        case TraceXYPlot::YAxisType::Magnitude:
//        case TraceXYPlot::YAxisType::Phase:
//        case TraceXYPlot::YAxisType::VSWR:
//            return t.size();
//        case TraceXYPlot::YAxisType::Impulse:
//        case TraceXYPlot::YAxisType::Step:
//        case TraceXYPlot::YAxisType::Impedance:
//            return t.getTDR().size();
//        default:
//            return 0;
//        }
//    }
//    QPointF sample(size_t i) const override {
//        switch(Ytype) {
//        case TraceXYPlot::YAxisType::Magnitude:
//        case TraceXYPlot::YAxisType::Phase:
//        case TraceXYPlot::YAxisType::VSWR: {
//            Trace::Data d = t.sample(i);
//            QPointF p;
//            p.setX(d.frequency);
//            p.setY(FrequencyAxisTransformation(Ytype, d.S));
//            return p;
//        }
//        case TraceXYPlot::YAxisType::Impulse:
//        case TraceXYPlot::YAxisType::Step:
//        case TraceXYPlot::YAxisType::Impedance: {
//            auto sample = t.getTDR()[i];
//            QPointF p;
//            if(plot->XAxis.type == TraceXYPlot::XAxisType::Time) {
//                p.setX(sample.time);
//            } else {
//                p.setX(sample.distance);
//            }
//            p.setY(TimeAxisTransformation(Ytype, &t, i));
//            return p;
//        }
//        default:
//            return QPointF();
//        }

//    }
//    QRectF boundingRect() const override {
//        return qwtBoundingRect(*this);
//    }

//private:
//    TraceXYPlot::YAxisType Ytype;
//    const TraceXYPlot *plot;
//    Trace &t;
//};

TraceXYPlot::TraceXYPlot(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent),
      selectedMarker(nullptr)
{
    YAxis[0].log = false;
    YAxis[0].type = YAxisType::Disabled;
    YAxis[0].rangeDiv = 1;
    YAxis[0].rangeMax = 10;
    YAxis[0].rangeMin = 0;
    YAxis[0].autorange = false;
    YAxis[1].log = false;
    YAxis[1].type = YAxisType::Disabled;
    YAxis[1].rangeDiv = 1;
    YAxis[1].rangeMax = 10;
    YAxis[1].rangeMin = 0;
    YAxis[1].autorange = false;
    XAxis.type = XAxisType::Frequency;
    XAxis.log = false;
    XAxis.rangeDiv = 1;
    XAxis.rangeMax = 10;
    XAxis.rangeMin = 0;
    XAxis.mode = XAxisMode::UseSpan;



//    plot = new QwtPlot(this);

//    auto canvas = new QwtPlotCanvas(plot);
//    canvas->setFrameStyle(QFrame::Plain);
//    plot->setCanvas(canvas);
//    plot->setAutoFillBackground(true);
//    grid = new QwtPlotGrid();
//    grid->attach(plot);
//    setColorFromPreferences();

//    auto selectPicker = new XYplotPicker(plot->xBottom, plot->yLeft, QwtPicker::NoRubberBand, QwtPicker::ActiveOnly, plot->canvas());
//    selectPicker->setStateMachine(new QwtPickerClickPointMachine);

//    drawPicker = new XYplotPicker(plot->xBottom, plot->yLeft, QwtPicker::NoRubberBand, QwtPicker::ActiveOnly, plot->canvas());
//    drawPicker->setStateMachine(new QwtPickerDragPointMachine);
//    drawPicker->setTrackerPen(QPen(Qt::white));

//    // Marker selection
//    connect(selectPicker, SIGNAL(selected(QPointF)), this, SLOT(clicked(QPointF)));;
//    // Marker movement
//    connect(drawPicker, SIGNAL(moved(QPointF)), this, SLOT(moved(QPointF)));

//    auto layout = new QGridLayout;
//    layout->addWidget(plot);
//    layout->setContentsMargins(0, 0, 0, 0);
//    setLayout(layout);
//    plot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    initializeTraceInfo();
//    setAutoFillBackground(true);

    // Setup default axis
    setYAxis(0, YAxisType::Magnitude, false, false, -120, 20, 10);
    setYAxis(1, YAxisType::Phase, false, false, -180, 180, 30);
    // enable autoscaling and set for full span (no information about actual span available yet)
    updateSpan(0, 6000000000);
    setXAxis(XAxisType::Frequency, XAxisMode::UseSpan, 0, 6000000000, 600000000);

//    allPlots.insert(this);
}

TraceXYPlot::~TraceXYPlot()
{
//    for(int axis = 0;axis < 2;axis++) {
//        for(auto pd : curves[axis]) {
//            delete pd.second.curve;
//        }
//    }
//    delete drawPicker;
//    allPlots.erase(this);
}

void TraceXYPlot::setYAxis(int axis, TraceXYPlot::YAxisType type, bool log, bool autorange, double min, double max, double div)
{
    if(YAxis[axis].type != type) {
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

        if(isTDRtype(YAxis[axis].type)) {
            for(auto t : tracesAxis[axis]) {
                t->removeTDRinterest();
            }
        }
        YAxis[axis].type = type;

        for(auto t : tracesAxis[axis]) {
            // supported but needs an adjusted QwtSeriesData
//            auto td = curves[axis][t];
//            td.data = createQwtSeriesData(*t, axis);
            // call to setSamples deletes old QwtSeriesData
//            td.curve->setSamples(td.data);
            if(axis == 0) {
                // update marker data
//                auto marker = t->getMarkers();
//                for(auto m : marker) {
//                    markerDataChanged(m);
//                }
            }
            if(isTDRtype(type)) {
                t->addTDRinterest();
            }
        }
    }
    YAxis[axis].log = log;
    YAxis[axis].autorange = autorange;
    YAxis[axis].rangeMin = min;
    YAxis[axis].rangeMax = max;
    YAxis[axis].rangeDiv = div;
    updateAxisTicks();
    // enable/disable y axis
//    auto qwtaxis = axis == 0 ? QwtPlot::yLeft : QwtPlot::yRight;
//    plot->enableAxis(qwtaxis, type != YAxisType::Disabled);
//    if(autorange) {
//        plot->setAxisAutoScale(qwtaxis, true);
//    } else {
//        plot->setAxisScale(qwtaxis, min, max, div);
//    }
    updateContextMenu();
    replot();
}

void TraceXYPlot::setXAxis(XAxisType type, XAxisMode mode, double min, double max, double div)
{
    XAxis.type = type;
    XAxis.mode = mode;
    XAxis.rangeMin = min;
    XAxis.rangeMax = max;
    XAxis.rangeDiv = div;
    updateAxisTicks();
}

void TraceXYPlot::enableTrace(Trace *t, bool enabled)
{
    for(int axis = 0;axis < 2;axis++) {
        if(supported(t, YAxis[axis].type)) {
            enableTraceAxis(t, axis, enabled);
        }
    }
}

void TraceXYPlot::updateSpan(double min, double max)
{
    TracePlot::updateSpan(min, max);
    updateAxisTicks();
}

//void TraceXYPlot::updateGraphColors()
//{
//    for(auto p : allPlots) {
//        p->setColorFromPreferences();
//    }
//}

bool TraceXYPlot::isTDRtype(TraceXYPlot::YAxisType type)
{
    switch(type) {
    case YAxisType::Impulse:
    case YAxisType::Step:
    case YAxisType::Impedance:
        return true;
    default:
        return false;
    }
}

void TraceXYPlot::axisSetupDialog()
{
    auto setup = new XYplotAxisDialog(this);
    setup->show();
}

void TraceXYPlot::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Axis setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TraceXYPlot::axisSetupDialog);
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

            auto action = new QAction(t.first->name(), contextmenu);
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
    auto close = new QAction("Close", contextmenu);
    contextmenu->addAction(close);
    connect(close, &QAction::triggered, [=]() {
        markedForDeletion = true;
    });
}

bool TraceXYPlot::supported(Trace *)
{
    // potentially possible to add every kind of trace (depends on axis)
    return true;
}

void TraceXYPlot::draw(QPainter &p)
{
    p.setBrush(palette().windowText());
    auto pref = Preferences::getInstance();

    constexpr int yAxisSpace = 50;
    constexpr int yAxisDisabledSpace = 10;
    constexpr int xAxisSpace = 30;
    auto w = p.window();
    auto pen = QPen(pref.General.graphColors.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    int plotAreaLeft = YAxis[0].type == YAxisType::Disabled ? yAxisDisabledSpace : yAxisSpace;
    int plotAreaWidth = w.width();
    int plotAreaBottom = w.height() - xAxisSpace;
    if(YAxis[0].type != YAxisType::Disabled) {
        plotAreaWidth -= yAxisSpace;
    } else {
        plotAreaWidth -= yAxisDisabledSpace;
    }
    if(YAxis[1].type != YAxisType::Disabled) {
        plotAreaWidth -= yAxisSpace;
    } else {
        plotAreaWidth -= yAxisDisabledSpace;
    }
    p.drawRect(plotAreaLeft, 0, plotAreaWidth, w.height()-xAxisSpace);

    // draw axis types
    QString labelX;
    switch(XAxis.type) {
    case XAxisType::Frequency: labelX = "Frequency"; break;
    case XAxisType::Time: labelX = "Time"; break;
    case XAxisType::Distance: labelX = "Distance"; break;
    }
    auto font = p.font();
    font.setPixelSize(AxisLabelSize);
    p.setFont(font);
    p.drawText(QRect(0, w.height()-AxisLabelSize*1.5, w.width(), AxisLabelSize*1.5), Qt::AlignHCenter, labelX);
    // draw X ticks
    // this only works for evenly distributed ticks:
    auto max = qMax(abs(XAxis.ticks.front()), abs(XAxis.ticks.back()));
    auto step = abs(XAxis.ticks[0] - XAxis.ticks[1]);
    int significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
    bool displayFullFreq = significantDigits <= 5;
    constexpr int displayLastDigits = 4;
    if(!displayFullFreq) {
        auto fullFreq = Unit::ToString(XAxis.ticks.front(), "", " kMG", significantDigits);

        auto front = fullFreq;
        front.truncate(fullFreq.size() - displayLastDigits);
        auto back = fullFreq;
        back.remove(0, front.size());
        back.append("..");
        p.setPen(QPen(QColor("orange")));
        QRect bounding;
        p.drawText(QRect(2, plotAreaBottom + AxisLabelSize + 5, w.width(), AxisLabelSize), 0, front, &bounding);
        p.setPen(pref.General.graphColors.axis);
        p.drawText(QRect(bounding.x() + bounding.width(), plotAreaBottom + AxisLabelSize + 5, w.width(), AxisLabelSize), 0, back);
    }

    for(auto t : XAxis.ticks) {
        auto xCoord = Util::Scale<double>(t, XAxis.ticks.front(), XAxis.ticks.back(), plotAreaLeft, plotAreaLeft + plotAreaWidth);
        auto tickValue = Unit::ToString(t, "", "fpnum kMG", significantDigits);
        p.setPen(QPen(pref.General.graphColors.axis, 1));
        if(displayFullFreq) {
            p.drawText(QRect(xCoord - 40, plotAreaBottom + 5, 80, AxisLabelSize), Qt::AlignHCenter, tickValue);
        } else {
            tickValue.remove(0, tickValue.size() - displayLastDigits);
            QRect bounding;
            p.drawText(QRect(xCoord - 40, plotAreaBottom + 5, 80, AxisLabelSize), Qt::AlignHCenter, tickValue, &bounding);
            p.setPen(QPen(QColor("orange")));
            p.drawText(QRect(0, plotAreaBottom + 5, bounding.x() - 1, AxisLabelSize), Qt::AlignRight, "..");
            p.setPen(QPen(pref.General.graphColors.axis, 1));
        }
        p.drawLine(xCoord, plotAreaBottom, xCoord, plotAreaBottom + 2);
        if(xCoord != plotAreaLeft && xCoord != plotAreaLeft + plotAreaWidth) {
            p.setPen(QPen(pref.General.graphColors.divisions, 0.5, Qt::DashLine));
            p.drawLine(xCoord, 0, xCoord, plotAreaBottom);
        }
    }

    for(int i=0;i<2;i++) {
        if(YAxis[i].type == YAxisType::Disabled) {
            continue;
        }
        QString labelY;
        p.setPen(QPen(pref.General.graphColors.axis, 1));
        switch(YAxis[i].type) {
        case YAxisType::Magnitude: labelY = "Magnitude"; break;
        case YAxisType::Phase: labelY = "Phase"; break;
        case YAxisType::VSWR: labelY = "VSWR"; break;
        case YAxisType::Impulse: labelY = "Impulse Response"; break;
        case YAxisType::Step: labelY = "Step Response"; break;
        case YAxisType::Impedance: labelY = "Impedance"; break;
        }
        auto xStart = i == 0 ? 0 : w.width() - AxisLabelSize * 1.5;
        p.save();
        p.translate(xStart, w.height()-xAxisSpace);
        p.rotate(-90);
        p.drawText(QRect(0, 0, w.height()-xAxisSpace, AxisLabelSize*1.5), Qt::AlignHCenter, labelY);
        p.restore();
        // draw ticks
        if(YAxis[0].type != YAxisType::Disabled) {
            // this only works for evenly distributed ticks:
            auto max = qMax(abs(YAxis[i].ticks.front()), abs(YAxis[i].ticks.back()));
            auto step = abs(YAxis[i].ticks[0] - YAxis[i].ticks[1]);
            int significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
            for(auto t : YAxis[i].ticks) {
                auto yCoord = Util::Scale<double>(t, YAxis[i].rangeMax, YAxis[i].rangeMin, 0, w.height() - xAxisSpace);
                p.setPen(QPen(pref.General.graphColors.axis, 1));
                // draw tickmark on axis
                auto tickStart = i == 0 ? plotAreaLeft : plotAreaLeft + plotAreaWidth;
                auto tickLen = i == 0 ? -2 : 2;
                p.drawLine(tickStart, yCoord, tickStart + tickLen, yCoord);
                auto tickValue = Unit::ToString(t, "", "fpnum kMG", significantDigits);
                if(i == 0) {
                    p.drawText(QRectF(0, yCoord - AxisLabelSize/2 - 2, tickStart + 2 * tickLen, AxisLabelSize), Qt::AlignRight, tickValue);
                } else {
                    p.drawText(QRectF(tickStart + 2 * tickLen + 2, yCoord - AxisLabelSize/2 - 2, yAxisSpace, AxisLabelSize), Qt::AlignLeft, tickValue);
                }

                // tick lines
                if(yCoord == 0 || yCoord == w.height() - xAxisSpace) {
                    // skip tick lines right on the plot borders
                    continue;
                }
                if(i == 0) {
                    // only draw tick lines for primary axis
                    p.setPen(QPen(pref.General.graphColors.divisions, 0.5, Qt::DashLine));
                    p.drawLine(plotAreaLeft, yCoord, plotAreaLeft + plotAreaWidth, yCoord);
                }
            }
        }

        auto toPlotCoordinates = [=](double x, double y, const class YAxis& ax) -> QPointF {
            QPointF p;
            p.setX(Util::Scale<double>(x, XAxis.rangeMin, XAxis.rangeMax, plotAreaLeft, plotAreaLeft + plotAreaWidth));
            p.setY(Util::Scale<double>(y, ax.rangeMin, ax.rangeMax, plotAreaBottom, 0));
            return p;
        };

        // plot traces
        p.setClipRect(QRect(plotAreaLeft + 1, 1, plotAreaWidth - 2, plotAreaBottom - 1));
        switch(XAxis.type) {
        case XAxisType::Frequency:
            for(auto t : tracesAxis[i]) {
                if(!t->isVisible()) {
                    continue;
                }
                pen = QPen(t->color(), 1.5);
                pen.setCosmetic(true);
                if(i == 1) {
                    pen.setStyle(Qt::DotLine);
                } else {
                    pen.setStyle(Qt::SolidLine);
                }
                p.setPen(pen);
                int nPoints = t->size();
                for(int j=1;j<nPoints;j++) {
                    auto last = t->sample(j-1);
                    auto now = t->sample(j);

                    auto yLast = transformFrequencyY(last.S, YAxis[i].type);
                    auto yNow = transformFrequencyY(now.S, YAxis[i].type);

                    if(isnan(yLast) || isnan(yNow) || isinf(yLast) || isinf(yNow)) {
                        continue;
                    }

                    // scale to plot coordinates
                    auto p1 = toPlotCoordinates(last.frequency, yLast, YAxis[i]);
                    auto p2 = toPlotCoordinates(now.frequency, yNow, YAxis[i]);
                    // draw line
                    p.drawLine(p1, p2);
                }
            }
            break;
        case XAxisType::Time:
        case XAxisType::Distance:
            // TODO
            break;
        }
        p.setClipping(false);
    }
}

void TraceXYPlot::updateAxisTicks()
{
    auto createEvenlySpacedTicks = [](vector<double>& ticks, double start, double stop, double step) {
        ticks.clear();
        for(double tick = start;tick <= stop;tick+= step) {
            ticks.push_back(tick);
        }
    };

    if(XAxis.mode == XAxisMode::Manual) {
        createEvenlySpacedTicks(XAxis.ticks, XAxis.rangeMin, XAxis.rangeMax, XAxis.rangeDiv);
    } else {
        XAxis.ticks.clear();
        // automatic mode, figure out limits
        double max = std::numeric_limits<double>::lowest();
        double min = std::numeric_limits<double>::max();
        if(XAxis.mode == XAxisMode::UseSpan) {
            min = sweep_fmin;
            max = sweep_fmax;
        } else if(XAxis.mode == XAxisMode::FitTraces) {
            for(auto t : traces) {
                bool enabled = (tracesAxis[0].find(t.first) != tracesAxis[0].end()
                        || tracesAxis[1].find(t.first) != tracesAxis[1].end());
                auto trace = t.first;
                if(enabled && trace->isVisible()) {
                    // this trace is currently displayed
                    double trace_min = std::numeric_limits<double>::max();
                    double trace_max = std::numeric_limits<double>::lowest();
                    switch(XAxis.type) {
                    case XAxisType::Frequency:
                        trace_min = trace->minFreq();
                        trace_max = trace->maxFreq();
                        break;
                    case XAxisType::Time:
                        trace_min = trace->getTDR().front().time;
                        trace_max = trace->getTDR().back().time;
                        break;
                    case XAxisType::Distance:
                        trace_min = trace->getTDR().front().distance;
                        trace_max = trace->getTDR().back().distance;
                        break;
                    }
                    if(trace_min < min) {
                        min = trace_min;
                    }
                    if(trace_max > max) {
                        max = trace_max;
                    }
                }
            }
        }
        if(min >= max) {
            // still at initial values, no traces are active, leave axis unchanged
            return;
        }
        XAxis.rangeMin = min;
        XAxis.rangeMax = max;
        constexpr int minDivisions = 8;
        double max_div_step = (max - min) / minDivisions;
        int zeros = floor(log10(max_div_step));
        double decimals_shift = pow(10, zeros);
        max_div_step /= decimals_shift;
        if(max_div_step >= 5) {
            max_div_step = 5;
        } else if(max_div_step >= 2) {
            max_div_step = 2;
        } else {
            max_div_step = 1;
        }
        auto div_step = max_div_step * decimals_shift;
        XAxis.rangeDiv = div_step;
        // round min up to next multiple of div_step
        auto start_div = ceil(min / div_step) * div_step;
        for(double tick = start_div;tick <= max;tick += div_step) {
            XAxis.ticks.push_back(tick);
        }
    }

    for(int i=0;i<2;i++) {
        if(!YAxis[i].autorange) {
            createEvenlySpacedTicks(YAxis[i].ticks, YAxis[i].rangeMin, YAxis[i].rangeMax, YAxis[i].rangeDiv);
        }
    }
    triggerReplot();
}

QString TraceXYPlot::AxisTypeToName(TraceXYPlot::YAxisType type)
{
    switch(type) {
    case YAxisType::Disabled: return "Disabled"; break;
    case YAxisType::Magnitude: return "Magnitude"; break;
    case YAxisType::Phase: return "Phase"; break;
    case YAxisType::VSWR: return "VSWR"; break;
    default: return "Unknown"; break;
    }
}

void TraceXYPlot::enableTraceAxis(Trace *t, int axis, bool enabled)
{
    if(axis == 0) {
        traces[t] = enabled;
    }
    bool alreadyEnabled = tracesAxis[axis].find(t) != tracesAxis[axis].end();
    if(alreadyEnabled != enabled) {
        if(enabled) {
            tracesAxis[axis].insert(t);
//            CurveData cd;
//            cd.data = createQwtSeriesData(*t, axis);
//            cd.curve = new QwtPlotPiecewiseCurve();
//            cd.curve->attach(plot);
//            cd.curve->setYAxis(axis == 0 ? QwtPlot::yLeft : QwtPlot::yRight);
//            cd.curve->setSamples(cd.data);
//            curves[axis][t] = cd;
            // connect signals
            connect(t, &Trace::dataChanged, this, &TraceXYPlot::triggerReplot);
//            connect(t, &Trace::colorChanged, this, &TraceXYPlot::traceColorChanged);
//            connect(t, &Trace::visibilityChanged, this, &TraceXYPlot::traceColorChanged);
            connect(t, &Trace::visibilityChanged, this, &TraceXYPlot::triggerReplot);
            if(axis == 0) {
                connect(t, &Trace::markerAdded, this, &TraceXYPlot::markerAdded);
                connect(t, &Trace::markerRemoved, this, &TraceXYPlot::markerRemoved);
                auto tracemarkers = t->getMarkers();
                for(auto m : tracemarkers) {
                    markerAdded(m);
                }
            }
            if(isTDRtype(YAxis[axis].type)) {
                t->addTDRinterest();
            }
//            traceColorChanged(t);
        } else {
            if(isTDRtype(YAxis[axis].type)) {
                t->removeTDRinterest();
            }
            tracesAxis[axis].erase(t);
            // clean up and delete
//            if(curves[axis].find(t) != curves[axis].end()) {
//                delete curves[axis][t].curve;
//                curves[axis].erase(t);
//            }
            int otherAxis = axis == 0 ? 1 : 0;
//            if(curves[otherAxis].find(t) == curves[otherAxis].end()) {
//                // this trace is not used anymore, disconnect from notifications
//                disconnect(t, &Trace::dataChanged, this, &TraceXYPlot::triggerReplot);
//                disconnect(t, &Trace::colorChanged, this, &TraceXYPlot::traceColorChanged);
//                disconnect(t, &Trace::visibilityChanged, this, &TraceXYPlot::traceColorChanged);
//                disconnect(t, &Trace::visibilityChanged, this, &TraceXYPlot::triggerReplot);
//            }
            if(axis == 0) {
                disconnect(t, &Trace::markerAdded, this, &TraceXYPlot::markerAdded);
                disconnect(t, &Trace::markerRemoved, this, &TraceXYPlot::markerRemoved);
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

bool TraceXYPlot::supported(Trace *t, TraceXYPlot::YAxisType type)
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

void TraceXYPlot::updateXAxis()
{
    if(XAxis.mode == XAxisMode::Manual) {
//        plot->setAxisScale(QwtPlot::xBottom, XAxis.rangeMin, XAxis.rangeMax, XAxis.rangeDiv);
    } else {
        // automatic mode, figure out limits
        double max = std::numeric_limits<double>::lowest();
        double min = std::numeric_limits<double>::max();
        if(XAxis.mode == XAxisMode::UseSpan) {
            min = sweep_fmin;
            max = sweep_fmax;
        } else if(XAxis.mode == XAxisMode::FitTraces) {
            for(auto t : traces) {
                bool enabled = (tracesAxis[0].find(t.first) != tracesAxis[0].end()
                        || tracesAxis[1].find(t.first) != tracesAxis[1].end());
                auto trace = t.first;
                if(enabled && trace->isVisible()) {
                    // this trace is currently displayed
                    double trace_min = std::numeric_limits<double>::max();
                    double trace_max = std::numeric_limits<double>::lowest();
                    switch(XAxis.type) {
                    case XAxisType::Frequency:
                        trace_min = trace->minFreq();
                        trace_max = trace->maxFreq();
                        break;
                    case XAxisType::Time:
                        trace_min = trace->getTDR().front().time;
                        trace_max = trace->getTDR().back().time;
                        break;
                    case XAxisType::Distance:
                        trace_min = trace->getTDR().front().distance;
                        trace_max = trace->getTDR().back().distance;
                        break;
                    }
                    if(trace_min < min) {
                        min = trace_min;
                    }
                    if(trace_max > max) {
                        max = trace_max;
                    }
                }
            }
        }
        if(min >= max) {
            // still at initial values, no traces are active, leave axis unchanged
            return;
        }
        constexpr int minDivisions = 8;
        double max_div_step = (max - min) / minDivisions;
        int zeros = floor(log10(max_div_step));
        double decimals_shift = pow(10, zeros);
        max_div_step /= decimals_shift;
        if(max_div_step >= 5) {
            max_div_step = 5;
        } else if(max_div_step >= 2) {
            max_div_step = 2;
        } else {
            max_div_step = 1;
        }
        auto div_step = max_div_step * decimals_shift;
        // round min up to next multiple of div_step
        auto start_div = ceil(min / div_step) * div_step;
        QList<double> tickList;
        for(double tick = start_div;tick <= max;tick += div_step) {
            tickList.append(tick);
        }
//        QwtScaleDiv scalediv(min, max, QList<double>(), QList<double>(), tickList);
//        plot->setAxisScaleDiv(QwtPlot::xBottom, scalediv);
    }
}

double TraceXYPlot::transformFrequencyY(std::complex<double> data, TraceXYPlot::YAxisType type)
{
    switch(type) {
    case YAxisType::Magnitude:
        return 20*log10(abs(data));
    case YAxisType::Phase:
        return arg(data) * 180.0 / M_PI;
    case YAxisType::VSWR:
        if(abs(data) < 1.0) {
            return (1+abs(data)) / (1-abs(data));
        }
        break;
    }
    return numeric_limits<double>::quiet_NaN();
}

//QwtSeriesData<QPointF> *TraceXYPlot::createQwtSeriesData(Trace &t, int axis)
//{
//    return new QwtTraceSeries(t, YAxis[axis].type, this);
//}

//void TraceXYPlot::traceColorChanged(Trace *t)
//{
//    for(int axis = 0;axis < 2;axis++) {
//        if(curves[axis].find(t) != curves[axis].end()) {
//            // trace active, change the pen color
//            if(t->isVisible()) {
//                if(axis == 0) {
//                    curves[axis][t].curve->setPen(t->color());
//                } else {
//                    curves[axis][t].curve->setPen(t->color(), 1.0, Qt::DashLine);
//                }
//                for(auto m : t->getMarkers()) {
//                    if(markers.count(m)) {
//                        markers[m]->attach(plot);
//                    }
//                }
//            } else {
//                curves[axis][t].curve->setPen(t->color(), 0.0, Qt::NoPen);
//                for(auto m : t->getMarkers()) {
//                    if(markers.count(m)) {
//                        markers[m]->detach();
//                    }
//                }
//            }
//        }
//    }
//}

//void TraceXYPlot::markerAdded(TraceMarker *m)
//{
//    if(markers.count(m)) {
//        return;
//    }
//    auto qwtMarker = new QwtPlotMarker;
//    markers[m] = qwtMarker;
//    markerSymbolChanged(m);
//    connect(m, &TraceMarker::symbolChanged, this, &TraceXYPlot::markerSymbolChanged);
//    connect(m, &TraceMarker::dataChanged, this, &TraceXYPlot::markerDataChanged);
//    markerDataChanged(m);
//    qwtMarker->attach(plot);
//    triggerReplot();
//}

//void TraceXYPlot::markerRemoved(TraceMarker *m)
//{
//    disconnect(m, &TraceMarker::symbolChanged, this, &TraceXYPlot::markerSymbolChanged);
//    disconnect(m, &TraceMarker::dataChanged, this, &TraceXYPlot::markerDataChanged);
//    if(markers.count(m)) {
//        markers[m]->detach();
//        delete markers[m];
//        markers.erase(m);
//    }
//    triggerReplot();
//}

//void TraceXYPlot::markerDataChanged(TraceMarker *m)
//{
//    auto qwtMarker = markers[m];
//    qwtMarker->setXValue(m->getFrequency());
//    qwtMarker->setYValue(FrequencyAxisTransformation(YAxis[0].type, m->getData()));
//    triggerReplot();
//}

//void TraceXYPlot::markerSymbolChanged(TraceMarker *m)
//{
//    auto qwtMarker = markers[m];
//    qwtMarker->setSymbol(nullptr);

//    QwtSymbol *sym=new QwtSymbol;
//    sym->setPixmap(m->getSymbol());
//    sym->setPinPoint(QPointF(m->getSymbol().width()/2, m->getSymbol().height()));
//    qwtMarker->setSymbol(sym);
//    triggerReplot();
//}

//void TraceXYPlot::clicked(const QPointF pos)
//{
//    auto clickPoint = drawPicker->plotToPixel(pos);
//    unsigned int closestDistance = numeric_limits<unsigned int>::max();
//    TraceMarker *closestMarker = nullptr;
//    for(auto m : markers) {
//        if(!m.first->isMovable()) {
//            continue;
//        }
//        auto markerPoint = drawPicker->plotToPixel(m.second->value());
//        auto yDiff = abs(markerPoint.y() - clickPoint.y());
//        auto xDiff = abs(markerPoint.x() - clickPoint.x());
//        unsigned int distance = xDiff * xDiff + yDiff * yDiff;
//        if(distance < closestDistance) {
//            closestDistance = distance;
//            closestMarker = m.first;
//        }
//    }
//    if(closestDistance <= 400) {
//        selectedMarker = closestMarker;
//        selectedCurve = curves[0][selectedMarker->trace()].curve;
//    } else {
//        selectedMarker = nullptr;
//        selectedCurve = nullptr;
//    }
//}

//void TraceXYPlot::moved(const QPointF pos)
//{
//    if(!selectedMarker || !selectedCurve) {
//        return;
//    }
//    selectedMarker->setFrequency(pos.x());
//}

//void TraceXYPlot::setColorFromPreferences()
//{
//    auto pref = Preferences::getInstance();
//    plot->setCanvasBackground(pref.General.graphColors.background);
//    auto pal = plot->palette();
//    pal.setColor(QPalette::Window, pref.General.graphColors.background);
//    pal.setColor(QPalette::WindowText, pref.General.graphColors.axis);
//    pal.setColor(QPalette::Text, pref.General.graphColors.axis);
//    plot->setPalette(pal);
//    grid->setPen(pref.General.graphColors.divisions);
//}

