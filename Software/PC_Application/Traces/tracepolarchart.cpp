#include "tracepolarchart.h"

#include "ui_polarchartdialog.h"
#include "preferences.h"
#include "tracesmithchart.h"
#include "unit.h"
#include "Marker/marker.h"
#include "Util/util.h"
#include "appwindow.h"

#include <QFileDialog>
#include <QPainter>

using namespace std;

TracePolarChart::TracePolarChart(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
{
    limitToSpan = true;
    limitToEdge = true;
    edgeReflection = 1.0;
    dx = 0.0;
    initializeTraceInfo();
}

void TracePolarChart::wheelEvent(QWheelEvent *event)
{
    // most mousewheel have 15 degree increments, the reported delta is in 1/8th degree -> 120
    auto increment = event->angleDelta().y() / 120.0;
    // round toward bigger step in case of special higher resolution mousewheel
    int steps = increment > 0 ? ceil(increment) : floor(increment);

    constexpr double zoomfactor = 1.1;
    auto zoom = pow(zoomfactor, steps);
    edgeReflection /= zoom;

    auto incrementX = event->angleDelta().x() / 120.0;
    dx += incrementX/10;
    triggerReplot();
}

void TracePolarChart::axisSetupDialog()
{
    auto dialog = new QDialog();
    auto ui = new Ui::PolarChartDialog();
    ui->setupUi(dialog);
    if(limitToSpan) {
        ui->displayModeFreq->setCurrentIndex(1);
    } else {
        ui->displayModeFreq->setCurrentIndex(0);
    }
    if(limitToEdge) {
        ui->displayModeRefl->setCurrentIndex(1);
    } else {
        ui->displayModeRefl->setCurrentIndex(0);
    }
    ui->zoomReflection->setPrecision(3);
    ui->zoomFactor->setPrecision(3);
    ui->offsetRealAxis->setPrecision(3);
    ui->zoomReflection->setValue(edgeReflection);
    ui->zoomFactor->setValue(1.0/edgeReflection);
    ui->offsetRealAxis->setValue(dx);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
       limitToSpan = ui->displayModeFreq->currentIndex() == 1;
       limitToEdge = ui->displayModeRefl->currentIndex() == 1;
       triggerReplot();
    });
    connect(ui->zoomFactor, &SIUnitEdit::valueChanged, [=](){
        edgeReflection = 1.0 / ui->zoomFactor->value();
        ui->zoomReflection->setValueQuiet(edgeReflection);
    });
    connect(ui->zoomReflection, &SIUnitEdit::valueChanged, [=](){
        edgeReflection = ui->zoomReflection->value();
        ui->zoomFactor->setValueQuiet(1.0 / edgeReflection);
    });
    connect(ui->offsetRealAxis, &SIUnitEdit::valueChanged, [=](){
        dx = ui->offsetRealAxis->value();
    });
    if(AppWindow::showGUI()) {
        dialog->show();
    }
}

QPoint TracePolarChart::dataToPixel(std::complex<double> d)
{
    return transform.map(QPoint(d.real() * polarCoordMax * (1.0 / edgeReflection), -d.imag() * polarCoordMax * (1.0 / edgeReflection)));
}

QPoint TracePolarChart::dataToPixel(Trace::Data d)
{
    return dataToPixel(d.y);
}

std::complex<double> TracePolarChart::dataAddDx(std::complex<double> d)
{
    auto dataShift = complex<double>(dx, 0);
    d = d + dataShift;
    return d;
}

Trace::Data TracePolarChart::dataAddDx(Trace::Data d)
{
    d.y = dataAddDx(d.y);
    return d;
}

std::complex<double> TracePolarChart::pixelToData(QPoint p)
{
    auto data = transform.inverted().map(QPointF(p));
    return complex<double>(data.x() / polarCoordMax * edgeReflection, -data.y() / polarCoordMax * edgeReflection);
}

void TracePolarChart::draw(QPainter &p) {
    auto pref = Preferences::getInstance();

    p.setRenderHint(QPainter::Antialiasing);
    auto w = p.window();
    p.save();
    p.translate(w.width()/2, w.height()/2);
    auto scale = qMin(w.height(), w.width()) / (2.0 * polarCoordMax);
    p.scale(scale, scale);

    transform = p.transform();
    p.restore();

    auto drawArc = [&](SmithChartArc a) {
        a.constrainToCircle(QPointF(0,0), edgeReflection);
        auto topleft = dataToPixel(complex<double>(a.center.x() - a.radius, a.center.y() - a.radius));
        auto bottomright = dataToPixel(complex<double>(a.center.x() + a.radius, a.center.y() + a.radius));
        a.startAngle *= 5760 / (2*M_PI);
        a.spanAngle *= 5760 / (2*M_PI);
        p.drawArc(QRect(topleft, bottomright), a.startAngle, a.spanAngle);
    };

    // Outer circle
    auto pen = QPen(pref.Graphs.Color.axis);
    pen.setCosmetic(true);
    p.setPen(pen);
    drawArc(SmithChartArc(QPointF(0.0, 0.0), edgeReflection, 0, 2*M_PI));

    constexpr int Circles = 6;
    pen = QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine);
    pen.setCosmetic(true);
    p.setPen(pen);
    for(int i=1;i<Circles;i++) {
        auto radius = (double) i / Circles;
        drawArc(SmithChartArc(QPointF(0.0 + dx,0), radius, 0, 2*M_PI));
    }

    auto constraintLineToCircle = [&](PolarChartCircle cir) {
        if ( (cir.spanAngle == 90 )&& (dx != 0.0)) {
            auto angle = acos(dx/cir.radius);
            auto p1 = complex<double>(dx, cir.center.y() + cir.radius*sin(angle));
            auto p2 = complex<double>(dx, cir.center.y() - cir.radius*sin(angle));
            p.drawLine(dataToPixel(p1),dataToPixel(p2));
        }
        else {
            auto slope = tan(cir.spanAngle*2*M_PI/360);
            auto y0 = cir.center.y();
            auto f = dx;
            auto a = 1 + (slope*slope);
            auto b = (-2*cir.center.x())-(2*f*slope*slope)+(2*slope*y0)-(2*cir.center.y()*slope);
            auto c = (cir.center.x()*cir.center.x()) +(cir.center.y()*cir.center.y()) - (cir.radius*cir.radius) + (y0*y0) \
                    + (slope*slope*f*f) - (2 * slope * f * y0 ) \
                    + (2*cir.center.y()*slope*f)-(2*cir.center.y()*y0);
            auto D = (b*b) - (4 * a * c);

            auto x1 = (-b + sqrt(D))/(2*a);
            auto x2 = (-b - sqrt(D))/(2*a);
            auto y1 = slope*(x1-f)+y0;
            auto y2 = slope*(x2-f)+y0;

            auto p1 = complex<double>(x1,y1);
            auto p2 = complex<double>(x2,y2);
            p.drawLine(dataToPixel(p1),dataToPixel(p2));
        }
    };

    constexpr int Lines = 6;
    for(int i=0;i<Lines;i++) {
        auto angle = (double) i * 30;
        constraintLineToCircle(PolarChartCircle(QPointF(0,0), edgeReflection, 0, angle));
    }

    for(auto t : traces) {
        if(!t.second) {
            // trace not enabled in plot
            continue;
        }
        auto trace = t.first;
        if(!trace->isVisible()) {
            // trace marked invisible
            continue;
        }
        pen = QPen(trace->color(), pref.Graphs.lineWidth);
        pen.setCosmetic(true);
        p.setPen(pen);
        int nPoints = trace->size();
        for(int i=1;i<nPoints;i++) {
            auto last = trace->sample(i-1);
            auto now = trace->sample(i);
            if (limitToSpan && (trace->getDataType() == Trace::DataType::Frequency) && (last.x < sweep_fmin || now.x > sweep_fmax)) {
                continue;
            }
            if(isnan(now.y.real())) {
                break;
            }

            last = dataAddDx(last);
            now = dataAddDx(now);

            if (limitToEdge && (abs(last.y) > edgeReflection || abs(now.y) > edgeReflection)) {
                // outside of visible area
                continue;
            }
            // scale to size of smith diagram
            auto p1 = dataToPixel(last);
            auto p2 = dataToPixel(now);
            // draw line
            p.drawLine(p1, p2);
        }
        if(trace->size() > 0) {
            // only draw markers if the trace has at least one point
            auto markers = t.first->getMarkers();
            for(auto m : markers) {
                if(!m->isVisible()) {
                    continue;
                }
                if (limitToSpan && (m->getPosition() < sweep_fmin || m->getPosition() > sweep_fmax)) {
                    continue;
                }
                if(m->getPosition() < trace->minX() || m->getPosition() > trace->maxX()) {
                    // marker not in trace range
                    continue;
                }
                auto coords = m->getData();
                coords = dataAddDx(coords);

                if (limitToEdge && abs(coords) > edgeReflection) {
                    // outside of visible area
                    continue;
                }
                auto point = dataToPixel(coords);
                auto symbol = m->getSymbol();
                p.drawPixmap(point.x() - symbol.width()/2, point.y() - symbol.height(), symbol);
            }
        }
    }

    if(dropPending) {
        // TODO adjust coords due to shifted restore
        p.setOpacity(0.5);
        p.setBrush(Qt::white);
        p.setPen(Qt::white);
        p.drawEllipse(-polarCoordMax, -polarCoordMax, 2*polarCoordMax, 2*polarCoordMax);
        auto font = p.font();
        font.setPixelSize(20);
        p.setFont(font);
        p.setOpacity(1.0);
        p.setPen(Qt::white);
        auto text = "Drop here to add\n" + dropTrace->name() + "\nto polar chart";
        p.drawText(p.window(), Qt::AlignCenter, text);
    } else {
    }

}

void TracePolarChart::fromJSON(nlohmann::json j)
{
    limitToSpan = j.value("limit_to_span", true);
    limitToEdge = j.value("limit_to_edge", false);
    edgeReflection = j.value("edge_reflection", 1.0);
    dx = j.value("offset_axis_x", 0.0);
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

nlohmann::json TracePolarChart::toJSON()
{
    nlohmann::json j;
    j["limit_to_span"] = limitToSpan;
    j["limit_to_edge"] = limitToEdge;
    j["edge_reflection"] = edgeReflection;
    j["offset_axis_x"] = dx;
    nlohmann::json jtraces;
    for(auto t : traces) {
        if(t.second) {
            jtraces.push_back(t.first->toHash());
        }
    }
    j["traces"] = jtraces;
    return j;
}

double TracePolarChart::nearestTracePoint(Trace *t, QPoint pixel, double *distance)
{
    double closestDistance = numeric_limits<double>::max();
    double closestXpos = 0;
    unsigned int closestIndex = 0;
    auto samples = t->size();
    for(unsigned int i=0;i<samples;i++) {
        auto data = t->sample(i);
        data = dataAddDx(data);
        auto plotPoint = dataToPixel(data);
        if (plotPoint.isNull()) {
            // destination point outside of currently displayed range
            continue;
        }
        auto diff = plotPoint - pixel;
        unsigned int distance = diff.x() * diff.x() + diff.y() * diff.y();
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(i).x;
            closestIndex = i;
        }
    }
    closestDistance = sqrt(closestDistance);

    if(closestIndex > 0) {
        auto l1 = dataToPixel(dataAddDx(t->sample(closestIndex-1)));
        auto l2 = dataToPixel(dataAddDx(t->sample(closestIndex)));
        double ratio;
        auto distance = Util::distanceToLine(pixel, l1, l2, nullptr, &ratio);
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(closestIndex-1).x + (t->sample(closestIndex).x - t->sample(closestIndex-1).x) * ratio;
        }
    }
    if(closestIndex < t->size() - 1) {
        auto l1 = dataToPixel(dataAddDx(t->sample(closestIndex)));
        auto l2 = dataToPixel(dataAddDx(t->sample(closestIndex+1)));
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

bool TracePolarChart::dropSupported(Trace *t)
{
    if(!t->isReflection()) {
        return false;
    }
    switch(t->outputType()) {
    case Trace::DataType::Frequency:
        return true;
    default:
        return false;
    }
}

bool TracePolarChart::markerVisible(double x)
{
    if(limitToSpan) {
        if(x >= sweep_fmin && x <= sweep_fmax) {
            return true;
        } else {
            return false;
        }
    } else {
        // complete traces visible
        return true;
    }
}

bool TracePolarChart::supported(Trace *t)
{
    return dropSupported(t);
}

void TracePolarChart::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TracePolarChart::axisSetupDialog);
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

QPoint TracePolarChart::markerToPixel(Marker *m)
{
    QPoint ret = QPoint();
    if(m->getPosition() >= sweep_fmin && m->getPosition() <= sweep_fmax) {
        auto d = m->getData();
        d = dataAddDx(d);
        ret = dataToPixel(d);
    }
    return ret;
}

QString TracePolarChart::mouseText(QPoint pos)
{
    auto dataDx = pixelToData(pos);
    if(abs(dataDx) <= edgeReflection) {
        auto data = complex<double>(dataDx.real()-dx, dataDx.imag());
        auto ret = Unit::ToString(abs(data), "", " ", 3);
        ret += QString("∠");
        auto phase = atan(data.imag()/data.real())*180/M_PI;
        if (data.imag() > 0 && data.real() < 0) {
            phase += 180;
        }
        else if (data.imag() < 0 && data.real() < 0 )  {
            phase += 180;
        }
        else if (data.imag() < 0 && data.real() > 0) {
            phase += 360;
        }
        ret += Unit::ToString(phase, "", " ", 3);
        return ret;
    } else {
        return QString();
    }
}

PolarChartCircle::PolarChartCircle(QPointF center, double radius,  double startAngle, double spanAngle)
    : center(center),
      radius(radius),
      startAngle(startAngle),
      spanAngle(spanAngle)
{

}
