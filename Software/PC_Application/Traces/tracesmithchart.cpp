#include "tracesmithchart.h"
#include <QPainter>
#include <array>
#include <math.h>
#include "Marker/marker.h"
#include <QDebug>
#include "preferences.h"
#include "ui_smithchartdialog.h"
#include "unit.h"
#include "QFileDialog"

using namespace std;

TraceSmithChart::TraceSmithChart(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
{
    limitToSpan = true;
    initializeTraceInfo();
}

nlohmann::json TraceSmithChart::toJSON()
{
    nlohmann::json j;
    j["limit_to_span"] = limitToSpan;
    nlohmann::json jtraces;
    for(auto t : traces) {
        if(t.second) {
            jtraces.push_back(t.first->toHash());
        }
    }
    j["traces"] = jtraces;
    return j;
}

void TraceSmithChart::fromJSON(nlohmann::json j)
{
    limitToSpan = j.value("limit_to_span", true);
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

void TraceSmithChart::axisSetupDialog()
{
    auto dialog = new QDialog();
    auto ui = new Ui::SmithChartDialog();
    ui->setupUi(dialog);
    if(limitToSpan) {
        ui->displayMode->setCurrentIndex(1);
    } else {
        ui->displayMode->setCurrentIndex(0);
    }
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
       limitToSpan = ui->displayMode->currentIndex() == 1;
       triggerReplot();
    });
    dialog->show();
}

QPoint TraceSmithChart::dataToPixel(Trace::Data d)
{
    if(d.x < sweep_fmin || d.x > sweep_fmax) {
        return QPoint();
    }
    return transform.map(QPoint(d.y.real() * smithCoordMax, -d.y.imag() * smithCoordMax));
}

std::complex<double> TraceSmithChart::pixelToData(QPoint p)
{
    auto data = transform.inverted().map(QPointF(p));
    return complex<double>(data.x() / smithCoordMax, -data.y() / smithCoordMax);
}

QPoint TraceSmithChart::markerToPixel(Marker *m)
{
    QPoint ret = QPoint();
//    if(!m->isTimeDomain()) {
        if(m->getPosition() >= sweep_fmin && m->getPosition() <= sweep_fmax) {
            auto d = m->getData();
            ret = transform.map(QPoint(d.real() * smithCoordMax, -d.imag() * smithCoordMax));
        }
//    }
    return ret;
}

double TraceSmithChart::nearestTracePoint(Trace *t, QPoint pixel, double *distance)
{
    double closestDistance = numeric_limits<double>::max();
    unsigned int closestIndex = 0;
    for(unsigned int i=0;i<t->size();i++) {
        auto data = t->sample(i);
        auto plotPoint = dataToPixel(data);
        if (plotPoint.isNull()) {
            // destination point outside of currently displayed range
            continue;
        }
        auto diff = plotPoint - pixel;
        unsigned int distance = diff.x() * diff.x() + diff.y() * diff.y();
        if(distance < closestDistance) {
            closestDistance = distance;
            closestIndex = i;
        }
    }
    if(distance) {
        *distance = closestDistance;
    }
    return t->sample(closestIndex).x;
}

bool TraceSmithChart::xCoordinateVisible(double x)
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

void TraceSmithChart::draw(QPainter &p) {
    auto pref = Preferences::getInstance();

    // translate coordinate system so that the smith chart sits in the origin has a size of 1
    auto w = p.window();
    p.save();
    p.translate(w.width()/2, w.height()/2);
    auto scale = qMin(w.height(), w.width()) / (2.0 * smithCoordMax);
    p.scale(scale, scale);

    transform = p.transform();

    // Outer circle
    auto pen = QPen(pref.Graphs.Color.axis);
    pen.setCosmetic(true);
    p.setPen(pen);
    QRectF rectangle(-smithCoordMax, -smithCoordMax, 2*smithCoordMax, 2*smithCoordMax);
    p.drawArc(rectangle, 0, 5760);

    constexpr int Circles = 6;
    pen = QPen(pref.Graphs.Color.divisions, 0.5, Qt::DashLine);
    pen.setCosmetic(true);
    p.setPen(pen);
    for(int i=1;i<Circles;i++) {
        rectangle.adjust(2.0*smithCoordMax/Circles, smithCoordMax/Circles, 0, -smithCoordMax/Circles);
        p.drawArc(rectangle, 0, 5760);
    }

    p.drawLine(-smithCoordMax, 0, smithCoordMax, 0);
    constexpr std::array<double, 5> impedanceLines = {10, 25, 50, 100, 250};
    for(auto z : impedanceLines) {
        z /= ReferenceImpedance;
        auto radius = smithCoordMax/z;
        double span = M_PI - 2 * atan(radius/smithCoordMax);
        span *= 5760 / (2 * M_PI);
        QRectF rectangle(smithCoordMax - radius, -2 * radius, 2 * radius, 2 * radius);
        p.drawArc(rectangle, 4320 - span, span);
        rectangle = QRectF(smithCoordMax - radius, 0, 2 * radius, 2 * radius);
        p.drawArc(rectangle, 1440, span);
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
        pen = QPen(trace->color(), 1);
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
            // scale to size of smith diagram
            last.y *= smithCoordMax;
            now.y *= smithCoordMax;
            // draw line
            p.drawLine(std::real(last.y), -std::imag(last.y), std::real(now.y), -std::imag(now.y));
        }
        if(trace->size() > 0) {
            // only draw markers if the trace has at least one point
            auto markers = t.first->getMarkers();
            for(auto m : markers) {
//                if (m->isTimeDomain()) {
//                    continue;
//                }
                if (limitToSpan && (m->getPosition() < sweep_fmin || m->getPosition() > sweep_fmax)) {
                    continue;
                }
                if(m->getPosition() < trace->minX() || m->getPosition() > trace->maxX()) {
                    // marker not in trace range
                    continue;
                }
                auto coords = m->getData();
                coords *= smithCoordMax;
                auto symbol = m->getSymbol();
                symbol = symbol.scaled(symbol.width()/scale, symbol.height()/scale);
                p.drawPixmap(coords.real() - symbol.width()/2, -coords.imag() - symbol.height(), symbol);
            }
        }
    }
    if(dropPending) {
        p.setOpacity(0.5);
        p.setBrush(Qt::white);
        p.setPen(Qt::white);
        p.drawEllipse(-smithCoordMax, -smithCoordMax, 2*smithCoordMax, 2*smithCoordMax);
        p.restore();
        auto font = p.font();
        font.setPixelSize(20);
        p.setFont(font);
        p.setOpacity(1.0);
        p.setPen(Qt::white);
        auto text = "Drop here to add\n" + dropTrace->name() + "\nto Smith chart";
        p.drawText(p.window(), Qt::AlignCenter, text);
    } else {
        p.restore();
    }
}

void TraceSmithChart::traceDropped(Trace *t, QPoint position)
{
    Q_UNUSED(t)
    Q_UNUSED(position);
    if(supported(t)) {
        enableTrace(t, true);
    }
}

QString TraceSmithChart::mouseText(QPoint pos)
{
    auto data = pixelToData(pos);
    if(abs(data) <= 1.0) {
        data = 50.0 * (1-.0 + data) / (1.0 - data);
        auto ret = Unit::ToString(data.real(), "", " ", 3);
        if(data.imag() >= 0) {
            ret += "+";
        }
        ret += Unit::ToString(data.imag(), "j", " ", 3);
        return ret;
    } else {
        return QString();
    }
}

void TraceSmithChart::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TraceSmithChart::axisSetupDialog);
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

bool TraceSmithChart::supported(Trace *t)
{
    return dropSupported(t);
}

bool TraceSmithChart::dropSupported(Trace *t)
{
    if(!t->isReflection()) {
        return false;
    }
    switch(t->outputType()) {
    case Trace::DataType::Frequency:
    case Trace::DataType::Power:
        return true;
    default:
        return false;
    }
}
