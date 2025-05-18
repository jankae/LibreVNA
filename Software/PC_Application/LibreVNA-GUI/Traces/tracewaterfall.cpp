#include "tracewaterfall.h"

#include "preferences.h"
#include "unit.h"
#include "Util/util.h"
#include "waterfallaxisdialog.h"
#include "appwindow.h"
#include "tracexyplot.h"

#include <QFileDialog>
#include <QPainter>

using namespace std;

TraceWaterfall::TraceWaterfall(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent),
      dir(Direction::TopToBottom),
      align(Alignment::PrimaryOnly),
      trace(nullptr),
      pixelsPerLine(1),
      keepDataBeyondPlotSize(false),
      maxDataSweeps(500)
{
    plotAreaTop = 0;
    plotAreaLeft = 0;
    plotAreaWidth = 0;
    plotAreaBottom = 0;

    xAxis.set(XAxis::Type::Frequency, false, true, 0, 6000000000, 10, false);
    yAxis.set(YAxis::Type::Magnitude, false, true, YAxis::getDefaultLimitMin(YAxis::Type::Magnitude), YAxis::getDefaultLimitMax(YAxis::Type::Magnitude), 10, false);
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
        if(trace) {
            disconnect(trace, &Trace::dataChanged, this, &TraceWaterfall::traceDataChanged);
        }
        trace = nullptr;
    }
}

void TraceWaterfall::replot()
{
    TracePlot::replot();
}

//void TraceWaterfall::move(const QPoint &vect)
//{
//    if(!xAxis.getLog()) {
//        // can only move axis in linear mode
//        // calculate amount of movement
//        double distance = xAxis.inverseTransform(vect.x(), 0, plotAreaWidth) - xAxis.getRangeMin();
//        xAxis.set(xAxis.getType(), false, false, xAxis.getRangeMin() - distance, xAxis.getRangeMax() - distance, xAxis.getRangeDiv());
//    }
//    replot();
//}

//void TraceWaterfall::zoom(const QPoint &center, double factor, bool horizontally, bool vertically)
//{
//    if(horizontally && !xAxis.getLog()) {
//        // can only zoom axis in linear mode
//        // calculate center point
//        double cp = xAxis.inverseTransform(center.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth);
//        double min = ((xAxis.getRangeMin() - cp) * factor) + cp;
//        double max = ((xAxis.getRangeMax() - cp) * factor) + cp;
//        xAxis.set(xAxis.getType(), false, false, min, max, xAxis.getRangeDiv() * factor);
//    }
//    replot();
//}

//void TraceWaterfall::setAuto(bool horizontally, bool vertically)
//{
//    if(horizontally) {
//        xAxis.set(xAxis.getType(), xAxis.getLog(), true, xAxis.getRangeMin(), xAxis.getRangeMax(), xAxis.getRangeDiv());
//    }
//    replot();
//}

void TraceWaterfall::fromJSON(nlohmann::json j)
{
    parseBaseJSON(j);
    resetWaterfall();
    pixelsPerLine = j.value("pixelsPerLine", pixelsPerLine);
    maxDataSweeps = j.value("maxLines", maxDataSweeps);
    keepDataBeyondPlotSize = j.value("keepDataBeyondPlot", keepDataBeyondPlotSize);
    if(QString::fromStdString(j.value("direction", "TopToBottom")) == "TopToBottom") {
        dir = Direction::TopToBottom;
    } else {
        dir = Direction::BottomToTop;
    }
    align = AlignmentFromString(QString::fromStdString(j.value("alignment", "")));
    if(align == Alignment::Last) {
        align = Alignment::PrimaryOnly;
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
    nlohmann::json j = getBaseJSON();
    j["pixelsPerLine"] = pixelsPerLine;
    j["direction"] = dir == Direction::TopToBottom ? "TopToBottom" : "BottomToTop";
    j["keepDataBeyondPlot"] = keepDataBeyondPlotSize;
    j["maxLines"] = maxDataSweeps;
    j["alignment"] = AlignmentToString(align).toStdString();
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
    YAxis::Type yDefault = YAxis::Type::Disabled;

    switch(t->outputType()) {
    case Trace::DataType::Frequency:
        xAxis.set(XAxis::Type::Frequency, false, true, 0, 1, 10, false);
        yDefault = YAxis::Type::Magnitude;
        break;
    case Trace::DataType::Power:
        xAxis.set(XAxis::Type::Power, false, true, 0, 1, 10, false);
        yDefault = YAxis::Type::Magnitude;
        break;
    case Trace::DataType::Time:
    case Trace::DataType::TimeZeroSpan:
    case Trace::DataType::Invalid:
        // unable to add
        return false;
    }
    if(!yAxis.isSupported(xAxis.getType(), getModel().getSource())) {
        yAxis.set(yDefault, false, true, 0, 1, 10, false);
    }
    traceRemovalPending = true;
    return true;
}

bool TraceWaterfall::domainMatch(Trace *t)
{
    switch(xAxis.getType()) {
    case XAxis::Type::Frequency:
        return t->outputType() == Trace::DataType::Frequency;
    case XAxis::Type::Distance:
    case XAxis::Type::Time:
        return t->outputType() == Trace::DataType::Time;
    case XAxis::Type::Power:
        return t->outputType() == Trace::DataType::Power;
    case XAxis::Type::TimeZeroSpan:
        return t->outputType() == Trace::DataType::TimeZeroSpan;
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
        auto filename = QFileDialog::getSaveFileName(nullptr, "Save plot image", "", "PNG image files (*.png)", nullptr, Preferences::QFileDialogOptions());
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
    for(auto t : orderedTraces()) {
        if(!supported(t)) {
            continue;
        }
        auto action = new QAction(t->name(), contextmenu);
        action->setCheckable(true);
        if(traces[t]) {
            action->setChecked(true);
        }
        connect(action, &QAction::toggled, [=](bool active) {
            enableTrace(t, active);
        });
        contextmenu->addAction(action);
    }

    finishContextMenu();
}

void TraceWaterfall::draw(QPainter &p)
{
    auto& pref = Preferences::getInstance();

    int xAxisSpace = pref.Graphs.fontSizeAxis * 3;
    constexpr int topMargin = 10;
    auto w = p.window();
    auto pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);

    auto leftMargin = TraceXYPlot::sideMargin(align == Alignment::PrimaryOnly || align == Alignment::BothAxes);
    auto rightMargin = TraceXYPlot::sideMargin(align == Alignment::SecondaryOnly || align == Alignment::BothAxes);
    auto plotRect = QRect(leftMargin, topMargin, w.width() - leftMargin - rightMargin, w.height()-topMargin-xAxisSpace);

    plotAreaTop = plotRect.y();
    plotAreaLeft = plotRect.x();
    plotAreaWidth = plotRect.width();
    plotAreaBottom = plotRect.y()+plotRect.height();

    // draw Y legend
    auto font = p.font();
    font.setPixelSize(pref.Graphs.fontSizeAxis);
    p.setFont(font);
    QRect legendRect;
    constexpr int legendMargin = 10;
    if(leftMargin < rightMargin) {
        legendRect = QRect(QPoint(plotRect.x()+plotRect.width()+legendMargin, plotAreaTop), QPoint(width() - legendMargin, plotAreaBottom));
    } else {
        legendRect = QRect(QPoint(legendMargin, plotAreaTop), QPoint(leftMargin - legendMargin, plotAreaBottom));
    }
    p.drawRect(legendRect);
    for(int i=plotAreaTop + 1;i<plotAreaBottom;i++) {
        auto color = Util::getIntensityGradeColor(Util::Scale<double>(i, plotAreaTop, plotAreaBottom, 1.0, 0.0));
        p.setPen(QColor(color));
        pen.setCosmetic(true);
        p.drawLine(legendRect.x()+1, i, legendRect.x()+legendRect.width()-1, i);
    }
    QString unit = "";
    if(pref.Graphs.showUnits) {
        unit = yAxis.Unit(getModel().getSource());
    }
    QString labelMin = Unit::ToString(yAxis.getRangeMin(), unit, yAxis.Prefixes(getModel().getSource()), 4);
    QString labelMax = Unit::ToString(yAxis.getRangeMax(), unit, yAxis.Prefixes(getModel().getSource()), 4);
    p.setPen(QPen(pref.Graphs.Color.axis, 1));
    p.save();
    p.translate(legendRect.x(), w.height());
    p.rotate(-90);
    p.drawText(QRect(xAxisSpace + 10, 0, plotAreaBottom - plotAreaTop - 20, legendRect.width()), Qt::AlignRight | Qt::AlignVCenter, labelMax);
    p.drawText(QRect(xAxisSpace + 10, 0, plotAreaBottom - plotAreaTop - 20, legendRect.width()), Qt::AlignLeft | Qt::AlignVCenter, labelMin);
    p.drawText(QRect(xAxisSpace + 10, 0, plotAreaBottom - plotAreaTop - 20, legendRect.width()), Qt::AlignHCenter | Qt::AlignVCenter, yAxis.TypeToName());
    p.restore();


    pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    p.drawRect(plotRect);

    // draw axis types
    p.drawText(QRect(0, w.height()-pref.Graphs.fontSizeAxis*1.5, w.width(), pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, xAxis.TypeToName());

    if(xAxis.getTicks().size() >= 1) {
        // draw X ticks
        int significantDigits;
        bool displayFullFreq;
        if(xAxis.getLog()) {
            significantDigits = 5;
            displayFullFreq = true;
        } else {
            // this only works for evenly distributed ticks:
            auto max = qMax(abs(xAxis.getTicks().front()), abs(xAxis.getTicks().back()));
            double step;
            if(xAxis.getTicks().size() >= 2) {
                step = abs(xAxis.getTicks()[0] - xAxis.getTicks()[1]);
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
            unit = xAxis.Unit();
        }
        QString commonPrefix = QString();
        if(!displayFullFreq) {
            auto fullFreq = Unit::ToString(xAxis.getTicks().front(), unit, prefixes, significantDigits);
            commonPrefix = fullFreq.at(fullFreq.size() - 1);
            auto front = fullFreq;
            front.truncate(fullFreq.size() - displayLastDigits - unit.length());
            auto back = fullFreq;
            back.remove(0, front.size());
            back.append("..");
            p.setPen(QPen(QColor("orange")));
            QRect bounding;
            p.drawText(QRect(2, plotAreaBottom + pref.Graphs.fontSizeAxis + 5, w.width(), pref.Graphs.fontSizeAxis*1.5), 0, front, &bounding);
            p.setPen(pref.Graphs.Color.axis);
            p.drawText(QRect(bounding.x() + bounding.width(), plotAreaBottom + pref.Graphs.fontSizeAxis + 5, w.width(), pref.Graphs.fontSizeAxis*1.5), 0, back);
        }

        int lastTickLabelEnd = 0;
        for(auto t : xAxis.getTicks()) {
            auto xCoord = xAxis.transform(t, plotAreaLeft, plotAreaLeft + plotAreaWidth);
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
                p.drawText(QRect(xCoord - pref.Graphs.fontSizeAxis*4, plotAreaBottom + 5, pref.Graphs.fontSizeAxis*8,
                                 pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, tickValue, &bounding);
                lastTickLabelEnd = bounding.x() + bounding.width();
            } else {
                // check if the same prefix was used as in the fullFreq string
                if(tickValue.at(tickValue.size() - 1) != commonPrefix) {
                    // prefix changed, we reached the next order of magnitude. Force same prefix as in fullFreq and add extra digit
                    tickValue = Unit::ToString(t, "", commonPrefix, significantDigits + 1);
                }

                tickValue.remove(0, tickValue.size() - displayLastDigits - unit.length());
                QRect bounding;
                p.drawText(QRect(xCoord - pref.Graphs.fontSizeAxis*4, plotAreaBottom + 5, pref.Graphs.fontSizeAxis*8, pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, tickValue, &bounding);
                lastTickLabelEnd = bounding.x() + bounding.width();
                p.setPen(QPen(QColor("orange")));
                p.drawText(QRect(0, plotAreaBottom + 5, bounding.x() - 1, pref.Graphs.fontSizeAxis*1.5), Qt::AlignRight, "..", &bounding);
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
                auto x = xAxis.sampleToCoordinate(sweep[s], trace);
                double x_start;
                double x_stop;
                if(x < xAxis.getRangeMin() || x > xAxis.getRangeMax()) {
                    // out of range, skip
                    continue;
                }
                if(s == 0) {
                    x_start = x;
                } else {
                    auto prev_x = xAxis.sampleToCoordinate(sweep[s-1], trace);
                    x_start = (prev_x + x) / 2.0;
                }
                x_start = xAxis.transform(x_start, plotAreaLeft, plotAreaLeft + plotAreaWidth);
                if(s == sweep.size() - 1) {
                    x_stop = x;
                } else {
                    auto next_x = xAxis.sampleToCoordinate(sweep[s+1], trace);
                    x_stop = (next_x + x) / 2.0;
                }
                x_stop = xAxis.transform(x_stop, plotAreaLeft, plotAreaLeft + plotAreaWidth);
                auto y = yAxis.sampleToCoordinate(sweep[s]);
                auto color = Util::getIntensityGradeColor(yAxis.transform(y, 0.0, 1.0));
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

    // show sweep indicator if activated
    if((xAxis.getType() == XAxis::Type::Frequency || xAxis.getType() == XAxis::Type::TimeZeroSpan || xAxis.getType() == XAxis::Type::Power)
            && !isnan(xSweep)) {
        if(xSweep >= xAxis.getRangeMin() && xSweep <= xAxis.getRangeMax()) {
            auto xpos = xAxis.transform(xSweep, plotAreaLeft, plotAreaLeft + plotAreaWidth);
            pen = QPen(pref.Graphs.Color.axis);
            pen.setCosmetic(true);
            p.setPen(pen);
            if(pref.Graphs.SweepIndicator.line) {
                p.drawLine(xpos, plotAreaTop, xpos, plotAreaBottom);
            }
            if(pref.Graphs.SweepIndicator.triangle) {
                for(int i=0;i<pref.Graphs.SweepIndicator.triangleSize;i++) {
                    p.drawLine(xpos - i,plotAreaBottom+i+1, xpos + i, plotAreaBottom+i+1);
                }
            }
        }
    }

    if(dropPending) {
        p.setOpacity(dropOpacity);
        p.setBrush(dropBackgroundColor);
        p.setPen(dropForegroundColor);
        // show drop area over whole plot
        p.drawRect(getDropRect());
        auto font = p.font();
        font.setPixelSize(20);
        p.setFont(font);
        p.setOpacity(1.0);
        p.setPen(dropSection == DropSection::OnPlot ? dropHighlightColor : dropForegroundColor);
        auto text = "Drop here to add\n" + dropTrace->name() + "\nto waterfall plot";
        p.drawText(getDropRect(), Qt::AlignCenter, text);
    }
}

bool TraceWaterfall::supported(Trace *t)
{
    if(!domainMatch(t)) {
        return false;
    }

    switch(yAxis.getType()) {
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
    if(QRect(plotAreaLeft, plotAreaTop, plotAreaWidth + 1, plotAreaBottom).contains(pos)) {
        double x = xAxis.inverseTransform(pos.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth);
        int significantDigits = floor(log10(abs(xAxis.getRangeMax()))) - floor(log10((abs(xAxis.getRangeMax() - xAxis.getRangeMin())) / 1000.0)) + 1;
        ret += Unit::ToString(x, xAxis.Unit(), "fpnum kMG", significantDigits) + "\n";
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
    if(xAxis.getAutorange()) {
        double min_x = trace->sample(0).x;
        double max_x = trace->sample(trace->size() - 1).x;
        if(min_x != xAxis.getRangeMin() || max_x != xAxis.getRangeMax()) {
            resetWaterfall();
            // adjust axis
            xAxis.set(xAxis.getType(), xAxis.getLog(), true, min_x, max_x, xAxis.getDivs(), xAxis.getAutoDivs());
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
    double min = yAxis.getRangeMin();
    double max = yAxis.getRangeMax();
    for(unsigned int i=begin;i<end;i++) {
        data.back()[i] = trace->sample(i);
        if(yAxis.getAutorange() && !YAxisUpdateRequired) {
            double val = yAxis.sampleToCoordinate(trace->sample(i));
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
    if(yAxis.getAutorange() && !YAxisUpdateRequired && (min != yAxis.getRangeMin() || max != yAxis.getRangeMax())) {
        // axis scaling needs update due to new trace data
        yAxis.set(yAxis.getType(), yAxis.getLog(), true, min, max, yAxis.getDivs(), yAxis.getAutoDivs());
    } else if(YAxisUpdateRequired) {
        updateYAxis();
    }
}

void TraceWaterfall::updateYAxis()
{
    if(yAxis.getAutorange()) {
        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::lowest();
        for(auto sweep : data) {
            for(unsigned int i=0;i<sweep.size();i++) {
                double val = yAxis.sampleToCoordinate(sweep[i]);
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
            yAxis.set(yAxis.getType(), yAxis.getLog(), true, min, max, yAxis.getDivs(), yAxis.getAutoDivs());
        }
    }
}

QString TraceWaterfall::AlignmentToString(Alignment a)
{
    switch(a) {
    case Alignment::PrimaryOnly: return "Primary Y axis only";
    case Alignment::SecondaryOnly: return "Secondary Y axis only";
    case Alignment::BothAxes: return "Both Y axes";
    case Alignment::Last:
    default: return "Invalid";
    }
}

TraceWaterfall::Alignment TraceWaterfall::AlignmentFromString(QString s)
{
    for(unsigned int i=0;i<(int) Alignment::Last;i++) {
        if(s == AlignmentToString((Alignment) i)) {
            return (Alignment) i;
        }
    }
    return Alignment::Last;
}
