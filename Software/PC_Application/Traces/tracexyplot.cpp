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

    // Setup default axis
    setYAxis(0, YAxis::Type::Magnitude, false, false, -120, 20, 10);
    setYAxis(1, YAxis::Type::Phase, false, false, -180, 180, 30);
    // enable autoscaling and set for full span (no information about actual span available yet)
    updateSpan(0, 6000000000);
    setXAxis(XAxis::Type::Frequency, XAxisMode::UseSpan, false, 0, 6000000000, 600000000);
    initializeTraceInfo();
}

TraceXYPlot::~TraceXYPlot()
{
    for(auto l : constantLines) {
        delete l;
    }
}

void TraceXYPlot::setYAxis(int axis, YAxis::Type type, bool log, bool autorange, double min, double max, double div)
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
    yAxis[axis].set(type, log, autorange, min, max, div);
    traceRemovalPending = true;
    updateContextMenu();
    replot();
}

void TraceXYPlot::setXAxis(XAxis::Type type, XAxisMode mode, bool log, double min, double max, double div)
{
    bool autorange = false;
    if(mode == XAxisMode::FitTraces || mode == XAxisMode::UseSpan) {
        autorange = true;
    }
    xAxis.set(type, log, autorange, min, max, div);
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

nlohmann::json TraceXYPlot::toJSON()
{
    nlohmann::json j;
    nlohmann::json jX;
    jX["type"] = xAxis.TypeToName().toStdString();
    jX["mode"] = AxisModeToName(xAxisMode).toStdString();
    jX["log"] = xAxis.getLog();
    jX["min"] = xAxis.getRangeMin();
    jX["max"] = xAxis.getRangeMax();
    jX["div"] = xAxis.getRangeDiv();
    j["XAxis"] = jX;
    for(unsigned int i=0;i<2;i++) {
        nlohmann::json jY;
        jY["type"] = yAxis[i].TypeToName().toStdString();
        jY["log"] = yAxis[i].getLog();
        jY["autorange"] = yAxis[i].getAutorange();
        jY["min"] = yAxis[i].getRangeMin();
        jY["max"] = yAxis[i].getRangeMax();
        jY["div"] = yAxis[i].getRangeDiv();
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
    auto xdiv = jX.value("div", 600000000.0);
    auto xlog = jX.value("log", false);
    setXAxis(xtype, xmode, xlog, xmin, xmax, xdiv);
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
        auto ydiv = jY[i].value("div", 10.0);
        setYAxis(i, ytype, ylog, yauto, ymin, ymax, ydiv);
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
    if(AppWindow::showGUI()) {
        setup->show();
    }
}

bool TraceXYPlot::configureForTrace(Trace *t)
{
    switch(t->outputType()) {
    case Trace::DataType::Frequency:
        setXAxis(XAxis::Type::Frequency, XAxisMode::FitTraces, false, 0, 1, 0.1);
        setYAxis(0, YAxis::Type::Magnitude, false, true, 0, 1, 1.0);
        setYAxis(1, YAxis::Type::Phase, false, true, 0, 1, 1.0);
        break;
    case Trace::DataType::Time:
        setXAxis(XAxis::Type::Time, XAxisMode::FitTraces, false, 0, 1, 0.1);
        setYAxis(0, YAxis::Type::ImpulseMag, false, true, 0, 1, 1.0);
        setYAxis(1, YAxis::Type::Disabled, false, true, 0, 1, 1.0);
        break;
    case Trace::DataType::Power:
        setXAxis(XAxis::Type::Power, XAxisMode::FitTraces, false, 0, 1, 0.1);
        setYAxis(0, YAxis::Type::Magnitude, false, true, 0, 1, 1.0);
        setYAxis(1, YAxis::Type::Phase, false, true, 0, 1, 1.0);
        break;
    case Trace::DataType::TimeZeroSpan:
        setXAxis(XAxis::Type::TimeZeroSpan, XAxisMode::FitTraces, false, 0, 1, 0.1);
        setYAxis(0, YAxis::Type::Magnitude, false, true, 0, 1, 1.0);
        setYAxis(1, YAxis::Type::Phase, false, true, 0, 1, 1.0);
        break;
    case Trace::DataType::Invalid:
        // unable to add
        return false;
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
    auto pref = Preferences::getInstance();

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

            for(unsigned int j = 0; j < yAxis[i].getTicks().size(); j++) {
                auto yCoord = yAxis[i].transform(yAxis[i].getTicks()[j], w.height() - xAxisSpace, plotAreaTop);
                p.setPen(QPen(pref.Graphs.Color.axis, 1));
                // draw tickmark on axis
                auto tickStart = i == 0 ? plotAreaLeft : plotAreaLeft + plotAreaWidth;
                auto tickLen = i == 0 ? -2 : 2;
                p.drawLine(tickStart, yCoord, tickStart + tickLen, yCoord);
                QString unit = "";
                QString prefix = " ";
                if(pref.Graphs.showUnits) {
                    unit = yAxis[i].Unit();
                    prefix = yAxis[i].Prefixes();
                }
                auto tickValue = Unit::ToString(yAxis[i].getTicks()[j], unit, prefix, significantDigits);
                if(i == 0) {
                    p.drawText(QRectF(0, yCoord - pref.Graphs.fontSizeAxis/2 - 2, tickStart + 2 * tickLen, pref.Graphs.fontSizeAxis), Qt::AlignRight, tickValue);
                } else {
                    p.drawText(QRectF(tickStart + 2 * tickLen + 2, yCoord - pref.Graphs.fontSizeAxis/2 - 2, yAxisSpace, pref.Graphs.fontSizeAxis), Qt::AlignLeft, tickValue);
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
                            int yCoordTop = yAxis[i].transform(yAxis[i].getTicks()[j], plotAreaTop, w.height() - xAxisSpace);
                            int yCoordBot = yAxis[i].transform(yAxis[i].getTicks()[j-1], plotAreaTop, w.height() - xAxisSpace);
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
            }
            if(i == 0 && nPoints > 0) {
                // only draw markers on primary YAxis and if the trace has at least one point
                auto markers = t->getMarkers();
                for(auto m : markers) {
                    if(!m->isVisible()) {
                        continue;
                    }
                    double xPosition = m->getPosition();
                    if (xPosition < xAxis.getRangeMin() || xPosition > xAxis.getRangeMax()) {
                        // marker not in graph range
                        continue;
                    }
                    if(xPosition < t->minX() || xPosition > t->maxX()) {
                        // marker not in trace range
                        continue;
                    }
                    auto t = m->getTrace();
                    auto index = t->index(xPosition);
                    QPointF markerPoint;
                    if(xPosition < t->sample(index).x && index > 0) {
                        // marker is not located exactly at this point, interpolate display location
                        QPointF l0 = traceToCoordinate(t, index - 1, yAxis[i]);
                        QPointF l1 = traceToCoordinate(t, index, yAxis[i]);
                        auto t0 = (xPosition - t->sample(index - 1).x) / (t->sample(index).x - t->sample(index - 1).x);
                        markerPoint = l0 + (l1 - l0) * t0;
                    } else {
                        markerPoint = traceToCoordinate(t, t->index(xPosition), yAxis[i]);
                    }
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
            p.drawText(QRect(2, plotAreaBottom + pref.Graphs.fontSizeAxis + 5, w.width(), pref.Graphs.fontSizeAxis), 0, front, &bounding);
            p.setPen(pref.Graphs.Color.axis);
            p.drawText(QRect(bounding.x() + bounding.width(), plotAreaBottom + pref.Graphs.fontSizeAxis + 5, w.width(), pref.Graphs.fontSizeAxis), 0, back);
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
                p.drawText(QRect(xCoord - 40, plotAreaBottom + 5, 80, pref.Graphs.fontSizeAxis), Qt::AlignHCenter, tickValue, &bounding);
                lastTickLabelEnd = bounding.x() + bounding.width();
            } else {
                // check if the same prefix was used as in the fullFreq string
                if(tickValue.at(tickValue.size() - 1) != commonPrefix) {
                    // prefix changed, we reached the next order of magnitude. Force same prefix as in fullFreq and add extra digit
                    tickValue = Unit::ToString(t, "", commonPrefix, significantDigits + 1);
                }

                tickValue.remove(0, tickValue.size() - displayLastDigits - unit.length());
                QRect bounding;
                p.drawText(QRect(xCoord - 40, plotAreaBottom + 5, 80, pref.Graphs.fontSizeAxis), Qt::AlignHCenter, tickValue, &bounding);
                lastTickLabelEnd = bounding.x() + bounding.width();
                p.setPen(QPen(QColor("orange")));
                p.drawText(QRect(0, plotAreaBottom + 5, bounding.x() - 1, pref.Graphs.fontSizeAxis), Qt::AlignRight, "..", &bounding);
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

    if(dropPending) {
        p.setOpacity(0.5);
        p.setBrush(Qt::white);
        p.setPen(Qt::white);
        if((yAxis[0].getType() == YAxis::Type::Disabled || !supported(dropTrace, yAxis[0].getType()))
            || (yAxis[1].getType() == YAxis::Type::Disabled || !supported(dropTrace, yAxis[1].getType()))) {
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
        if(min < max) {
            xAxis.set(xAxis.getType(), xAxis.getLog(), true, min, max, 0);
        }
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
            } else {
                // max/min still at default values, no valid samples are available for this axis, use default range
                max = 1.0;
                min = -1.0;
            }
            yAxis[i].set(yAxis[i].getType(), yAxis[i].getLog(), true, min, max, 0);
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
    if(axis == 0) {
        TracePlot::enableTrace(t, enabled);
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
    QPointF plotPoint = traceToCoordinate(t, t->index(m->getPosition()), yAxis[0]);
    return plotValueToPixel(plotPoint, 0);
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
    if(xAxis.getType() == XAxis::Type::Distance) {
        closestXpos = t->distanceToTime(closestXpos);
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
    if(!supported(t)) {
        // needs to switch to a different domain for the graph
        if(!InformationBox::AskQuestion("X Axis Domain Change", "You dropped a trace that is not supported with the currently selected X axis domain."
                                    " Do you want to remove all traces and change the graph to the correct domain?", true, "DomainChangeRequest")) {
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
        int significantDigits = floor(log10(abs(xAxis.getRangeMax()))) - floor(log10((abs(xAxis.getRangeMax() - xAxis.getRangeMin())) / 1000.0)) + 1;
        ret += Unit::ToString(coords[0].x(), xAxis.Unit(), "fpnum kMG", significantDigits) + "\n";
        for(int i=0;i<2;i++) {
            if(yAxis[i].getType() != YAxis::Type::Disabled) {
                auto max = qMax(abs(yAxis[i].getRangeMax()), abs(yAxis[i].getRangeMin()));
                auto step = abs(yAxis[i].getRangeMax() - yAxis[i].getRangeMin()) / 1000.0;
                significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
                ret += Unit::ToString(coords[i].y(), yAxis[i].Unit(), yAxis[i].Prefixes(), significantDigits) + "\n";
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
