#include "tracexyplot.h"
#include <QGridLayout>
#include "trace.h"
#include <cmath>
#include <QFrame>
#include "tracemarker.h"
#include "xyplotaxisdialog.h"
#include <preferences.h>
#include <QPainter>
#include "Util/util.h"
#include "unit.h"
#include <QDebug>

using namespace std;

const set<TraceXYPlot::YAxisType> TraceXYPlot::YAxisTypes = {TraceXYPlot::YAxisType::Disabled,
                                           TraceXYPlot::YAxisType::Magnitude,
                                           TraceXYPlot::YAxisType::Phase,
                                           TraceXYPlot::YAxisType::VSWR,
                                           TraceXYPlot::YAxisType::Impulse,
                                           TraceXYPlot::YAxisType::Step,
                                           TraceXYPlot::YAxisType::Impedance};

TraceXYPlot::TraceXYPlot(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
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

    // Setup default axis
    setYAxis(0, YAxisType::Magnitude, false, false, -120, 20, 10);
    setYAxis(1, YAxisType::Phase, false, false, -180, 180, 30);
    // enable autoscaling and set for full span (no information about actual span available yet)
    updateSpan(0, 6000000000);
    setXAxis(XAxisType::Frequency, XAxisMode::UseSpan, 0, 6000000000, 600000000);
    initializeTraceInfo();
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

        auto oldType = YAxis[axis].type;
        if(isTDRtype(YAxis[axis].type) && !isTDRtype(type)) {
            // not TDR axis anymore
            for(auto t : tracesAxis[axis]) {
                t->removeTDRinterest();
            }
        }
        YAxis[axis].type = type;
        if(isTDRtype(type) && !isTDRtype(oldType)) {
            // now a TDR axis
            for(auto t : tracesAxis[axis]) {
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

void TraceXYPlot::replot()
{
    if(XAxis.mode != XAxisMode::Manual || YAxis[0].autorange || YAxis[1].autorange) {
        updateAxisTicks();
    }
    TracePlot::replot();
}

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
    if(YAxis[0].type != YAxisType::Disabled || YAxis[1].type != YAxisType::Disabled) {
        return true;
    } else {
        // no axis
        return false;
    }
}

void TraceXYPlot::draw(QPainter &p)
{
    auto pref = Preferences::getInstance();

    constexpr int yAxisSpace = 50;
    constexpr int yAxisDisabledSpace = 10;
    constexpr int xAxisSpace = 30;
    auto w = p.window();
    auto pen = QPen(pref.General.graphColors.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    plotAreaLeft = YAxis[0].type == YAxisType::Disabled ? yAxisDisabledSpace : yAxisSpace;
    plotAreaWidth = w.width();
    plotAreaBottom = w.height() - xAxisSpace;
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

    auto plotRect = QRect(plotAreaLeft, 0, plotAreaWidth + 1, plotAreaBottom);
    p.drawRect(plotRect);

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
    if(XAxis.ticks.size() >= 1) {
        // draw X ticks
        // this only works for evenly distributed ticks:
        auto max = qMax(abs(XAxis.ticks.front()), abs(XAxis.ticks.back()));
        auto minLabel = qMin(abs(XAxis.ticks.front()), abs(XAxis.ticks.back()));
        double step;
        if(XAxis.ticks.size() >= 2) {
            step = abs(XAxis.ticks[0] - XAxis.ticks[1]);
        } else {
            // only one tick, set arbitrary number of digits
            step = max / 1000;
        }
        if(minLabel > 0 && minLabel < step) {
            step = minLabel;
        }
        int significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
        bool displayFullFreq = significantDigits <= 5;
        constexpr int displayLastDigits = 4;
        QString prefixes = "fpnum kMG";
        QString commonPrefix = QString();
        if(!displayFullFreq) {
            auto fullFreq = Unit::ToString(XAxis.ticks.front(), "", prefixes, significantDigits);
            commonPrefix = fullFreq.at(fullFreq.size() - 1);
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
            auto xCoord = Util::Scale<double>(t, XAxis.rangeMin, XAxis.rangeMax, plotAreaLeft, plotAreaLeft + plotAreaWidth);
            auto tickValue = Unit::ToString(t, "", prefixes, significantDigits);
            p.setPen(QPen(pref.General.graphColors.axis, 1));
            if(displayFullFreq) {
                p.drawText(QRect(xCoord - 40, plotAreaBottom + 5, 80, AxisLabelSize), Qt::AlignHCenter, tickValue);
            } else {
                // check if the same prefix was used as in the fullFreq string
                if(tickValue.at(tickValue.size() - 1) != commonPrefix) {
                    // prefix changed, we reached the next order of magnitude. Force same prefix as in fullFreq and add extra digit
                    tickValue = Unit::ToString(t, "", commonPrefix, significantDigits + 1);
                }

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
        default: break;
        }
        auto xStart = i == 0 ? 0 : w.width() - AxisLabelSize * 1.5;
        p.save();
        p.translate(xStart, w.height()-xAxisSpace);
        p.rotate(-90);
        p.drawText(QRect(0, 0, w.height()-xAxisSpace, AxisLabelSize*1.5), Qt::AlignHCenter, labelY);
        p.restore();
        // draw ticks
        if(YAxis[i].type != YAxisType::Disabled && YAxis[i].ticks.size() > 0) {
            // this only works for evenly distributed ticks:
            auto max = qMax(abs(YAxis[i].ticks.front()), abs(YAxis[i].ticks.back()));
            double step;
            if(YAxis[i].ticks.size() >= 2) {
                step = abs(YAxis[i].ticks[0] - YAxis[i].ticks[1]);
            } else {
                // only one tick, set arbitrary number of digits
                step = max / 1000;
            }
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

        // plot traces
        p.setClipRect(QRect(plotRect.x()+1, plotRect.y()+1, plotRect.width()-2, plotRect.height()-2));
        for(auto t : tracesAxis[i]) {
            if(!t->isVisible()) {
                continue;
            }
            pen = QPen(t->color(), 1);
            pen.setCosmetic(true);
            if(i == 1) {
                pen.setStyle(Qt::DotLine);
            } else {
                pen.setStyle(Qt::SolidLine);
            }
            p.setPen(pen);
            auto nPoints = numTraceSamples(t);
            for(unsigned int j=1;j<nPoints;j++) {
                auto last = traceToCoordinate(t, j-1, YAxis[i].type);
                auto now = traceToCoordinate(t, j, YAxis[i].type);

                if(isnan(last.y()) || isnan(now.y()) || isinf(last.y()) || isinf(now.y())) {
                    continue;
                }

                // scale to plot coordinates
                auto p1 = plotValueToPixel(last, i);
                auto p2 = plotValueToPixel(now, i);
                if(!plotRect.contains(p1) && !plotRect.contains(p2)) {
                    // completely out of frame
                    continue;
                }
                // draw line
                p.drawLine(p1, p2);
            }
            if(i == 0 && nPoints > 0) {
                // only draw markers on primary YAxis and if the trace has at least one point
                auto markers = t->getMarkers();
                for(auto m : markers) {
                    if(m->isTimeDomain() != (XAxis.type != XAxisType::Frequency)) {
                        // wrong domain, skip this marker
                        continue;
                    }
                    double xPosition;
                    if(m->isTimeDomain()) {
                        if(XAxis.type == XAxisType::Distance) {
                            xPosition = m->getTimeData().distance;
                        } else {
                            xPosition = m->getTimeData().time;
                        }
                    } else {
                        xPosition = m->getPosition();
                    }
                    if (xPosition < XAxis.rangeMin || xPosition > XAxis.rangeMax) {
                        continue;
                    }
                    QPointF markerPoint = QPointF(xPosition, traceToCoordinate(m->getData(), YAxis[i].type));
                    auto point = plotValueToPixel(markerPoint, i);
                    if(!plotRect.contains(point)) {
                        // out of screen
                        continue;
                    }
                    auto symbol = m->getSymbol();
                    point += QPoint(-symbol.width()/2, -symbol.height());
                    p.drawPixmap(point, symbol);
                }
            }
        }
        p.setClipping(false);
    }

    if(dropPending) {
        p.setOpacity(0.5);
        p.setBrush(Qt::white);
        p.setPen(Qt::white);
        if(YAxis[0].type == YAxisType::Disabled || YAxis[1].type == YAxisType::Disabled) {
            // only one axis enabled, show drop area over whole plot
            p.drawRect(plotRect);
            auto font = p.font();
            font.setPixelSize(20);
            p.setFont(font);
            p.setOpacity(1.0);
            p.setPen(Qt::white);
            auto text = "Drop here to add\n" + dropTrace->name() + "\nto XY-plot";
            p.drawText(plotRect, Qt::AlignCenter, text);
        } else {
            // both axis enabled, show regions
            auto leftRect = plotRect;
            leftRect.setWidth(plotRect.width() * 0.3);
            auto centerRect = plotRect;
            centerRect.setX(centerRect.x() + plotRect.width() * 0.35);
            centerRect.setWidth(plotRect.width() * 0.3);
            auto rightRect = plotRect;
            rightRect.setX(rightRect.x() + plotRect.width() * 0.7);
            rightRect.setWidth(plotRect.width() * 0.3);
            p.drawRect(leftRect);
            p.drawRect(centerRect);
            p.drawRect(rightRect);
            p.setOpacity(1.0);
            p.setPen(Qt::white);
            auto font = p.font();
            font.setPixelSize(20);
            p.setFont(font);
            p.drawText(leftRect, Qt::AlignCenter, "Drop here to add\nto primary axis");
            p.drawText(centerRect, Qt::AlignCenter, "Drop here to add\nto boths axes");
            p.drawText(rightRect, Qt::AlignCenter, "Drop here to add\nto secondary axis");
        }
    }
}

void TraceXYPlot::updateAxisTicks()
{
    auto createEvenlySpacedTicks = [](vector<double>& ticks, double start, double stop, double step) {
        ticks.clear();
        if(start > stop) {
            swap(start, stop);
        }
        step = abs(step);
        constexpr unsigned int maxTicks = 100;
        for(double tick = start; tick - stop < numeric_limits<double>::epsilon() && ticks.size() <= maxTicks;tick+= step) {
            ticks.push_back(tick);
        }
    };

    auto createAutomaticTicks = [](vector<double>& ticks, double start, double stop, int minDivisions) -> double {
        Q_ASSERT(stop > start);
        ticks.clear();
        double max_div_step = (stop - start) / minDivisions;
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
        auto start_div = ceil(start / div_step) * div_step;
        for(double tick = start_div;tick <= stop;tick += div_step) {
            ticks.push_back(tick);
        }
        return div_step;
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
                    if(!numTraceSamples(trace)) {
                        // empty trace, do not use for automatic axis calculation
                        continue;
                    }
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
        if(min < max) {
            // found min/max values
            XAxis.rangeMin = min;
            XAxis.rangeMax = max;
            XAxis.rangeDiv = createAutomaticTicks(XAxis.ticks, min, max, 8);
        }
    }

    for(int i=0;i<2;i++) {
        if(!YAxis[i].autorange) {
            createEvenlySpacedTicks(YAxis[i].ticks, YAxis[i].rangeMin, YAxis[i].rangeMax, YAxis[i].rangeDiv);
        } else {
            // automatic mode, figure out limits
            double max = std::numeric_limits<double>::lowest();
            double min = std::numeric_limits<double>::max();
            for(auto t : tracesAxis[i]) {
                unsigned int samples = numTraceSamples(t);
                for(unsigned int j=0;j<samples;j++) {
                    auto point = traceToCoordinate(t, j, YAxis[i].type);

                    if(point.x() < XAxis.rangeMin || point.x() > XAxis.rangeMax) {
                        // this point is not in the displayed X range, skip for auto Y range calculation
                        continue;
                    }

                    if(point.y() > max) {
                        max = point.y();
                    } else if(point.y() < min) {
                        min = point.y();
                    }
                }
            }
            if(max >= min) {
                auto range = max - min;
                if(range == 0.0) {
                    // this could happen if all values in a trace are identical (e.g. imported ideal touchstone files)
                    if(max == 0.0) {
                        // simply use +/-1 range
                        max = 1.0;
                        min = -1.0;
                    } else {
                        // +/-5% around value
                        max += abs(max * 0.05);
                        min -= abs(max * 0.05);
                    }
                } else {
                    // add 5% of range at both ends
                    min -= range * 0.05;
                    max += range * 0.05;
                }
                YAxis[i].rangeMin = min;
                YAxis[i].rangeMax = max;
                YAxis[i].rangeDiv = createAutomaticTicks(YAxis[i].ticks, min, max, 8);
            }
        }
    }
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
        TracePlot::enableTrace(t, enabled);
    }
    bool alreadyEnabled = tracesAxis[axis].find(t) != tracesAxis[axis].end();
    if(alreadyEnabled != enabled) {
        if(enabled) {
            tracesAxis[axis].insert(t);
            // connect signals
            if(isTDRtype(YAxis[axis].type)) {
                t->addTDRinterest();
            }
        } else {
            if(isTDRtype(YAxis[axis].type)) {
                t->removeTDRinterest();
            }
            tracesAxis[axis].erase(t);
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

double TraceXYPlot::traceToCoordinate(std::complex<double> data, TraceXYPlot::YAxisType type)
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
    case YAxisType::Step:
        return data.real();
    case YAxisType::Impulse:
        return data.imag();
    case YAxisType::Impedance:
        if(abs(data.real()) < 1.0) {
            return 50 * (1+data.real()) / (1-data.real());
        }
    default:
        break;
    }
    return numeric_limits<double>::quiet_NaN();
}

QPointF TraceXYPlot::traceToCoordinate(Trace *t, unsigned int sample, TraceXYPlot::YAxisType type)
{
    QPointF ret = QPointF(numeric_limits<double>::quiet_NaN(), numeric_limits<double>::quiet_NaN());
    switch(type) {
    case YAxisType::Magnitude:
    case YAxisType::Phase:
    case YAxisType::VSWR: {
        auto d = t->sample(sample);
        ret.setY(traceToCoordinate(d.S, type));
        ret.setX(d.frequency);
    }
        break;
    case YAxisType::Impulse:
        ret.setY(t->getTDR()[sample].impulseResponse);
        if(XAxis.type == XAxisType::Distance) {
            ret.setX(t->getTDR()[sample].distance);
        } else {
            ret.setX(t->getTDR()[sample].time);
        }
        break;
    case YAxisType::Step:
        ret.setY(t->getTDR()[sample].stepResponse);
        if(XAxis.type == XAxisType::Distance) {
            ret.setX(t->getTDR()[sample].distance);
        } else {
            ret.setX(t->getTDR()[sample].time);
        }
        break;
    case YAxisType::Impedance: {
        ret.setY(t->getTDR()[sample].impedance);
        if(XAxis.type == XAxisType::Distance) {
            ret.setX(t->getTDR()[sample].distance);
        } else {
            ret.setX(t->getTDR()[sample].time);
        }
    }
        break;
    case YAxisType::Disabled:
    case YAxisType::Last:
        // no valid axis
        break;
    }
    return ret;
}

unsigned int TraceXYPlot::numTraceSamples(Trace *t)
{
    if(XAxis.type == XAxisType::Frequency) {
        return t->size();
    } else {
        return t->getTDR().size();
    }
}

QPoint TraceXYPlot::dataToPixel(Trace::Data d)
{
    if(d.frequency < XAxis.rangeMin || d.frequency > XAxis.rangeMax) {
        return QPoint();
    }
    auto y = traceToCoordinate(d.S, YAxis[0].type);
    QPoint p;
    p.setX(Util::Scale<double>(d.frequency, XAxis.rangeMin, XAxis.rangeMax, plotAreaLeft, plotAreaLeft + plotAreaWidth));
    p.setY(Util::Scale<double>(y, YAxis[0].rangeMin, YAxis[0].rangeMax, plotAreaBottom, 0));
    return p;
}

QPoint TraceXYPlot::plotValueToPixel(QPointF plotValue, int Yaxis)
{
    QPoint p;
    p.setX(Util::Scale<double>(plotValue.x(), XAxis.rangeMin, XAxis.rangeMax, plotAreaLeft, plotAreaLeft + plotAreaWidth));
    p.setY(Util::Scale<double>(plotValue.y(), YAxis[Yaxis].rangeMin, YAxis[Yaxis].rangeMax, plotAreaBottom, 0));
    return p;
}

QPointF TraceXYPlot::pixelToPlotValue(QPoint pixel, int Yaxis)
{
    QPointF p;
    p.setX(Util::Scale<double>(pixel.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth, XAxis.rangeMin, XAxis.rangeMax));
    p.setY(Util::Scale<double>(pixel.y(), plotAreaBottom, 0, YAxis[Yaxis].rangeMin, YAxis[Yaxis].rangeMax));
    return p;
}

QPoint TraceXYPlot::markerToPixel(TraceMarker *m)
{
    QPoint ret = QPoint();
    if(m->isTimeDomain() != (XAxis.type != XAxisType::Frequency)) {
        // invalid domain
        return ret;
    }
    QPointF plotPoint;
    plotPoint.setY(traceToCoordinate(m->getData(), YAxis[0].type));
    if(m->isTimeDomain()) {
        auto timedata = m->getTimeData();
        if(XAxis.type == XAxisType::Distance) {
            plotPoint.setX(timedata.distance);
        } else {
            plotPoint.setX(timedata.time);
        }
    } else {
        plotPoint.setX(m->getPosition());
    }
    return plotValueToPixel(plotPoint, 0);
}

double TraceXYPlot::nearestTracePoint(Trace *t, QPoint pixel)
{
    if(!tracesAxis[0].count(t)) {
        // trace not enabled
        return 0;
    }
    double closestDistance = numeric_limits<double>::max();
    double closestXpos = 0;
    auto samples = numTraceSamples(t);
    for(unsigned int i=0;i<samples;i++) {
        auto point = traceToCoordinate(t, i, YAxis[0].type);
        if(isnan(point.x()) || isnan(point.y())) {
            continue;
        }
        auto plotPoint = plotValueToPixel(point, 0);
        QPointF diff = plotPoint - pixel;
        auto distance = diff.x() * diff.x() + diff.y() * diff.y();
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = point.x();
        }
    }
    return closestXpos;
}

void TraceXYPlot::traceDropped(Trace *t, QPoint position)
{
    if(YAxis[0].type == YAxisType::Disabled && YAxis[1].type == YAxisType::Disabled) {
        // no Y axis enabled, unable to drop
        return;
    }
    if(YAxis[0].type == YAxisType::Disabled) {
        // only axis 1 enabled
        enableTraceAxis(t, 1, true);
        return;
    }
    if(YAxis[1].type == YAxisType::Disabled) {
        // only axis 0 enabled
        enableTraceAxis(t, 0, true);
        return;
    }
    // both axis enabled, check drop position
    auto drop = Util::Scale<double>(position.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth, 0.0, 1.0);
    if(drop < 0.66) {
        enableTraceAxis(t, 0, true);
    }
    if(drop > 0.33) {
        enableTraceAxis(t, 1, true);
    }
}

QString TraceXYPlot::mouseText(QPoint pos)
{
    QString ret;
    if(QRect(plotAreaLeft, 0, plotAreaWidth + 1, plotAreaBottom).contains(pos)) {
        // cursor within plot area
        QPointF coords[2];
        coords[0] = pixelToPlotValue(pos, 0);
        coords[1] = pixelToPlotValue(pos, 1);
        int significantDigits = floor(log10(XAxis.rangeMax)) - floor(log10((XAxis.rangeMax - XAxis.rangeMin) / 1000.0)) + 1;
        ret += Unit::ToString(coords[0].x(), AxisUnit(XAxis.type), "fpnum kMG", significantDigits) + "\n";
        for(int i=0;i<2;i++) {
            if(YAxis[i].type != YAxisType::Disabled) {
                auto max = qMax(abs(YAxis[i].rangeMax), abs(YAxis[i].rangeMin));
                auto step = abs(YAxis[i].rangeMax - YAxis[i].rangeMin) / 1000.0;
                significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
                ret += Unit::ToString(coords[i].y(), AxisUnit(YAxis[i].type), "fpnum kMG", significantDigits) + "\n";
            }
        }
    }
    return ret;
}

QString TraceXYPlot::AxisUnit(TraceXYPlot::YAxisType type)
{
    switch(type) {
    case TraceXYPlot::YAxisType::Magnitude: return "db"; break;
    case TraceXYPlot::YAxisType::Phase: return "°"; break;
    case TraceXYPlot::YAxisType::VSWR: return ""; break;
    case TraceXYPlot::YAxisType::Impulse: return ""; break;
    case TraceXYPlot::YAxisType::Step: return ""; break;
    case TraceXYPlot::YAxisType::Impedance: return "Ohm"; break;
    default: return ""; break;
    }
}

QString TraceXYPlot::AxisUnit(TraceXYPlot::XAxisType type)
{
    switch(type) {
    case XAxisType::Frequency: return "Hz"; break;
    case XAxisType::Time: return "s"; break;
    case XAxisType::Distance: return "m"; break;
    }
}
