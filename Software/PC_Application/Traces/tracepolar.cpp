#include "tracepolar.h"

#include "Marker/marker.h"
#include "Util/util.h"

#include <QFileDialog>

using namespace std;

TracePolar::TracePolar(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
{
    limitToSpan = true;
    limitToEdge = true;
    edgeReflection = 1.0;
    initializeTraceInfo();
}

nlohmann::json TracePolar::toJSON()
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

void TracePolar::fromJSON(nlohmann::json j)
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

void TracePolar::wheelEvent(QWheelEvent *event)
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

QPoint TracePolar::dataToPixel(std::complex<double> d)
{
    return transform.map(QPoint(d.real() * polarCoordMax * (1.0 / edgeReflection), -d.imag() * polarCoordMax * (1.0 / edgeReflection)));
}

QPoint TracePolar::dataToPixel(Trace::Data d)
{
    return dataToPixel(d.y);
}

std::complex<double> TracePolar::dataAddDx(std::complex<double> d)
{
    auto dataShift = complex<double>(dx, 0);
    d = d + dataShift;
    return d;
}

Trace::Data TracePolar::dataAddDx(Trace::Data d)
{
    d.y = dataAddDx(d.y);
    return d;
}

std::complex<double> TracePolar::pixelToData(QPoint p)
{
    auto data = transform.inverted().map(QPointF(p));
    return complex<double>(data.x() / polarCoordMax * edgeReflection, -data.y() / polarCoordMax * edgeReflection);
}

QPoint TracePolar::markerToPixel(Marker *m)
{
    QPoint ret = QPoint();
//    if(!m->isTimeDomain()) {
        if(m->getPosition() >= sweep_fmin && m->getPosition() <= sweep_fmax) {
            auto d = m->getData();
            d = dataAddDx(d);
            ret = dataToPixel(d);
        }
//    }
    return ret;
}

double TracePolar::nearestTracePoint(Trace *t, QPoint pixel, double *distance)
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

bool TracePolar::markerVisible(double x)
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

void TracePolar::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TracePolar::axisSetupDialog, Qt::UniqueConnection);
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

PolarArc::PolarArc(QPointF center, double radius,  double startAngle, double spanAngle)
    : center(center),
      radius(radius),
      startAngle(startAngle),
      spanAngle(spanAngle)
{

}

void PolarArc::constrainToCircle(QPointF center, double radius)
{
    // check if arc/circle intersect
    auto centerDiff = this->center - center;
    auto centerDistSquared = centerDiff.x() * centerDiff.x() + centerDiff.y() * centerDiff.y();
    if (centerDistSquared >= (radius + this->radius) * (radius + this->radius)) {
        // arc completely outside of constraining circle
        spanAngle = 0.0;
        return;
    } else if (centerDistSquared <= (radius - this->radius) * (radius - this->radius)) {
        if (radius >= this->radius) {
            // arc completely in constraining circle, do nothing
            return;
        } else {
            // arc completely outside of circle
            spanAngle = 0.0;
            return;
        }
    } else {
        // there are intersections between the arc and the circle. Calculate points according to https://stackoverflow.com/questions/3349125/circle-circle-intersection-points
        auto distance = sqrt(centerDistSquared);
        auto a = (this->radius*this->radius-radius*radius+distance*distance) / (2*distance);
        auto h = sqrt(this->radius*this->radius - a*a);
        auto intersectMiddle = this->center + a*(center - this->center) / distance;
        auto rotatedCenterDiff = center - this->center;
        swap(rotatedCenterDiff.rx(), rotatedCenterDiff.ry());
        rotatedCenterDiff.setY(-rotatedCenterDiff.y());
        auto intersect1 = intersectMiddle + h * rotatedCenterDiff / distance;
        auto intersect2 = intersectMiddle - h * rotatedCenterDiff / distance;

        // got intersection points, convert into angles from arc center
        auto wrapAngle = [](double angle) -> double {
            double ret = fmod(angle, 2*M_PI);
            if(ret < 0) {
                ret += 2*M_PI;
            }
            return ret;
        };

        auto angle1 = wrapAngle(atan2((intersect1 - this->center).y(), (intersect1 - this->center).x()));
        auto angle2 = wrapAngle(atan2((intersect2 - this->center).y(), (intersect2 - this->center).x()));

        auto angleDiff = wrapAngle(angle2 - angle1);
        if ((angleDiff >= M_PI) ^ (a > 0.0)) {
            // allowed angles go from intersect1 to intersect2
            if(startAngle < angle1) {
                startAngle = angle1;
            }
            auto maxSpan = wrapAngle(angle2 - startAngle);
            if(spanAngle > maxSpan) {
                spanAngle = maxSpan;
            }
        } else {
            // allowed angles go from intersect2 to intersect1
            if(startAngle < angle2) {
                startAngle = angle2;
            }
            auto maxSpan = wrapAngle(angle1 - startAngle);
            if(spanAngle > maxSpan) {
                spanAngle = maxSpan;
            }
        }
    }
}

