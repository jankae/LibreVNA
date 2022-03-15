#include "tracewaterfall.h"

#include "preferences.h"
#include "unit.h"
#include "Util/util.h"

#include <QFileDialog>
#include <QPainter>

using namespace std;

TraceWaterfall::TraceWaterfall(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent),
    pixelsPerLine(1)
{
    initializeTraceInfo();
}

void TraceWaterfall::enableTrace(Trace *t, bool enabled)
{
    if(enabled) {
        // only one trace at a time is allowed, disable all others
        for(auto t : traces) {
            if(t.second) {
                TracePlot::enableTrace(t.first, false);
            }
        }
    }
    TracePlot::enableTrace(t, enabled);
    resetWaterfall();
}

void TraceWaterfall::updateSpan(double min, double max)
{
    TracePlot::updateSpan(min, max);
    updateAxisTicks();
    resetWaterfall();
}

void TraceWaterfall::replot()
{
    if(XAxis.mode != XAxisMode::Manual) {
        updateAxisTicks();
    }
    TracePlot::replot();
}

void TraceWaterfall::fromJSON(nlohmann::json j)
{
    resetWaterfall();
    pixelsPerLine = j.value("pixelsPerLine", 1);
    for(unsigned int hash : j["traces"]) {
        // attempt to find the traces with this hash
        bool found = false;
        for(auto t : model.getTraces()) {
            if(t->toHash() == hash) {
                enableTrace(t, true);
                found = true;
                break;
            }
        }
        if(!found) {
            qWarning() << "Unable to find trace with hash" << hash;
        }
    }
}

nlohmann::json TraceWaterfall::toJSON()
{
    nlohmann::json j;
    j["pixelsPerLine"] = pixelsPerLine;
    nlohmann::json jtraces;
    for(auto t : traces) {
        if(t.second) {
            jtraces.push_back(t.first->toHash());
        }
    }
    j["traces"] = jtraces;
    return j;
}

void TraceWaterfall::setXAxis(XAxisType type, XAxisMode mode, bool log, double min, double max, double div)
{
    XAxis.type = type;
    XAxis.mode = mode;
    XAxis.log = log;
    XAxis.rangeMin = min;
    XAxis.rangeMax = max;
    XAxis.rangeDiv = div;
    traceRemovalPending = true;
    updateAxisTicks();
    updateContextMenu();
    replot();
}

void TraceWaterfall::axisSetupDialog()
{
    // TODO

}

void TraceWaterfall::resetWaterfall()
{
    data.clear();
}

bool TraceWaterfall::configureForTrace(Trace *t)
{
    // TODO
    return true;
}

void TraceWaterfall::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TraceWaterfall::axisSetupDialog);
    contextmenu->addAction(setup);

    contextmenu->addSeparator();
    auto image = new QAction("Save image...", contextmenu);
    contextmenu->addAction(image);
    connect(image, &QAction::triggered, [=]() {
        auto filename = QFileDialog::getSaveFileName(nullptr, "Save plot image", "", "PNG image files (*.png)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(filename.endsWith(".png")) {
            filename.chop(4);
        }
        filename += ".png";
        grab().save(filename);
    });

    contextmenu->addSection("Traces");
    // Populate context menu
    for(auto t : traces) {
        if(!supported(t.first)) {
            continue;
        }
        auto action = new QAction(t.first->name(), contextmenu);
        action->setCheckable(true);
        if(t.second) {
            action->setChecked(true);
        }
        connect(action, &QAction::toggled, [=](bool active) {
            enableTrace(t.first, active);
        });
        contextmenu->addAction(action);
    }

    contextmenu->addSeparator();
    auto close = new QAction("Close", contextmenu);
    contextmenu->addAction(close);
    connect(close, &QAction::triggered, [=]() {
        markedForDeletion = true;
    });
}

void TraceWaterfall::draw(QPainter &p)
{
    auto pref = Preferences::getInstance();

    constexpr int yAxisSpace = 55;
    constexpr int yAxisDisabledSpace = 10;
    constexpr int xAxisSpace = 30;
    auto w = p.window();
    auto pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    plotAreaLeft = yAxisDisabledSpace;
    plotAreaWidth = w.width() - 2 * yAxisDisabledSpace;
    plotAreaTop = 10;
    plotAreaBottom = w.height() - xAxisSpace;

    auto plotRect = QRect(plotAreaLeft, plotAreaTop, plotAreaWidth + 1, plotAreaBottom-plotAreaTop);
    p.drawRect(plotRect);

    // draw axis types
    auto font = p.font();
    font.setPixelSize(AxisLabelSize);
    p.setFont(font);
    p.drawText(QRect(0, w.height()-AxisLabelSize*1.5, w.width(), AxisLabelSize*1.5), Qt::AlignHCenter, AxisTypeToName(XAxis.type));

    if(XAxis.ticks.size() >= 1) {
        // draw X ticks
        int significantDigits;
        bool displayFullFreq;
        if(XAxis.log) {
            significantDigits = 5;
            displayFullFreq = true;
        } else {
            // this only works for evenly distributed ticks:
            auto max = qMax(abs(XAxis.ticks.front()), abs(XAxis.ticks.back()));
            double step;
            if(XAxis.ticks.size() >= 2) {
                step = abs(XAxis.ticks[0] - XAxis.ticks[1]);
            } else {
                // only one tick, set arbitrary number of digits
                step = max / 1000;
            }
            significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
            displayFullFreq = significantDigits <= 5;
        }
        constexpr int displayLastDigits = 4;
        QString prefixes = "fpnum kMG";
        QString unit = "";
        if(pref.Graphs.showUnits) {
            unit = AxisUnit(XAxis.type);
        }
        QString commonPrefix = QString();
        if(!displayFullFreq) {
            auto fullFreq = Unit::ToString(XAxis.ticks.front(), unit, prefixes, significantDigits);
            commonPrefix = fullFreq.at(fullFreq.size() - 1);
            auto front = fullFreq;
            front.truncate(fullFreq.size() - displayLastDigits - unit.length());
            auto back = fullFreq;
            back.remove(0, front.size());
            back.append("..");
            p.setPen(QPen(QColor("orange")));
            QRect bounding;
            p.drawText(QRect(2, plotAreaBottom + AxisLabelSize + 5, w.width(), AxisLabelSize), 0, front, &bounding);
            p.setPen(pref.Graphs.Color.axis);
            p.drawText(QRect(bounding.x() + bounding.width(), plotAreaBottom + AxisLabelSize + 5, w.width(), AxisLabelSize), 0, back);
        }

        int lastTickLabelEnd = 0;
        for(auto t : XAxis.ticks) {
            auto xCoord = Util::Scale<double>(t, XAxis.rangeMin, XAxis.rangeMax, plotAreaLeft, plotAreaLeft + plotAreaWidth, XAxis.log);
            p.setPen(QPen(pref.Graphs.Color.axis, 1));
            p.drawLine(xCoord, plotAreaBottom, xCoord, plotAreaBottom + 2);
            if(xCoord != plotAreaLeft && xCoord != plotAreaLeft + plotAreaWidth) {
                p.setPen(QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine));
                p.drawLine(xCoord, plotAreaTop, xCoord, plotAreaBottom);
            }
            if(xCoord - 40 <= lastTickLabelEnd) {
                // would overlap previous tick label, skip
                continue;
            }
            auto tickValue = Unit::ToString(t, unit, prefixes, significantDigits);
            p.setPen(QPen(pref.Graphs.Color.axis, 1));
            if(displayFullFreq) {
                QRect bounding;
                p.drawText(QRect(xCoord - 40, plotAreaBottom + 5, 80, AxisLabelSize), Qt::AlignHCenter, tickValue, &bounding);
                lastTickLabelEnd = bounding.x() + bounding.width();
            } else {
                // check if the same prefix was used as in the fullFreq string
                if(tickValue.at(tickValue.size() - 1) != commonPrefix) {
                    // prefix changed, we reached the next order of magnitude. Force same prefix as in fullFreq and add extra digit
                    tickValue = Unit::ToString(t, "", commonPrefix, significantDigits + 1);
                }

                tickValue.remove(0, tickValue.size() - displayLastDigits - unit.length());
                QRect bounding;
                p.drawText(QRect(xCoord - 40, plotAreaBottom + 5, 80, AxisLabelSize), Qt::AlignHCenter, tickValue, &bounding);
                lastTickLabelEnd = bounding.x() + bounding.width();
                p.setPen(QPen(QColor("orange")));
                p.drawText(QRect(0, plotAreaBottom + 5, bounding.x() - 1, AxisLabelSize), Qt::AlignRight, "..", &bounding);
            }
        }
    }

    if(dropPending) {
        p.setOpacity(0.5);
        p.setBrush(Qt::white);
        p.setPen(Qt::white);
        // show drop area over whole plot
        p.drawRect(plotRect);
        auto font = p.font();
        font.setPixelSize(20);
        p.setFont(font);
        p.setOpacity(1.0);
        p.setPen(Qt::white);
        auto text = "Drop here to add\n" + dropTrace->name() + "\nto waterfall plot";
        p.drawText(plotRect, Qt::AlignCenter, text);
    }
}

bool TraceWaterfall::supported(Trace *t)
{
    // TODO
    return true;
}

double TraceWaterfall::nearestTracePoint(Trace *t, QPoint pixel, double *distance)
{
    // TODO
    return 0;
}

QString TraceWaterfall::mouseText(QPoint pos)
{
    // TODO
    return "Test";
}

bool TraceWaterfall::xCoordinateVisible(double x)
{
    // TODO
    return true;
}

void TraceWaterfall::updateAxisTicks()
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

    auto createLogarithmicTicks = [](vector<double>& ticks, double start, double stop, int minDivisions) {
        // enforce usable log settings
        if(start <= 0) {
            start = 1.0;
        }
        if(stop <= start) {
            stop = start + 1.0;
        }
        ticks.clear();

        auto decades = log10(stop) - log10(start);
        double max_div_decade = minDivisions / decades;
        int zeros = floor(log10(max_div_decade));
        double decimals_shift = pow(10, zeros);
        max_div_decade /= decimals_shift;
        if(max_div_decade < 2) {
            max_div_decade = 2;
        } else if(max_div_decade < 5) {
            max_div_decade = 5;
        } else {
            max_div_decade = 10;
        }
        auto step = pow(10, floor(log10(start))+1) / (max_div_decade * decimals_shift);
        // round min up to next multiple of div_step
        auto div = ceil(start / step) * step;
        if(floor(log10(div)) != floor(log10(start))) {
            // first div is already at the next decade
            step *= 10;
        }
        do {
            ticks.push_back(div);
            if(ticks.size() > 1 && div != step && floor(log10(div)) != floor(log10(div - step))) {
                // reached a new decade with this switch
                step *= 10;
                div = step;
            } else {
                div += step;
            }
        } while(div <= stop);
    };

    if(XAxis.mode == XAxisMode::Manual) {
        if(XAxis.log) {
            createLogarithmicTicks(XAxis.ticks, XAxis.rangeMin, XAxis.rangeMax, 20);
        } else {
            createEvenlySpacedTicks(XAxis.ticks, XAxis.rangeMin, XAxis.rangeMax, XAxis.rangeDiv);
        }
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
                bool enabled = t.second;
                auto trace = t.first;
                if(enabled && trace->isVisible()) {
                    if(!trace->size()) {
                        // empty trace, do not use for automatic axis calculation
                        continue;
                    }
                    // this trace is currently displayed
                    double trace_min = trace->minX();
                    double trace_max = trace->maxX();
                    if(XAxis.type == XAxisType::Distance) {
                        trace_min = trace->timeToDistance(trace_min);
                        trace_max = trace->timeToDistance(trace_max);
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
            if(XAxis.log) {
                createLogarithmicTicks(XAxis.ticks, XAxis.rangeMin, XAxis.rangeMax, 20);
            } else {
                XAxis.rangeDiv = createAutomaticTicks(XAxis.ticks, min, max, 8);
            }
        }
    }
}

QString TraceWaterfall::AxisTypeToName(TraceWaterfall::XAxisType type)
{
    switch(type) {
    case XAxisType::Frequency: return "Frequency";
    case XAxisType::Time: return "Time";
    case XAxisType::Distance: return "Distance";
    case XAxisType::Power: return "Power";
    default: return "Unknown";
    }
}

QString TraceWaterfall::AxisModeToName(TraceWaterfall::XAxisMode mode)
{
    switch(mode) {
    case XAxisMode::Manual: return "Manual"; break;
    case XAxisMode::FitTraces: return "Fit Traces"; break;
    case XAxisMode::UseSpan: return "Use Span"; break;
    default: return "Unknown";
    }
}

TraceWaterfall::XAxisType TraceWaterfall::XAxisTypeFromName(QString name)
{
    for(unsigned int i=0;i<(int) XAxisType::Last;i++) {
        if(AxisTypeToName((XAxisType) i) == name) {
            return (XAxisType) i;
        }
    }
    // not found, use default
    return XAxisType::Frequency;
}

TraceWaterfall::XAxisMode TraceWaterfall::AxisModeFromName(QString name)
{
    for(unsigned int i=0;i<(int) XAxisMode::Last;i++) {
        if(AxisModeToName((XAxisMode) i) == name) {
            return (XAxisMode) i;
        }
    }
    // not found, use default
    return XAxisMode::UseSpan;
}

QString TraceWaterfall::AxisUnit(XAxisType type)
{
    switch(type) {
    case XAxisType::Frequency: return "Hz";
    case XAxisType::Time: return "s";
    case XAxisType::Distance: return "m";
    case XAxisType::Power: return "dBm";
    default: return "";
    }
}
