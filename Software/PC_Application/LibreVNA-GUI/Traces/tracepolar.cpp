#include "tracepolar.h"

#include "Marker/marker.h"
#include "Util/util.h"
#include "preferences.h"

#include <QFileDialog>

using namespace std;

TracePolar::TracePolar(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
{
    limitToSpan = true;
    limitToEdge = true;
    manualFrequencyRange = false;
    fmin = 0;
    fmax = 6000000000;
    edgeReflection = 1.0;
    offset = QPointF(0.0, 0.0);
    initializeTraceInfo();
}

nlohmann::json TracePolar::toJSON()
{
    nlohmann::json j = getBaseJSON();
    j["limit_to_span"] = limitToSpan;
    j["limit_to_edge"] = limitToEdge;
    j["edge_reflection"] = edgeReflection;
    j["offset_axis_x"] = offset.x();
    j["offset_axis_y"] = offset.y();
    j["frequency_override"] = manualFrequencyRange;
    j["override_min"] = fmin;
    j["override_max"] = fmax;
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
    parseBaseJSON(j);
    limitToSpan = j.value("limit_to_span", true);
    limitToEdge = j.value("limit_to_edge", false);
    edgeReflection = j.value("edge_reflection", 1.0);
    manualFrequencyRange = j.value("frequency_override", false);
    fmin = j.value("override_min", 0.0);
    fmax = j.value("override_max", 6000000000.0);
    offset = QPointF(j.value("offset_axis_x", 0.0), j.value("offset_axis_y", 0.0));
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

void TracePolar::move(const QPoint &vect)
{
    Trace::Data center;
    center.y = 0.0;
    center.x = 0.0;
    auto shift = pixelToData(dataToPixel(center) + vect);
    offset.rx() += shift.real();
    offset.ry() += shift.imag();
    replot();
}

void TracePolar::zoom(const QPoint &center, double factor, bool horizontally, bool vertically)
{
    Q_UNUSED(horizontally);
    Q_UNUSED(vertically);
    auto pos = pixelToData(center);
    auto shift = QPointF(pos.real(), pos.imag());
    offset -= shift;
    edgeReflection *= factor;
    offset += shift * factor;
    replot();
}

void TracePolar::setAuto(bool horizontally, bool vertically)
{
    Q_UNUSED(horizontally);
    Q_UNUSED(vertically);
    edgeReflection = 1.0;
    offset = QPointF(0.0, 0.0);
    replot();
}

//void TracePolar::wheelEvent(QWheelEvent *event)
//{
//    // most mousewheel have 15 degree increments, the reported delta is in 1/8th degree -> 120
//    auto increment = event->angleDelta().y() / 120.0;
//    // round toward bigger step in case of special higher resolution mousewheel
//    int steps = increment > 0 ? ceil(increment) : floor(increment);

//    constexpr double zoomfactor = 1.1;
//    auto zoom = pow(zoomfactor, steps);
//    edgeReflection /= zoom;

//    auto incrementX = event->angleDelta().x() / 120.0;
////    dx += incrementX/10;

//    triggerReplot();
//}

bool TracePolar::positionWithinGraphArea(const QPoint &p)
{
    auto coord = pixelToData(p);
    if(abs(coord) <= edgeReflection) {
        return true;
    } else {
        return false;
    }
}

QPoint TracePolar::dataToPixel(std::complex<double> d)
{
    return transform.map(QPoint(d.real() * polarCoordMax * (1.0 / edgeReflection), -d.imag() * polarCoordMax * (1.0 / edgeReflection)));
}

QPoint TracePolar:: dataToPixel(Trace::Data d)
{
    return dataToPixel(d.y);
}

std::complex<double> TracePolar::dataAddOffset(std::complex<double> d)
{
    auto dataShift = complex<double>(offset.x(), offset.y());
    d = d + dataShift;
    return d;
}

Trace::Data TracePolar::dataAddOffset(Trace::Data d)
{
    d.y = dataAddOffset(d.y);
    return d;
}

QPoint TracePolar::dataToPixel(QPointF d)
{
    return dataToPixel(complex<double>(d.x(), d.y()));
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
            d = dataAddOffset(d);
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
        if(data.x < minimumVisibleFrequency() || data.x > maximumVisibleFrequency()) {
            // outside of displayed range
            continue;
        }
        data = dataAddOffset(data);
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

    if(closestIndex > 0 && t->sample(closestIndex-1).x >= minimumVisibleFrequency()) {
        auto l1 = dataToPixel(dataAddOffset(t->sample(closestIndex-1)));
        auto l2 = dataToPixel(dataAddOffset(t->sample(closestIndex)));
        double ratio;
        auto distance = Util::distanceToLine(pixel, l1, l2, nullptr, &ratio);
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(closestIndex-1).x + (t->sample(closestIndex).x - t->sample(closestIndex-1).x) * ratio;
        }
    }
    if(closestIndex < t->size() - 1 && t->sample(closestIndex+1).x <= maximumVisibleFrequency()) {
        auto l1 = dataToPixel(dataAddOffset(t->sample(closestIndex)));
        auto l2 = dataToPixel(dataAddOffset(t->sample(closestIndex+1)));
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

double TracePolar::minimumVisibleFrequency()
{
    if(manualFrequencyRange) {
        return fmin;
    } else if(limitToSpan) {
        return sweep_fmin;
    } else {
        return std::numeric_limits<double>::lowest();
    }
}

double TracePolar::maximumVisibleFrequency()
{
    if(manualFrequencyRange) {
        return fmax;
    } else if(limitToSpan) {
        return sweep_fmax;
    } else {
        return std::numeric_limits<double>::max();
    }
}

bool TracePolar::constrainLineToCircle(QPointF &a, QPointF &b, QPointF center, double radius)
{
    auto distance = [](const QPointF &a, const QPointF &b) {
        auto dx = b.x() - a.x();
        auto dy = b.y() - a.y();
        return sqrt(dx*dx + dy*dy);
    };

    if(distance(a, center) <= radius && distance(b, center) <= radius) {
        // both points are completely contained within the circle, no adjustment necessary
        return true;
    }

    // shift points, the formulas assume center = (0,0)
    a -= center;
    b -= center;

    // according to https://mathworld.wolfram.com/Circle-LineIntersection.html
    auto dx = b.x() - a.x();
    auto dy = b.y() - a.y();
    auto dr = sqrt(dx*dx+dy*dy);
    auto D = a.x()*b.y() - b.x()*a.y();
    // check intersection
    auto delta = radius*radius * dr*dr - D*D;
    if(delta <= 0) {
        // line does not intersect the circle
        return false;
    }
    // line intersects the circle, calculate intersection points
    auto x1 = (D*dy+copysign(1.0, dy) * dx*sqrt(delta)) / (dr*dr);
    auto x2 = (D*dy-copysign(1.0, dy) * dx*sqrt(delta)) / (dr*dr);
    auto y1 = (-D*dx+abs(dy)*sqrt(delta)) / (dr*dr);
    auto y2 = (-D*dx-abs(dy)*sqrt(delta)) / (dr*dr);

    auto inter1 = QPointF(x1, y1);
    auto inter2 = QPointF(x2, y2);

    bool inter1betweenPoints = false;
    bool inter2betweenPoints = false;
    if(abs(distance(a, inter1) + distance(b, inter1) - distance(a, b)) < 0.000001) {
        inter1betweenPoints = true;
    }
    if(abs(distance(a, inter2) + distance(b, inter2) - distance(a, b)) < 0.000001) {
        inter2betweenPoints = true;
    }
    if(inter1betweenPoints && inter2betweenPoints) {
        // adjust both points, order does not matter
        a = inter1;
        b = inter2;
    } else if(!inter1betweenPoints && !inter2betweenPoints) {
        // the line intersect the circle but outside of the segment defined by the points -> ignore
        a += center;
        b += center;
        return false;
    } else {
        // exactly one intersection point must lie between the two line points, otherwise we would have returned already
        auto inter = inter1betweenPoints ? inter1 : inter2;
        if(distance(a, QPointF(0,0)) < radius) {
            // point is in the circle and can remain unchanged. Use inter as new point b
            b = inter;
        } else {
            // the other way around
            a = inter;
        }
    }
    a += center;
    b += center;
    return true;
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

