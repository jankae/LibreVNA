#include "tracewaterfall.h"

#include "preferences.h"
#include "unit.h"
#include "Util/util.h"
#include "waterfallaxisdialog.h"
#include "appwindow.h"

#include <QFileDialog>
#include <QPainter>

using namespace std;

TraceWaterfall::TraceWaterfall(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent),
      dir(Direction::TopToBottom),
      trace(nullptr),
      pixelsPerLine(1),
      keepDataBeyondPlotSize(false),
      maxDataSweeps(500)
{
    XAxis.set(XAxis::Type::Frequency, false, true, 0, 6000000000, 500000000);
    YAxis.set(YAxis::Type::Magnitude, false, true, -1, 1, 1);
    initializeTraceInfo();
}

void TraceWaterfall::enableTrace(Trace *t, bool enabled)
{
    if(enabled) {
        // only one trace at a time is allowed, disable all others
        for(auto t : traces) {
            if(t.second) {
                TracePlot::enableTrace(t.first, false);
                disconnect(t.first, &Trace::dataChanged, this, &TraceWaterfall::traceDataChanged);
                break;
            }
        }
    }
    TracePlot::enableTrace(t, enabled);
    resetWaterfall();
    if(enabled) {
        trace = t;
        connect(t, &Trace::dataChanged, this, &TraceWaterfall::traceDataChanged);
    } else {
        trace = nullptr;
    }

}

void TraceWaterfall::replot()
{
    TracePlot::replot();
}

void TraceWaterfall::fromJSON(nlohmann::json j)
{
    resetWaterfall();
    pixelsPerLine = j.value("pixelsPerLine", pixelsPerLine);
    maxDataSweeps = j.value("maxLines", maxDataSweeps);
    keepDataBeyondPlotSize = j.value("keepDataBeyondPlot", keepDataBeyondPlotSize);
    if(QString::fromStdString(j.value("direction", "TopToBottom")) == "TopToBottom") {
        dir = Direction::TopToBottom;
    } else {
        dir = Direction::BottomToTop;
    }

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
    j["direction"] = dir == Direction::TopToBottom ? "TopToBottom" : "BottomToTop";
    j["keepDataBeyondPlot"] = keepDataBeyondPlotSize;
    j["maxLines"] = maxDataSweeps;
    nlohmann::json jtraces;
    for(auto t : traces) {
        if(t.second) {
            jtraces.push_back(t.first->toHash());
        }
    }
    j["traces"] = jtraces;
    return j;
}

void TraceWaterfall::axisSetupDialog()
{
    auto setup = new WaterfallAxisDialog(this);
    if(AppWindow::showGUI()) {
        setup->show();
    }
}

void TraceWaterfall::resetWaterfall()
{
    data.clear();
    updateYAxis();
}

bool TraceWaterfall::configureForTrace(Trace *t)
{
    switch(t->outputType()) {
    case Trace::DataType::Frequency:
        XAxis.set(XAxis::Type::Frequency, false, true, 0, 1, 0.1);
        YAxis.set(YAxis::Type::Magnitude, false, true, 0, 1, 1.0);
        break;
    case Trace::DataType::Time:
        XAxis.set(XAxis::Type::Time, false, true, 0, 1, 0.1);
        YAxis.set(YAxis::Type::ImpulseMag, false, true, 0, 1, 1.0);
        break;
    case Trace::DataType::Power:
        XAxis.set(XAxis::Type::Power, false, true, 0, 1, 0.1);
        YAxis.set(YAxis::Type::Magnitude, false, true, 0, 1, 1.0);
        break;
    case Trace::DataType::Invalid:
        // unable to add
        return false;
    }
    traceRemovalPending = true;
    return true;
}

bool TraceWaterfall::domainMatch(Trace *t)
{
    switch(XAxis.getType()) {
    case XAxis::Type::Frequency:
        return t->outputType() == Trace::DataType::Frequency;
    case XAxis::Type::Distance:
    case XAxis::Type::Time:
        return t->outputType() == Trace::DataType::Time;
    case XAxis::Type::Power:
        return t->outputType() == Trace::DataType::Power;
    case XAxis::Type::Last:
        return false;
    }
    return false;
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

    constexpr int yAxisLegendSpace = 25;
    constexpr int yAxisDisabledSpace = 10;
    constexpr int xAxisSpace = 30;
    auto w = p.window();
    auto pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    plotAreaLeft = yAxisDisabledSpace;
    plotAreaWidth = w.width() - 3 * yAxisDisabledSpace - yAxisLegendSpace;
    plotAreaTop = 10;
    plotAreaBottom = w.height() - xAxisSpace;

    // draw Y legend
    auto plotRect = QRect(w.width() - yAxisDisabledSpace - yAxisLegendSpace, plotAreaTop, yAxisLegendSpace, plotAreaBottom-plotAreaTop);
    p.drawRect(plotRect);
    for(int i=plotAreaTop + 1;i<plotAreaBottom;i++) {
        auto color = getColor(Util::Scale<double>(i, plotAreaTop, plotAreaBottom, 1.0, 0.0));
        p.setPen(QColor(color));
        pen.setCosmetic(true);
        p.drawLine(w.width() - yAxisDisabledSpace - yAxisLegendSpace + 1, i, w.width() - yAxisDisabledSpace - 1, i);
    }
    QString unit = "";
    if(pref.Graphs.showUnits) {
        unit = YAxis.Unit();
    }
    QString labelMin = Unit::ToString(YAxis.getRangeMin(), unit, YAxis.Prefixes(), 4);
    QString labelMax = Unit::ToString(YAxis.getRangeMax(), unit, YAxis.Prefixes(), 4);
    p.setPen(QPen(pref.Graphs.Color.axis, 1));
    p.save();
    p.translate(w.width() - yAxisDisabledSpace - yAxisLegendSpace, w.height());
    p.rotate(-90);
    p.drawText(QRect(xAxisSpace + 10, 0, plotAreaBottom - plotAreaTop - 20, yAxisLegendSpace), Qt::AlignRight | Qt::AlignVCenter, labelMax);
    p.drawText(QRect(xAxisSpace + 10, 0, plotAreaBottom - plotAreaTop - 20, yAxisLegendSpace), Qt::AlignLeft | Qt::AlignVCenter, labelMin);
    p.restore();


    pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    plotRect = QRect(plotAreaLeft, plotAreaTop, plotAreaWidth + 1, plotAreaBottom-plotAreaTop);
    p.drawRect(plotRect);

    // draw axis types
    auto font = p.font();
    font.setPixelSize(AxisLabelSize);
    p.setFont(font);
    p.drawText(QRect(0, w.height()-AxisLabelSize*1.5, w.width(), AxisLabelSize*1.5), Qt::AlignHCenter, XAxis.TypeToName());

    if(XAxis.getTicks().size() >= 1) {
        // draw X ticks
        int significantDigits;
        bool displayFullFreq;
        if(XAxis.getLog()) {
            significantDigits = 5;
            displayFullFreq = true;
        } else {
            // this only works for evenly distributed ticks:
            auto max = qMax(abs(XAxis.getTicks().front()), abs(XAxis.getTicks().back()));
            double step;
            if(XAxis.getTicks().size() >= 2) {
                step = abs(XAxis.getTicks()[0] - XAxis.getTicks()[1]);
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
            unit = XAxis.Unit();
        }
        QString commonPrefix = QString();
        if(!displayFullFreq) {
            auto fullFreq = Unit::ToString(XAxis.getTicks().front(), unit, prefixes, significantDigits);
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
        for(auto t : XAxis.getTicks()) {
            auto xCoord = XAxis.transform(t, plotAreaLeft, plotAreaLeft + plotAreaWidth);
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

    p.setClipRect(QRect(plotRect.x()+1, plotRect.y()+1, plotRect.width()-1, plotRect.height()-1));
    if(data.size()) {
        // plot waterfall data
        int ytop, ybottom;
        bool lastLine = false;
        if(dir == Direction::TopToBottom) {
            ytop = plotAreaTop;
            ybottom = ytop + pixelsPerLine - 1;
        } else {
            ybottom = plotAreaBottom - 1;
            ytop = ybottom - pixelsPerLine + 1;
        }
        int i;
        for(i=data.size() - 1;i>=0;i--) {
            auto sweep = data[i];
            for(unsigned int s=0;s<sweep.size();s++) {
                auto x = XAxis.sampleToCoordinate(sweep[s], trace);
                double x_start;
                double x_stop;
                if(x < XAxis.getRangeMin() || x > XAxis.getRangeMax()) {
                    // out of range, skip
                    continue;
                }
                if(s == 0) {
                    x_start = x;
                } else {
                    auto prev_x = XAxis.sampleToCoordinate(sweep[s-1], trace);
                    x_start = (prev_x + x) / 2.0;
                }
                x_start = XAxis.transform(x_start, plotAreaLeft, plotAreaLeft + plotAreaWidth);
                if(s == sweep.size() - 1) {
                    x_stop = x;
                } else {
                    auto next_x = XAxis.sampleToCoordinate(sweep[s+1], trace);
                    x_stop = (next_x + x) / 2.0;
                }
                x_stop = XAxis.transform(x_stop, plotAreaLeft, plotAreaLeft + plotAreaWidth);
                auto y = YAxis.sampleToCoordinate(sweep[s]);
                auto color = getColor(YAxis.transform(y, 0.0, 1.0));
                auto rect = QRect(round(x_start), ytop, round(x_stop - x_start) + 1, ybottom - ytop + 1);
                p.fillRect(rect, QBrush(color));
            }
            if(lastLine) {
                break;
            }
            // update ycoords for next line
            if(dir == Direction::TopToBottom) {
                ytop = ybottom + 1;
                ybottom = ytop + pixelsPerLine - 1;
                if(ybottom >= plotAreaBottom) {
                    ybottom = plotAreaBottom;
                    lastLine = true;
                }
            } else {
                ybottom = ytop - 1;
                ytop = ybottom - pixelsPerLine + 1;
                if(ytop <= plotAreaTop) {
                    ytop = plotAreaTop;
                    lastLine = true;
                }
            }
        }
        if(!keepDataBeyondPlotSize && i >= 0) {
            // not all data could be plotted, drop
            data.erase(data.begin(), data.begin() + i);
            updateYAxis();
        }
    }
    p.setClipping(false);

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
    if(!domainMatch(t)) {
        return false;
    }

    switch(YAxis.getType()) {
    case YAxis::Type::Disabled:
        return false;
    case YAxis::Type::VSWR:
    case YAxis::Type::SeriesR:
    case YAxis::Type::Reactance:
    case YAxis::Type::Capacitance:
    case YAxis::Type::Inductance:
    case YAxis::Type::QualityFactor:
        if(!t->isReflection()) {
            return false;
        }
        break;
    case YAxis::Type::GroupDelay:
        if(t->isReflection()) {
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}

double TraceWaterfall::nearestTracePoint(Trace *t, QPoint pixel, double *distance)
{
    // this function is used for the movement of markers.
    // No markers on waterfall plot, nothing to do
    Q_UNUSED(t)
    Q_UNUSED(pixel)
    Q_UNUSED(distance)
    return 0;
}

QString TraceWaterfall::mouseText(QPoint pos)
{
    QString ret;
    if(QRect(plotAreaLeft, 0, plotAreaWidth + 1, plotAreaBottom).contains(pos)) {
        double x = XAxis.inverseTransform(pos.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth);
        int significantDigits = floor(log10(abs(XAxis.getRangeMax()))) - floor(log10((abs(XAxis.getRangeMax() - XAxis.getRangeMin())) / 1000.0)) + 1;
        ret += Unit::ToString(x, XAxis.Unit(), "fpnum kMG", significantDigits) + "\n";
    }
    return ret;
}

bool TraceWaterfall::markerVisible(double x)
{
    // no markers on waterfall
    Q_UNUSED(x)
    return false;
}

void TraceWaterfall::traceDataChanged(unsigned int begin, unsigned int end)
{
    if(XAxis.getAutorange()) {
        double min_x = trace->sample(0).x;
        double max_x = trace->sample(trace->size() - 1).x;
        if(min_x != XAxis.getRangeMin() || max_x != XAxis.getRangeMax()) {
            resetWaterfall();
            // adjust axis
            XAxis.set(XAxis.getType(), XAxis.getLog(), true, min_x, max_x, 0);
        }
    }
    bool YAxisUpdateRequired = false;
    if (begin == 0 || data.size() == 0) {
        if(data.size() == 1) {
            YAxisUpdateRequired = true;
        }
        // start new row
        data.push_back(std::vector<Trace::Data>());
        while (data.size() > maxDataSweeps) {
            data.pop_front();
            // min/max might have changed due to removed data
            YAxisUpdateRequired = true;
        }
    }
    // grab trace data
    data.back().resize(trace->size());
    double min = YAxis.getRangeMin();
    double max = YAxis.getRangeMax();
    for(unsigned int i=begin;i<end;i++) {
        data.back()[i] = trace->sample(i);
        if(YAxis.getAutorange() && !YAxisUpdateRequired) {
            double val = YAxis.sampleToCoordinate(trace->sample(i));
            if(val < min) {
                min = val;
            }
            if(val > max) {
                max = val;
            }
        }
    }
    if(YAxis.getAutorange() && !YAxisUpdateRequired && (min != YAxis.getRangeMin() || max != YAxis.getRangeMax())) {
        // axis scaling needs update due to new trace data
        YAxis.set(YAxis.getType(), YAxis.getLog(), true, min, max, 0);
    } else if(YAxisUpdateRequired) {
        updateYAxis();
    }
}

void TraceWaterfall::updateYAxis()
{
    if(YAxis.getAutorange()) {
        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::lowest();
        for(auto sweep : data) {
            for(unsigned int i=0;i<sweep.size();i++) {
                double val = YAxis.sampleToCoordinate(sweep[i]);
                if(isnan(val) || isinf(val)) {
                    continue;
                }
                if(val < min) {
                    min = val;
                }
                if(val > max) {
                    max = val;
                }
            }
        }
        if(max > min) {
            YAxis.set(YAxis.getType(), YAxis.getLog(), true, min, max, 0);
        }
    }
}

QColor TraceWaterfall::getColor(double scale)
{
    if(scale < 0.0) {
        return Qt::black;
    } else if(scale > 1.0) {
        return Qt::white;
    } else if(scale >= 0.0 && scale <= 1.0) {
        return QColor::fromHsv(Util::Scale<double>(scale, 0.0, 1.0, 240, 0), 255, 255);
    } else {
        return Qt::black;
    }
}
