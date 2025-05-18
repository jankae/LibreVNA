#include "tracexyplot.h"

#include "trace.h"
#include "CustomWidgets/informationbox.h"
#include "Marker/marker.h"
#include "xyplotaxisdialog.h"
#include "Util/util.h"
#include "unit.h"
#include "preferences.h"
#include "appwindow.h"
#include "ui_XYPlotConstantLineEditDialog.h"

#include <QGridLayout>
#include <cmath>
#include <QFrame>
#include <QPainter>
#include <QDebug>
#include <QFileDialog>

using namespace std;

TraceXYPlot::TraceXYPlot(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
{
    xAxisMode = XAxisMode::UseSpan;

    yAxis[1].setTickMaster(yAxis[0]);

    // Setup default axis
    setYAxis(0, YAxis::Type::Magnitude, false, false, YAxis::getDefaultLimitMin(YAxis::Type::Magnitude), YAxis::getDefaultLimitMax(YAxis::Type::Magnitude), 14, true);
    setYAxis(1, YAxis::Type::Phase, false, false, YAxis::getDefaultLimitMin(YAxis::Type::Phase), YAxis::getDefaultLimitMax(YAxis::Type::Phase), 12, true);
    // enable autoscaling and set for full span (no information about actual span available yet)
    updateSpan(0, 6000000000);
    setXAxis(XAxis::Type::Frequency, XAxisMode::UseSpan, false, 0, 6000000000, 10, true);
    initializeTraceInfo();
}
TraceXYPlot::~TraceXYPlot()
{
    for(auto l : constantLines) {
        delete l;
    }
}

void TraceXYPlot::setYAxis(int axis, YAxis::Type type, bool log, bool autorange, double min, double max, unsigned int divs, bool autoDivs)
{
    if(yAxis[axis].getType() != type) {
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
    }
    yAxis[axis].set(type, log, autorange, min, max, divs, autoDivs);
    traceRemovalPending = true;
    updateContextMenu();
    replot();
}

void TraceXYPlot::setXAxis(XAxis::Type type, XAxisMode mode, bool log, double min, double max, unsigned int divs, bool autoDivs)
{
    bool autorange = false;
    if(mode == XAxisMode::FitTraces || mode == XAxisMode::UseSpan) {
        autorange = true;
    }
    xAxis.set(type, log, autorange, min, max, divs, autoDivs);
    xAxisMode = mode;
    traceRemovalPending = true;
    updateContextMenu();
    replot();
}

void TraceXYPlot::enableTrace(Trace *t, bool enabled)
{
    for(int axis = 0;axis < 2;axis++) {
        enableTraceAxis(t, axis, enabled && supported(t, yAxis[axis].getType()));
    }
}

void TraceXYPlot::updateSpan(double min, double max)
{
    TracePlot::updateSpan(min, max);
    updateAxisTicks();
}

void TraceXYPlot::replot()
{
    if(xAxisMode != XAxisMode::Manual || yAxis[0].getAutorange() || yAxis[1].getAutorange()) {
        updateAxisTicks();
    }
    TracePlot::replot();
}

void TraceXYPlot::move(const QPoint &vect)
{
    if(!xAxis.getLog()) {
        // can only move axis in linear mode
        // calculate amount of movement
        double distance = xAxis.inverseTransform(vect.x(), 0, plotAreaWidth) - xAxis.getRangeMin();
        xAxis.set(xAxis.getType(), false, false, xAxis.getRangeMin() - distance, xAxis.getRangeMax() - distance, xAxis.getDivs(), xAxis.getAutoDivs());
        xAxisMode = XAxisMode::Manual;
    }
    for(int i=0;i<2;i++) {
        if(!yAxis[i].getLog()) {
            // can only move axis in linear mode
            // calculate amount of movement
            double distance = yAxis[i].inverseTransform(vect.y(), 0, plotAreaTop - plotAreaBottom) - yAxis[i].getRangeMin();
            yAxis[i].set(yAxis[i].getType(), false, false, yAxis[i].getRangeMin() - distance, yAxis[i].getRangeMax() - distance, yAxis[i].getDivs(), yAxis[i].getAutoDivs());
        }
    }
    replot();
}

void TraceXYPlot::zoom(const QPoint &center, double factor, bool horizontally, bool vertically)
{
    if(horizontally && !xAxis.getLog()) {
        // can only zoom axis in linear mode
        // calculate center point
        double cp = xAxis.inverseTransform(center.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth);
        double min = ((xAxis.getRangeMin() - cp) * factor) + cp;
        double max = ((xAxis.getRangeMax() - cp) * factor) + cp;
        xAxis.set(xAxis.getType(), false, false, min, max, xAxis.getDivs(), xAxis.getAutoDivs());
        xAxisMode = XAxisMode::Manual;
    }
    for(int i=0;i<2;i++) {
        if(vertically && yAxis[i].getType() != YAxis::Type::Disabled && !yAxis[i].getLog()) {
            // can only move axis in linear mode
            // calculate center point
            double cp = yAxis[i].inverseTransform(center.y(), plotAreaBottom, plotAreaTop);
            double min = ((yAxis[i].getRangeMin() - cp) * factor) + cp;
            double max = ((yAxis[i].getRangeMax() - cp) * factor) + cp;
            yAxis[i].set(yAxis[i].getType(), false, false, min, max, yAxis[i].getDivs(), yAxis[i].getAutoDivs());
        }
    }
    replot();
}

void TraceXYPlot::setAuto(bool horizontally, bool vertically)
{
    if(horizontally) {
        xAxisMode = XAxisMode::FitTraces;
        xAxis.set(xAxis.getType(), xAxis.getLog(), true, xAxis.getRangeMin(), xAxis.getRangeMax(), xAxis.getDivs(), xAxis.getAutoDivs());
    }
    for(int i=0;i<2;i++) {
        if(vertically && yAxis[i].getType() != YAxis::Type::Disabled) {
            yAxis[i].set(yAxis[i].getType(), yAxis[i].getLog(), true, yAxis[i].getRangeMin(), yAxis[i].getRangeMax(), yAxis[i].getDivs(), yAxis[i].getAutoDivs());
        }
    }
    replot();
}

nlohmann::json TraceXYPlot::toJSON()
{
    nlohmann::json j = getBaseJSON();
    nlohmann::json jX;
    jX["type"] = xAxis.TypeToName().toStdString();
    jX["mode"] = AxisModeToName(xAxisMode).toStdString();
    jX["log"] = xAxis.getLog();
    jX["min"] = xAxis.getRangeMin();
    jX["max"] = xAxis.getRangeMax();
    jX["divs"] = xAxis.getDivs();
    jX["autoDivs"] = xAxis.getAutoDivs();
    j["XAxis"] = jX;
    for(unsigned int i=0;i<2;i++) {
        nlohmann::json jY;
        jY["type"] = yAxis[i].TypeToName().toStdString();
        jY["log"] = yAxis[i].getLog();
        jY["autorange"] = yAxis[i].getAutorange();
        jY["min"] = yAxis[i].getRangeMin();
        jY["max"] = yAxis[i].getRangeMax();
        jY["divs"] = yAxis[i].getDivs();
        jY["autoDivs"] = yAxis[i].getAutoDivs();
        nlohmann::json jtraces;
        for(auto t : tracesAxis[i]) {
            jtraces.push_back(t->toHash());
        }
        jY["traces"] = jtraces;

        if(i==0) {
            j["YPrimary"] = jY;
        } else {
            j["YSecondary"] = jY;
        }
    }
    nlohmann::json jlines;
    for(auto l : constantLines) {
        jlines.push_back(l->toJSON());
    }
    j["limitLines"] = jlines;
    return j;
}

void TraceXYPlot::fromJSON(nlohmann::json j)
{
    parseBaseJSON(j);
    auto jX = j["XAxis"];
    // old format used enum value for type and mode, new format uses string encoding (more robust when additional enum values are added).
    // Check which format is used and parse accordingly
    XAxis::Type xtype;
    if(jX["type"].type() == nlohmann::json::value_t::string) {
        xtype = XAxis::TypeFromName(QString::fromStdString(jX["type"]));
    } else {
        xtype = jX.value("type", XAxis::Type::Frequency);
    }
    XAxisMode xmode;
    if(jX["mode"].type() == nlohmann::json::value_t::string) {
        xmode = AxisModeFromName(QString::fromStdString(jX["mode"]));
    } else {
        xmode = jX.value("mode", XAxisMode::UseSpan);
    }
//    auto xlog = jX.value("log", false);
    auto xmin = jX.value("min", 0.0);
    auto xmax = jX.value("max", 6000000000.0);
    auto xdivs = jX.value("divs", 10);
    // older formats specified the spacing instead of the number of divisions
    if(jX.contains("div")) {
        xdivs = (xmax - xmin) / jX.value("div", 600000000.0);
    }
    auto xautodivs = jX.value("autoDivs", false);
    auto xlog = jX.value("log", false);
    setXAxis(xtype, xmode, xlog, xmin, xmax, xdivs, xautodivs);
    nlohmann::json jY[2] = {j["YPrimary"], j["YSecondary"]};
    for(unsigned int i=0;i<2;i++) {
        YAxis::Type ytype;
        if(jY[i]["type"].type() == nlohmann::json::value_t::string) {
            ytype = YAxis::TypeFromName(QString::fromStdString(jY[i]["type"]));
        } else {
            ytype = jY[i].value("type", YAxis::Type::Disabled);
        }
        auto yauto = jY[i].value("autorange", true);
        auto ylog = jY[i].value("log", false);
        auto ymin = jY[i].value("min", -120.0);
        auto ymax = jY[i].value("max", 20.0);
        auto ydivs = jY[i].value("divs", 10);
        // older formats specified the spacing instead of the number of divisions
        if(jY[i].contains("div")) {
            ydivs = (ymax - ymin) / jY[i].value("div", 10);
        }
        auto yautodivs = jY[i].value("autoDivs", false);
        setYAxis(i, ytype, ylog, yauto, ymin, ymax, ydivs, yautodivs);
        for(unsigned int hash : jY[i]["traces"]) {
            // attempt to find the traces with this hash
            bool found = false;
            for(auto t : model.getTraces()) {
                if(t->toHash() == hash) {
                    enableTraceAxis(t, i, true);
                    found = true;
                    break;
                }
            }
            if(!found) {
                qWarning() << "Unable to find trace with hash" << hash;
            }
        }
    }
    if(j.contains("limitLines")) {
        for(auto jline : j["limitLines"]) {
            auto line = new XYPlotConstantLine;
            line->fromJSON(jline);
            constantLines.push_back(line);
        }
    }
}

bool TraceXYPlot::isTDRtype(YAxis::Type type)
{
    switch(type) {
    case YAxis::Type::ImpulseReal:
    case YAxis::Type::ImpulseMag:
    case YAxis::Type::Step:
    case YAxis::Type::Impedance:
        return true;
    default:
        return false;
    }
}

int TraceXYPlot::sideMargin(bool YAxisEnabled)
{
    if(YAxisEnabled) {
        return Preferences::getInstance().Graphs.fontSizeAxis * 5.5;
    } else {
        return yAxisDisabledSpace;
    }
}

void TraceXYPlot::axisSetupDialog()
{
    auto setup = new XYplotAxisDialog(this);
    setup->setAttribute(Qt::WA_DeleteOnClose);
    connect(setup, &QDialog::finished, this, &TraceXYPlot::updateAxisTicks);
    if(AppWindow::showGUI()) {
        setup->show();
    }
}

bool TraceXYPlot::configureForTrace(Trace *t)
{
    YAxis::Type yLeftDefault = YAxis::Type::Disabled;
    YAxis::Type yRightDefault = YAxis::Type::Disabled;

    switch(t->outputType()) {
    case Trace::DataType::Frequency:
        setXAxis(XAxis::Type::Frequency, XAxisMode::FitTraces, false, 0, 1, 10, false);
        yLeftDefault = YAxis::Type::Magnitude;
        yRightDefault = YAxis::Type::Phase;
        break;
    case Trace::DataType::Time:
        setXAxis(XAxis::Type::Time, XAxisMode::FitTraces, false, 0, 1, 10, false);
        yLeftDefault = YAxis::Type::ImpulseMag;
        break;
    case Trace::DataType::Power:
        setXAxis(XAxis::Type::Power, XAxisMode::FitTraces, false, 0, 1, 10, false);
        yLeftDefault = YAxis::Type::Magnitude;
        yRightDefault = YAxis::Type::Phase;
        break;
    case Trace::DataType::TimeZeroSpan:
        setXAxis(XAxis::Type::TimeZeroSpan, XAxisMode::FitTraces, false, 0, 1, 10, false);
        yLeftDefault = YAxis::Type::Magnitude;
        yRightDefault = YAxis::Type::Phase;
        break;
    case Trace::DataType::Invalid:
        // unable to add
        return false;
    }
    if(!yAxis[0].isSupported(xAxis.getType(), getModel().getSource())) {
        setYAxis(0, yLeftDefault, false, false, YAxis::getDefaultLimitMin(yLeftDefault), YAxis::getDefaultLimitMax(yLeftDefault), 10, true);
    }
    if(!yAxis[1].isSupported(xAxis.getType(), getModel().getSource())) {
        setYAxis(1, yRightDefault, false, false, YAxis::getDefaultLimitMin(yRightDefault), YAxis::getDefaultLimitMax(yRightDefault), 10, true);
    }
    traceRemovalPending = true;
    return true;
}

void TraceXYPlot::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Axis setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TraceXYPlot::axisSetupDialog);
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

    auto createMarker = contextmenu->addAction("Add marker here");
    bool activeTraces = false;
    for(auto t : traces) {
        if(t.second) {
            activeTraces = true;
            break;
        }
    }
    if(!activeTraces) {
        createMarker->setEnabled(false);
    }
    connect(createMarker, &QAction::triggered, [=](){
        createMarkerAtPosition(contextmenuClickpoint);
    });

    for(int axis = 0;axis < 2;axis++) {
        if(yAxis[axis].getType() == YAxis::Type::Disabled) {
            continue;
        }
        if(axis == 0) {
            contextmenu->addSection("Primary Traces");
        } else {
            contextmenu->addSection("Secondary Traces");
        }
        for(auto t : orderedTraces()) {
            // Skip traces that are not applicable for the selected axis type
            if(!supported(t, yAxis[axis].getType())) {
                continue;
            }

            auto action = new QAction(t->name(), contextmenu);
            action->setCheckable(true);
            if(tracesAxis[axis].find(t) != tracesAxis[axis].end()) {
                action->setChecked(true);
            }
            connect(action, &QAction::toggled, [=](bool active) {
                enableTraceAxis(t, axis, active);
            });
            contextmenu->addAction(action);
        }
    }

    finishContextMenu();
}

bool TraceXYPlot::positionWithinGraphArea(const QPoint &p)
{
    return p.x() >= plotAreaLeft && p.x() <= plotAreaLeft + plotAreaWidth
            && p.y() >= plotAreaTop && p.y() <= plotAreaBottom;
}

bool TraceXYPlot::dropSupported(Trace *t)
{
    if(domainMatch(t) && !supported(t)) {
        // correct domain configured but Y axis do not match, prevent drop
        return false;
    }
    // either directly compatible or domain change required
    return true;
}

bool TraceXYPlot::supported(Trace *t)
{
    // potentially possible to add every kind of trace (depends on axis)
    if(supported(t, yAxis[0].getType()) || supported(t, yAxis[1].getType())) {
        return true;
    } else {
        // no axis
        return false;
    }
}

void TraceXYPlot::draw(QPainter &p)
{
    auto& pref = Preferences::getInstance();

    limitPassing = true;

    auto w = p.window();
    auto pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    auto yAxisSpace = pref.Graphs.fontSizeAxis * 5.5;
    auto xAxisSpace = pref.Graphs.fontSizeAxis * 3;
    plotAreaLeft = yAxis[0].getType() == YAxis::Type::Disabled ? yAxisDisabledSpace : yAxisSpace;
    plotAreaWidth = w.width();
    plotAreaTop = 10;
    plotAreaBottom = w.height() - xAxisSpace;
    if(yAxis[0].getType() != YAxis::Type::Disabled) {
        plotAreaWidth -= yAxisSpace;
    } else {
        plotAreaWidth -= yAxisDisabledSpace;
    }
    if(yAxis[1].getType() != YAxis::Type::Disabled) {
        plotAreaWidth -= yAxisSpace;
    } else {
        plotAreaWidth -= yAxisDisabledSpace;
    }

    auto plotRect = QRect(plotAreaLeft, plotAreaTop, plotAreaWidth + 1, plotAreaBottom-plotAreaTop);
    p.drawRect(plotRect);

    // draw axis types
    auto font = p.font();
    font.setPixelSize(pref.Graphs.fontSizeAxis);
    p.setFont(font);
    p.drawText(QRect(0, w.height()-pref.Graphs.fontSizeAxis*1.5, w.width(), pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, xAxis.TypeToName());
    for(int i=0;i<2;i++) {
        if(yAxis[i].getType() == YAxis::Type::Disabled) {
            continue;
        }
        QString labelY = yAxis[i].TypeToName();
        p.setPen(QPen(pref.Graphs.Color.axis, 1));
        auto xStart = i == 0 ? 0 : w.width() - pref.Graphs.fontSizeAxis * 1.5;
        p.save();
        p.translate(xStart, w.height()-xAxisSpace);
        p.rotate(-90);
        p.drawText(QRect(0, 0, w.height()-xAxisSpace, pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, labelY);
        p.restore();
        // draw ticks
        if(yAxis[i].getType() != YAxis::Type::Disabled && yAxis[i].getTicks().size() > 0) {
            // this only works for evenly distributed ticks:
            auto max = qMax(abs(yAxis[i].getTicks().front()), abs(yAxis[i].getTicks().back()));
            double step;
            if(yAxis[i].getTicks().size() >= 2) {
                step = abs(yAxis[i].getTicks()[0] - yAxis[i].getTicks()[1]);
            } else {
                // only one tick, set arbitrary number of digits
                step = max / 1000;
            }
            int significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
            if(yAxis[i].getLog() && yAxis[i].getRangeMax()/yAxis[i].getRangeMin() >= 100) {
                significantDigits = floor(log10(max)) + 1;
            }

            int lastTickLabelEnd = std::numeric_limits<int>::max();
            for(unsigned int j = 0; j < yAxis[i].getTicks().size(); j++) {
                auto yCoord = yAxis[i].transform(yAxis[i].getTicks()[j], w.height() - xAxisSpace, plotAreaTop);
                p.setPen(QPen(pref.Graphs.Color.axis, 1));
                // draw tickmark on axis
                auto tickStart = i == 0 ? plotAreaLeft : plotAreaLeft + plotAreaWidth;
                auto tickLen = i == 0 ? -2 : 2;
                p.drawLine(tickStart, yCoord, tickStart + tickLen, yCoord);
                if(yCoord + pref.Graphs.fontSizeAxis >= lastTickLabelEnd) {
                    // would overlap previous tick label, skip
                } else {
                    QString unit = "";
                    QString prefix = " ";
                    if(pref.Graphs.showUnits) {
                        unit = yAxis[i].Unit(getModel().getSource());
                        prefix = yAxis[i].Prefixes(getModel().getSource());
                    }
                    auto tickValue = Unit::ToString(yAxis[i].getTicks()[j], unit, prefix, significantDigits);
                    QRect bounding;
                    if(i == 0) {
                        p.drawText(QRect(0, yCoord - pref.Graphs.fontSizeAxis/2 - 2, tickStart + 2 * tickLen, pref.Graphs.fontSizeAxis*1.5), Qt::AlignRight, tickValue, &bounding);
                    } else {
                        p.drawText(QRect(tickStart + 2 * tickLen + 2, yCoord - pref.Graphs.fontSizeAxis/2 - 2, yAxisSpace, pref.Graphs.fontSizeAxis*1.5), Qt::AlignLeft, tickValue, &bounding);
                    }
                    lastTickLabelEnd = bounding.y();
                }

                // tick lines
                if(yCoord == plotAreaTop || yCoord == w.height() - xAxisSpace) {
                    // skip tick lines right on the plot borders
                    continue;
                }
                if(i == 0) {
                    // only draw tick lines for primary axis
                    if (pref.Graphs.Color.Ticks.Background.enabled) {
                        if (j%2)
                        {
                            int yCoordTop = yCoord;
                            int yCoordBot = yAxis[i].transform(yAxis[i].getTicks()[j-1], w.height() - xAxisSpace, plotAreaTop);
                            if(yCoordTop > yCoordBot) {
                                auto buf = yCoordBot;
                                yCoordBot = yCoordTop;
                                yCoordTop = buf;
                            }
                            p.setBrush(pref.Graphs.Color.Ticks.Background.background);
                            p.setPen(pref.Graphs.Color.Ticks.Background.background);
                            auto rect = QRect(plotAreaLeft+1, yCoordTop+1, plotAreaWidth-2, yCoordBot-yCoordTop-2);
                            p.drawRect(rect);
                        }
                    }
                    p.setPen(QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine));
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
            pen = QPen(t->color(), pref.Graphs.lineWidth);
            pen.setCosmetic(true);
            if(i == 1) {
                pen.setStyle(Qt::DotLine);
            } else {
                pen.setStyle(Qt::SolidLine);
            }
            p.setPen(pen);
            auto nPoints = t->size();
            for(unsigned int j=1;j<nPoints;j++) {
                auto last = traceToCoordinate(t, j-1, yAxis[i]);
                auto now = traceToCoordinate(t, j, yAxis[i]);

                // checking limits
                for(auto limit : constantLines) {
                    if(i == 0 && limit->getAxis() != XYPlotConstantLine::Axis::Primary) {
                        continue;
                    }
                    if(i == 1 && limit->getAxis() != XYPlotConstantLine::Axis::Secondary) {
                        continue;
                    }
                    if(!limit->pass(now)) {
                        limitPassing = false;
                    }
                }

                if(isnan(last.y()) || isnan(now.y()) || isinf(last.y()) || isinf(now.y())) {
                    continue;
                }

                if((xAxis.getType() == XAxis::Type::Frequency || xAxis.getType() == XAxis::Type::TimeZeroSpan || xAxis.getType() == XAxis::Type::Power)
                        && pref.Graphs.SweepIndicator.hide && !isnan(xSweep) && t->getSource() == Trace::Source::Live && t->isVisible() && !t->isPaused()) {
                    // check if this part of the trace is visible
                    double range = xAxis.getRangeMax() - xAxis.getRangeMin();
                    double afterSweep = now.x() - xSweep;
                    if(afterSweep > 0 && afterSweep * 100 / range <= pref.Graphs.SweepIndicator.hidePercent) {
                        // do not display this part of the trace
                        continue;
                    }
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
                    if(!m->isVisible()) {
                        continue;
                    }
                    auto point = markerToPixel(m);
                    if(point.isNull()) {
                        continue;
                    }

                    for(auto line : m->getLines()) {
                        QPointF pF1 = QPointF(numeric_limits<double>::quiet_NaN(), numeric_limits<double>::quiet_NaN());
                        pF1.setX(xAxis.sampleToCoordinate(line.p1));
                        pF1.setY(yAxis[0].sampleToCoordinate(line.p1));
                        QPointF pF2 = QPointF(numeric_limits<double>::quiet_NaN(), numeric_limits<double>::quiet_NaN());
                        pF2.setX(xAxis.sampleToCoordinate(line.p2));
                        pF2.setY(yAxis[0].sampleToCoordinate(line.p2));
                        auto p1 = plotValueToPixel(pF1, i);
                        auto p2 = plotValueToPixel(pF2, i);
                        if(!plotRect.contains(p1) && !plotRect.contains(p2)) {
                            // completely out of frame
                            continue;
                        }
                        // draw line
                        p.drawLine(p1, p2);
                    }

                    if(pref.Marker.clipToYAxis) {
                        // clip Y coordinate of markers to visible area (always show markers, even when out of range)
                        if(point.y() < plotRect.top()) {
                            point.ry() = plotRect.top();
                        } else if(point.y() > plotRect.bottom()) {
                            point.ry() = plotRect.bottom();
                        }
                    }

                    if(!plotRect.contains(point)) {
                        // out of screen
                        continue;
                    }
                    auto symbol = m->getSymbol();
                    point += QPoint(-symbol.width()/2, -symbol.height());
                    // ignore clipRect for markers
                    p.setClipping(false);
                    p.drawPixmap(point, symbol);
                    p.setClipping(true);
                }
            }
        }
        // plot constant lines
        for(auto line : constantLines) {
            // skip lines on wrong axis
            if(i == 0 && line->getAxis() != XYPlotConstantLine::Axis::Primary) {
                continue;
            }
            if(i == 1 && line->getAxis() != XYPlotConstantLine::Axis::Secondary) {
                continue;
            }
            pen = QPen(line->getColor(), pref.Graphs.lineWidth);
            pen.setCosmetic(true);
            if(i == 1) {
                pen.setStyle(Qt::DotLine);
            } else {
                pen.setStyle(Qt::SolidLine);
            }
            p.setPen(pen);
            for(unsigned int j=1;j<line->getPoints().size();j++) {
                // scale to plot coordinates
                auto p1 = plotValueToPixel(line->getPoints()[j-1], i);
                auto p2 = plotValueToPixel(line->getPoints()[j], i);
                // draw line
                p.drawLine(p1, p2);
            }
        }
        p.setClipping(false);
    }

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

    // only show limit indication if there are limit lines configured
    if(constantLines.size() > 0) {
        switch(pref.Graphs.limitIndication) {
        case GraphLimitIndication::PassFailText: {
            QString text;
            if(limitPassing) {
                p.setPen(Qt::green);
                text = "PASS";
            } else {
                p.setPen(Qt::red);
                text = "FAIL";
            }
            auto font = p.font();
            font.setPixelSize(20);
            p.setFont(font);
            p.drawText(plotRect.x() + 2, plotRect.y() + 22, text);
        }
            break;
        case GraphLimitIndication::Overlay:
            if(!limitPassing) {
                p.setOpacity(0.5);
                p.setBrush(Qt::red);
                p.setPen(Qt::red);
                p.drawRect(plotRect);
                auto font = p.font();
                font.setPixelSize(20);
                p.setFont(font);
                p.setOpacity(1.0);
                p.setPen(Qt::red);
                p.drawText(plotRect, Qt::AlignCenter, "LIMIT FAIL");
            }
            break;
        default:
            break;
        }
    }


    // show sweep indicator if activated
    if((xAxis.getType() == XAxis::Type::Frequency || xAxis.getType() == XAxis::Type::TimeZeroSpan || xAxis.getType() == XAxis::Type::Power)
            && !isnan(xSweep)) {
        if(xSweep >= xAxis.getRangeMin() && xSweep <= xAxis.getRangeMax()) {
            auto xpos = plotValueToPixel(QPointF(xSweep, 1.0), 0);
            pen = QPen(pref.Graphs.Color.axis);
            pen.setCosmetic(true);
            p.setPen(pen);
            if(pref.Graphs.SweepIndicator.line) {
                p.drawLine(xpos.x(), plotAreaTop, xpos.x(), plotAreaBottom);
            }
            if(pref.Graphs.SweepIndicator.triangle) {
                for(int i=0;i<pref.Graphs.SweepIndicator.triangleSize;i++) {
                    p.drawLine(xpos.x() - i,plotAreaBottom+i+1, xpos.x() + i, plotAreaBottom+i+1);
                }
            }
        }
    }

    if(dropPending) {
        p.setOpacity(dropOpacity);
        p.setBrush(dropBackgroundColor);
        p.setPen(dropForegroundColor);

        auto dropRect = getDropRect();

        if((yAxis[0].getType() == YAxis::Type::Disabled || !supported(dropTrace, yAxis[0].getType()))
            || (yAxis[1].getType() == YAxis::Type::Disabled || !supported(dropTrace, yAxis[1].getType()))) {
            // only one axis enabled, show drop area over whole plot
            p.drawRect(dropRect);
            auto font = p.font();
            font.setPixelSize(20);
            p.setFont(font);
            p.setOpacity(1.0);
            p.setPen(dropSection == DropSection::OnPlot ? dropHighlightColor : dropForegroundColor);
            auto text = "Drop here to add\n" + dropTrace->name() + "\nto XY-plot";
            p.drawText(dropRect, Qt::AlignCenter, text);
            dropOnLeftAxis = true;
            dropOnRightAxis = true;
        } else {
            // both axis enabled, show regions
            auto leftRect = dropRect;
            leftRect.setWidth(dropRect.width() * 0.333);
            auto centerRect = dropRect;
            centerRect.setX(centerRect.x() + dropRect.width() * 0.333);
            centerRect.setWidth(dropRect.width() * 0.333);
            auto rightRect = dropRect;
            rightRect.setX(rightRect.x() + dropRect.width() * 0.666);
            rightRect.setWidth(dropRect.width() * 0.333);
            p.drawRect(leftRect);
            p.drawRect(centerRect);
            p.drawRect(rightRect);
            p.setOpacity(1.0);
            p.setPen(dropForegroundColor);
            auto font = p.font();
            font.setPixelSize(20);
            p.setFont(font);
            if(dropSection == DropSection::OnPlot && leftRect.contains(dropPosition)) {
                p.setPen(dropHighlightColor);
                dropOnLeftAxis = true;
                dropOnRightAxis = false;
            } else {
                p.setPen(dropForegroundColor);
            }
            p.drawText(leftRect, Qt::AlignCenter, "Drop here to add\nto primary axis");
            if(dropSection == DropSection::OnPlot && centerRect.contains(dropPosition)) {
                p.setPen(dropHighlightColor);
                dropOnLeftAxis = true;
                dropOnRightAxis = true;
            } else {
                p.setPen(dropForegroundColor);
            }
            p.drawText(centerRect, Qt::AlignCenter, "Drop here to add\nto boths axes");
            if(dropSection == DropSection::OnPlot && rightRect.contains(dropPosition)) {
                p.setPen(dropHighlightColor);
                dropOnLeftAxis = false;
                dropOnRightAxis = true;
            } else {
                p.setPen(dropForegroundColor);
            }
            p.drawText(rightRect, Qt::AlignCenter, "Drop here to add\nto secondary axis");
        }
    }
}

void TraceXYPlot::updateAxisTicks()
{
    if(xAxisMode != XAxisMode::Manual) {
        // automatic mode, figure out limits
        double max = std::numeric_limits<double>::lowest();
        double min = std::numeric_limits<double>::max();
        if(xAxisMode == XAxisMode::UseSpan) {
            min = sweep_fmin;
            max = sweep_fmax;
        } else if(xAxisMode == XAxisMode::FitTraces) {
            for(auto t : traces) {
                bool enabled = (tracesAxis[0].find(t.first) != tracesAxis[0].end()
                        || tracesAxis[1].find(t.first) != tracesAxis[1].end());
                auto trace = t.first;
                if(enabled && trace->isVisible()) {
                    if(!trace->size()) {
                        // empty trace, do not use for automatic axis calculation
                        continue;
                    }
                    // this trace is currently displayed
                    double trace_min = trace->minX();
                    double trace_max = trace->maxX();
                    if(xAxis.getType() == XAxis::Type::Distance) {
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
        xAxis.set(xAxis.getType(), xAxis.getLog(), true, min, max, 0, xAxis.getAutoDivs());
    }

    for(int i=0;i<2;i++) {
        if(yAxis[i].getAutorange()) {
            // automatic mode, figure out limits
            double max = std::numeric_limits<double>::lowest();
            double min = std::numeric_limits<double>::max();
            for(auto t : tracesAxis[i]) {
                if(!t->isVisible()) {
                    continue;
                }
                unsigned int samples = t->size();
                for(unsigned int j=0;j<samples;j++) {
                    auto point = traceToCoordinate(t, j, yAxis[i]);

                    if(point.x() < xAxis.getRangeMin() || point.x() > xAxis.getRangeMax()) {
                        // this point is not in the displayed X range, skip for auto Y range calculation
                        continue;
                    }

                    if(isnan(point.y()) || isinf(point.y())) {
                        // skip NaN and Infty values
                        continue;
                    }

                    if(point.y() > max) {
                        max = point.y();
                    }
                    if(point.y() < min) {
                        min = point.y();
                    }
                }
            }
            if(max >= min) {
                auto range = max - min;
                if(yAxis[i].getLog()){
                    // log axis

                    double maxLog10 = log10(abs(max));
                    // prevent zero-crossing
                    if(min <= 0.0 && max > 0) {
                        min = pow(10, maxLog10 - 3); // just show 3 decades by default
                    } else if(min >= 0.0 && max < 0) {
                        // same thing if negative
                        min = -pow(10, maxLog10 - 3);
                    }
                    // add 5% visible range
                    double ratio = log10(max/min);
                    max *= pow(10, ratio * 0.05);
                    min /= pow(10, ratio * 0.05);
                } else {
                    // linear axis
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
                }
            } else {
                // max/min still at default values, no valid samples are available for this axis, use default range
                if(!yAxis[i].getLog()) {
                    // linear axis
                    max = 1.0;
                    min = -1.0;
                } else {
                    // log axis
                    max = 100.0;
                    min = 0.1;
                }
            }
            yAxis[i].set(yAxis[i].getType(), yAxis[i].getLog(), true, min, max, 0, xAxis.getAutoDivs());
        }
    }
}

TraceXYPlot::XAxisMode TraceXYPlot::AxisModeFromName(QString name)
{
    for(unsigned int i=0;i<(int) XAxisMode::Last;i++) {
         if(AxisModeToName((XAxisMode) i) == name) {
             return (XAxisMode) i;
         }
     }
     // not found, use default
     return XAxisMode::UseSpan;
}

QString TraceXYPlot::AxisModeToName(TraceXYPlot::XAxisMode mode)
{
    switch(mode) {
    case XAxisMode::Manual: return "Manual"; break;
    case XAxisMode::FitTraces: return "Fit Traces"; break;
    case XAxisMode::UseSpan: return "Use Span"; break;
    default: return "Unknown";
    }
}

void TraceXYPlot::enableTraceAxis(Trace *t, int axis, bool enabled)
{
    if(enabled && !supported(t, yAxis[axis].getType())) {
        // unable to add trace to the requested axis
        return;
    }
    if(enabled) {
        TracePlot::enableTrace(t, true);
    } else {
        // only disable trace on parent trace list if disabled for both axes
        int otherAxis = axis ? 0 : 1;
        if(tracesAxis[otherAxis].find(t) == tracesAxis[otherAxis].end()) {
            TracePlot::enableTrace(t, false);
        }
    }
    bool alreadyEnabled = tracesAxis[axis].find(t) != tracesAxis[axis].end();
    if(alreadyEnabled != enabled) {
        if(enabled) {
            tracesAxis[axis].insert(t);
        } else {
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

bool TraceXYPlot::domainMatch(Trace *t)
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

bool TraceXYPlot::supported(Trace *t, YAxis::Type type)
{
    if(!domainMatch(t)) {
        return false;
    }

    switch(type) {
    case YAxis::Type::Disabled:
        return false;
    case YAxis::Type::VSWR:
    case YAxis::Type::SeriesR:
    case YAxis::Type::Reactance:
    case YAxis::Type::Capacitance:
    case YAxis::Type::Inductance:
    case YAxis::Type::QualityFactor:
    case YAxis::Type::AbsImpedance:
        if(!t->isReflection()) {
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}

QPointF TraceXYPlot::traceToCoordinate(Trace *t, unsigned int sample, YAxis &yaxis)
{
    QPointF ret = QPointF(numeric_limits<double>::quiet_NaN(), numeric_limits<double>::quiet_NaN());
    ret.setX(xAxis.sampleToCoordinate(t->sample(sample), t, sample));
    ret.setY(yaxis.sampleToCoordinate(t->sample(sample), t, sample));
    return ret;
}

QPoint TraceXYPlot::plotValueToPixel(QPointF plotValue, int Yaxis)
{
    QPoint p;
    p.setX(xAxis.transform(plotValue.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth));
    p.setY(yAxis[Yaxis].transform(plotValue.y(), plotAreaBottom, plotAreaTop));
    return p;
}

QPointF TraceXYPlot::pixelToPlotValue(QPoint pixel, int Yaxis)
{
    QPointF p;
    p.setX(xAxis.inverseTransform(pixel.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth));
    p.setY(yAxis[Yaxis].inverseTransform(pixel.y(), plotAreaBottom, plotAreaTop));
    return p;
}

QPoint TraceXYPlot::markerToPixel(Marker *m)
{
    auto t = m->getTrace();
    double xPosition = m->getPosition();
    double xPosGraph = xPosition;
    if(xAxis.getType() == XAxis::Type::Distance) {
        xPosGraph = t->timeToDistance(xPosition);
    }
    if (xPosGraph < xAxis.getRangeMin() || xPosGraph > xAxis.getRangeMax()) {
        // marker not in graph range
        return QPoint(0.0, 0.0);
    }
    if(xPosition < t->minX() || xPosition > t->maxX()) {
        // marker not in trace range
        return QPoint(0.0, 0.0);
    }
    auto index = t->index(xPosition);
    QPointF markerPoint;
    if(xPosition < t->sample(index).x && index > 0) {
        // marker is not located exactly at this point, interpolate display location
        QPointF l0 = traceToCoordinate(t, index - 1, yAxis[0]);
        QPointF l1 = traceToCoordinate(t, index, yAxis[0]);
        auto t0 = (xPosition - t->sample(index - 1).x) / (t->sample(index).x - t->sample(index - 1).x);
        markerPoint = l0 + (l1 - l0) * t0;
    } else {
        markerPoint = traceToCoordinate(t, t->index(xPosition), yAxis[0]);
    }
    return plotValueToPixel(markerPoint, 0);
}

double TraceXYPlot::nearestTracePoint(Trace *t, QPoint pixel, double *distance)
{
    if(!tracesAxis[0].count(t)) {
        // trace not enabled
        return 0;
    }
    double closestDistance = numeric_limits<double>::max();
    double closestXpos = 0;
    unsigned int closestIndex = 0;
    auto samples = t->size();
    for(unsigned int i=0;i<samples;i++) {
        auto point = traceToCoordinate(t, i, yAxis[0]);
        if(isnan(point.x()) || isnan(point.y())) {
            continue;
        }
        auto plotPoint = plotValueToPixel(point, 0);
        QPointF diff = plotPoint - pixel;
        auto distance = diff.x() * diff.x() + diff.y() * diff.y();
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = point.x();
            closestIndex = i;
        }
    }
    if(xAxis.getType() == XAxis::Type::Distance) {
        closestXpos = t->distanceToTime(closestXpos);
    }
    closestDistance = sqrt(closestDistance);
    if(closestIndex > 0) {
        auto l1 = plotValueToPixel(traceToCoordinate(t, closestIndex - 1, yAxis[0]), 0);
        auto l2 = plotValueToPixel(traceToCoordinate(t, closestIndex, yAxis[0]), 0);
        double ratio;
        auto distance = Util::distanceToLine(pixel, l1, l2, nullptr, &ratio);
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(closestIndex-1).x + (t->sample(closestIndex).x - t->sample(closestIndex-1).x) * ratio;
        }
    }
    if(closestIndex < t->size() - 1) {
        auto l1 = plotValueToPixel(traceToCoordinate(t, closestIndex, yAxis[0]), 0);
        auto l2 = plotValueToPixel(traceToCoordinate(t, closestIndex + 1, yAxis[0]), 0);
        double ratio;
        auto distance = Util::distanceToLine(pixel, l1, l2, nullptr, &ratio);
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(closestIndex).x + (t->sample(closestIndex+1).x - t->sample(closestIndex).x) * ratio;
        }
    }
    if(distance) {
        *distance = closestDistance;
    }
    return closestXpos;
}

bool TraceXYPlot::markerVisible(double x)
{
    if(x >= min(xAxis.getRangeMin(), xAxis.getRangeMax()) && x <= max(xAxis.getRangeMax(), xAxis.getRangeMin())) {
        return true;
    } else {
        return false;
    }
}

void TraceXYPlot::traceDropped(Trace *t, QPoint position)
{
    Q_UNUSED(position)
    if(!supported(t)) {
        // needs to switch to a different setting for the graph
        if(!InformationBox::AskQuestion("Graph Configuration Change", "You dropped a trace that is not supported with the currently configured axes."
                                    " Do you want to remove all traces and change the graph to the correct configuration?", true, "DomainChangeRequest")) {
            // user declined to change domain, to not add trace
            return;
        }
        if(!configureForTrace(t)) {
            // failed to configure
            return;
        }
    }
    if(yAxis[0].getType() == YAxis::Type::Disabled && yAxis[1].getType() == YAxis::Type::Disabled) {
        // no Y axis enabled, unable to drop
        return;
    }
    if(yAxis[0].getType() == YAxis::Type::Disabled) {
        // only axis 1 enabled
        enableTraceAxis(t, 1, true);
        return;
    }
    if(yAxis[1].getType() == YAxis::Type::Disabled) {
        // only axis 0 enabled
        enableTraceAxis(t, 0, true);
        return;
    }
    // both axis enabled
    if(dropOnLeftAxis) {
        enableTraceAxis(t, 0, true);
    }
    if(dropOnRightAxis) {
        enableTraceAxis(t, 1, true);
    }
}

QString TraceXYPlot::mouseText(QPoint pos)
{
    QString ret;
    if(positionWithinGraphArea(pos)) {
        // cursor within plot area
        QPointF coords[2];
        coords[0] = pixelToPlotValue(pos, 0);
        coords[1] = pixelToPlotValue(pos, 1);
        int significantDigits = floor(log10(abs(xAxis.getRangeMax()))) - floor(log10((abs(xAxis.getRangeMax() - xAxis.getRangeMin())) / 1000.0)) + 1;
        ret += Unit::ToString(coords[0].x(), xAxis.Unit(), "fpnum kMG", significantDigits) + "\n";
        for(int i=0;i<2;i++) {
            if(yAxis[i].getType() != YAxis::Type::Disabled) {
                auto max = qMax(abs(yAxis[i].getRangeMax()), abs(yAxis[i].getRangeMin()));
                auto step = abs(yAxis[i].getRangeMax() - yAxis[i].getRangeMin()) / 1000.0;
                significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
                ret += Unit::ToString(coords[i].y(), yAxis[i].Unit(getModel().getSource()), yAxis[i].Prefixes(getModel().getSource()), significantDigits) + "\n";
            }
        }
    }
    return ret;
}

XYPlotConstantLine::XYPlotConstantLine()
    : name("Name"),
      color(Qt::red),
      axis(Axis::Primary),
      passFail(PassFail::DontCare),
      points()
{

}

QColor XYPlotConstantLine::getColor() const
{
    return color;
}

void XYPlotConstantLine::setColor(const QColor &value)
{
    color = value;
}

void XYPlotConstantLine::fromJSON(nlohmann::json j)
{
    name = QString::fromStdString(j.value("name", "Name"));
    color = QColor(QString::fromStdString(j.value("color", "red")));
    axis = AxisFromString(QString::fromStdString(j.value("axis", AxisToString(Axis::Primary).toStdString())));
    if(axis == Axis::Last) {
        axis = Axis::Primary;
    }
    passFail = PassFailFromString(QString::fromStdString(j.value("passfail", PassFailToString(PassFail::DontCare).toStdString())));
    if(passFail == PassFail::Last) {
        passFail = PassFail::DontCare;
    }
    points.clear();
    if(j.contains("points")) {
        for(auto jp : j["points"]) {
            QPointF p;
            p.setX(jp.value("x", 0.0));
            p.setY(jp.value("y", 0.0));
            points.push_back(p);
        }
    }
}

nlohmann::json XYPlotConstantLine::toJSON()
{
    nlohmann::json j;
    j["name"] = name.toStdString();
    j["color"] = color.name().toStdString();
    j["axis"] = AxisToString(axis).toStdString();
    j["passfail"] = PassFailToString(passFail).toStdString();
    nlohmann::json jpoints;
    for(auto p : points) {
        nlohmann::json jp;
        jp["x"] = p.x();
        jp["y"] = p.y();
        jpoints.push_back(jp);
    }
    j["points"] = jpoints;
    return j;
}

bool XYPlotConstantLine::pass(QPointF testPoint)
{
    if(passFail == PassFail::DontCare) {
        // always passes
        return true;
    }
    if(points.size() < 2) {
        // no lines, always passes
        return true;
    }
    if(testPoint.x() < points.front().x() || testPoint.x() > points.back().x()) {
        // out of range, always passes
        return true;
    }

    // check for inf/nan on Y value
    if(isnan(testPoint.y()) || isinf(testPoint.y())) {
        // can not actually evaluate the limits, use pass/fail from preferences for this case
        return Preferences::getInstance().Graphs.limitNaNpasses;
    }

    auto p = lower_bound(points.begin(), points.end(), testPoint.x(), [](QPointF p, double x) -> bool {
        return p.x() < x;
    });
    double compareY = 0.0;
    if(p->x() == testPoint.x()) {
        // Exact match
        compareY = p->y();
    } else {
        // need to interpolate
        auto high = p;
        p--;
        auto low = p;
        double alpha = (testPoint.x() - low->x()) / (high->x() - low->x());
        compareY = low->y() * (1 - alpha) + high->y() * alpha;
    }
    if (compareY < testPoint.y() && passFail == PassFail::HighLimit) {
        // high limit exceeded
        return false;
    }
    if (compareY > testPoint.y() && passFail == PassFail::LowLimit) {
        // low limit exceeded
        return false;
    }
    return true;
}

QString XYPlotConstantLine::AxisToString(Axis axis)
{
    switch(axis) {
    case Axis::Primary: return "Primary";
    case Axis::Secondary: return "Secondary";
    default: return "Invalid";
    }
}

XYPlotConstantLine::Axis XYPlotConstantLine::AxisFromString(QString s)
{
    for(unsigned int i=0;i<(unsigned int)Axis::Last;i++) {
        if(AxisToString((Axis) i) == s) {
            return (Axis) i;
        }
    }
    return Axis::Last;
}

QString XYPlotConstantLine::PassFailToString(PassFail pf)
{
    switch(pf) {
    case PassFail::DontCare: return "Dont Care";
    case PassFail::HighLimit: return "High limit";
    case PassFail::LowLimit: return "Low limit";
    default: return "Invalid";
    }
}

XYPlotConstantLine::PassFail XYPlotConstantLine::PassFailFromString(QString s)
{
    for(unsigned int i=0;i<(unsigned int)PassFail::Last;i++) {
        if(PassFailToString((PassFail) i) == s) {
            return (PassFail) i;
        }
    }
    return PassFail::Last;

}

void XYPlotConstantLine::editDialog(QString xUnit, QString yUnitPrimary, QString yUnitSecondary)
{
    auto d = new QDialog();
    auto ui = new Ui_XYPlotConstantLineEditDialog;
    ui->setupUi(d);

    ui->name->setText(name);
    ui->color->setColor(color);

    for(unsigned int i=0;i<(unsigned int)Axis::Last;i++) {
        ui->axis->addItem(AxisToString((Axis) i));
    }
    ui->axis->setCurrentIndex((int) axis);

    for(unsigned int i=0;i<(unsigned int)PassFail::Last;i++) {
        ui->passFail->addItem(PassFailToString((PassFail) i));
    }
    ui->passFail->setCurrentIndex((int) passFail);

    connect(ui->name, &QLineEdit::textChanged, [=](){
       name = ui->name->text();
    });
    connect(ui->color, &ColorPickerButton::colorChanged, [=](){
       color = ui->color->getColor();
    });
    auto updatePointTable = [=](){
        sort(points.begin(), points.end(), [](QPointF &a, QPointF &b) -> bool{
            return a.x() < b.x();
        });
        ui->pointTable->blockSignals(true);
        ui->pointTable->clear();
        ui->pointTable->setHorizontalHeaderLabels({"#", "X", "Y"});
        ui->pointTable->setColumnCount(3);
        ui->pointTable->setRowCount(points.size());
        QString yUnit = axis == Axis::Primary ? yUnitPrimary : yUnitSecondary;
        for(unsigned int i=0;i<points.size();i++) {
            auto numItem = new QTableWidgetItem(QString::number(i+1));
            numItem->setFlags(numItem->flags() &= ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
            auto xItem = new QTableWidgetItem(Unit::ToString(points[i].x(), xUnit, "pnum kMG", 6));
            auto yItem = new QTableWidgetItem(Unit::ToString(points[i].y(), yUnit, "pnum kMG", 6));
            ui->pointTable->setItem(i, 0, numItem);
            ui->pointTable->setItem(i, 1, xItem);
            ui->pointTable->setItem(i, 2, yItem);
        }
        ui->pointTable->blockSignals(false);
    };
    connect(ui->axis, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        axis = (Axis) ui->axis->currentIndex();
        updatePointTable();
    });
    connect(ui->passFail, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        passFail = (PassFail) ui->passFail->currentIndex();
    });

    // handle adding/removing of points
    connect(ui->pointTable, &QTableWidget::itemChanged, [=](QTableWidgetItem *item){
        auto row = ui->pointTable->row(item);
        auto column = ui->pointTable->column(item);
        auto& point = points[row];
        if(column == 1) {
            // changed X coordinate
            point.setX(Unit::FromString(item->text(), xUnit, "pnum kMG"));
            // potentially reordered the points, update whole table
            updatePointTable();
        } else {
            // change Y coordinate
            QString yUnit = axis == Axis::Primary ? yUnitPrimary : yUnitSecondary;
            point.setY(Unit::FromString(item->text(), yUnit, "pnum kMG"));
            // point order only depends on X coordinate, no table update necessary, only update text of the changed item
            ui->pointTable->blockSignals(true);
            item->setText(Unit::ToString(point.y(), yUnit, "pnum kMG", 6));
            ui->pointTable->blockSignals(false);
        }
    });

    connect(ui->addPoint, &QPushButton::clicked, [=](){
        points.push_back(QPointF());
        updatePointTable();
    });
    connect(ui->removePoint, &QPushButton::clicked, [=](){
        auto row = ui->pointTable->currentRow();
        if(row >= 0 && row < (int) points.size()) {
            points.erase(points.begin() + row);
            updatePointTable();
        }
    });

    connect(d, &QDialog::finished, this, &XYPlotConstantLine::editingFinished);

    updatePointTable();

    if(AppWindow::showGUI()) {
        d->show();
    }
}

QString XYPlotConstantLine::getDescription()
{
    QString ret;
    ret += name;
    ret += ", " + AxisToString(axis) + " axis, ";
    ret += QString::number(points.size()) + " points, limit: "+PassFailToString(passFail);
    return ret;
}

XYPlotConstantLine::Axis XYPlotConstantLine::getAxis() const
{
    return axis;
}

const std::vector<QPointF> &XYPlotConstantLine::getPoints() const
{
    return points;
}
