#include "tracesmithchart.h"
#include <QPainter>
#include <array>
#include <math.h>
#include "tracemarker.h"
#include <QDebug>
#include "preferences.h"
#include "ui_smithchartdialog.h"

using namespace std;

TraceSmithChart::TraceSmithChart(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
{
    chartLinesPen = QPen(palette().windowText(), 0.75);
    thinPen = QPen(palette().windowText(), 0.25);
    textPen = QPen(palette().windowText(), 0.25);
    pointDataPen = QPen(QColor("red"), 4.0, Qt::SolidLine, Qt::RoundCap);
    lineDataPen = QPen(QColor("blue"), 1.0);
    limitToSpan = true;
    initializeTraceInfo();
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

QPoint TraceSmithChart::plotToPixel(std::complex<double> S)
{
    return transform.map(QPoint(S.real() * smithCoordMax, -S.imag() * smithCoordMax));
}

std::complex<double> TraceSmithChart::pixelToPlot(const QPoint &pos)
{
    QPointF inv = transform.inverted().map(pos);
    return complex<double>(inv.x()/smithCoordMax, -inv.y()/smithCoordMax);
}

void TraceSmithChart::mousePressEvent(QMouseEvent *event)
{
    auto clickPoint = event->pos();
    unsigned int closestDistance = numeric_limits<unsigned int>::max();
    TraceMarker *closestMarker = nullptr;
    for(auto t : traces) {
        auto markers = t.first->getMarkers();
        for(auto m : markers) {
            if(!m->isMovable()) {
                continue;
            }
            if (limitToSpan && (m->getFrequency() < sweep_fmin || m->getFrequency() > sweep_fmax)) {
                // marker outside of currently displayed range
                continue;
            }
            auto S = m->getData();
            auto markerPoint = plotToPixel(S);
            auto yDiff = abs(markerPoint.y() - clickPoint.y());
            auto xDiff = abs(markerPoint.x() - clickPoint.x());
            unsigned int distance = xDiff * xDiff + yDiff * yDiff;
            if(distance < closestDistance) {
                closestDistance = distance;
                closestMarker = m;
            }
        }
    }
    if(closestDistance <= 400) {
        selectedMarker = closestMarker;
    } else {
        selectedMarker = nullptr;
    }
}

void TraceSmithChart::mouseMoveEvent(QMouseEvent *event)
{
    if(selectedMarker) {
        auto t = selectedMarker->trace();
        auto mouseS = pixelToPlot(event->pos());
        auto samples = t->size();
        if(!samples) {
            return;
        }
        double closestDistance = numeric_limits<double>::max();
        unsigned int closestIndex = 0;
        for(unsigned int i=0;i<samples;i++) {
            auto data = t->sample(i);
            if (limitToSpan && (data.frequency < sweep_fmin || data.frequency > sweep_fmax)) {
                // destination point outside of currently displayed range
                continue;
            }
            auto distance = norm(data.S - mouseS);
            if(distance < closestDistance) {
                closestDistance = distance;
                closestIndex = i;
            }
        }
        selectedMarker->setFrequency(t->sample(closestIndex).frequency);
    }
}

void TraceSmithChart::draw(QPainter &p) {
    p.setBrush(palette().windowText());
    auto pref = Preferences::getInstance();

    // translate coordinate system so that the smith chart sits in the origin has a size of 1
    auto w = p.window();
    p.translate(w.width()/2, w.height()/2);
    auto scale = qMin(w.height(), w.width()) / (2.0 * smithCoordMax);
    p.scale(scale, scale);

    transform = p.transform();

    // Outer circle
    auto pen = QPen(pref.General.graphColors.axis);
    pen.setCosmetic(true);
    p.setPen(pen);
    QRectF rectangle(-smithCoordMax, -smithCoordMax, 2*smithCoordMax, 2*smithCoordMax);
    p.drawArc(rectangle, 0, 5760);

    constexpr int Circles = 6;
    pen = QPen(pref.General.graphColors.divisions, 0.5, Qt::DashLine);
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
        pen = QPen(trace->color(), 1.5);
        pen.setCosmetic(true);
        p.setPen(pen);
        int nPoints = trace->size();
        for(int i=1;i<nPoints;i++) {
            auto last = trace->sample(i-1);
            auto now = trace->sample(i);
            if (limitToSpan && (last.frequency < sweep_fmin || now.frequency > sweep_fmax)) {
                continue;
            }
            if(isnan(now.S.real())) {
                break;
            }
            // scale to size of smith diagram
            last.S *= smithCoordMax;
            now.S *= smithCoordMax;
            // draw line
            p.drawLine(std::real(last.S), -std::imag(last.S), std::real(now.S), -std::imag(now.S));
        }
        if(trace->size() > 0) {
            // only draw markers if the trace has at least one point
            auto markers = t.first->getMarkers();
            for(auto m : markers) {
                if (limitToSpan && (m->getFrequency() < sweep_fmin || m->getFrequency() > sweep_fmax)) {
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
}

//void TraceSmithChart::paintEvent(QPaintEvent * /* the event */)
//{
//    auto pref = Preferences::getInstance();
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setBackground(QBrush(pref.General.graphColors.background));
//    painter.fillRect(0, 0, width(), height(), QBrush(pref.General.graphColors.background));

//    double side = qMin(width(), height()) * screenUsage;

//    //painter.setViewport((width()-side)/2, (height()-side)/2, side, side);
//    //painter.setWindow(-smithCoordMax, -smithCoordMax, 2*smithCoordMax, 2*smithCoordMax);

//    plotToPixelXOffset = width()/2;
//    plotToPixelYOffset = height()/2;
//    plotToPixelXScale = side/2;
//    plotToPixelYScale = -side/2;

//    draw(painter, 2*smithCoordMax/side);
//}

void TraceSmithChart::updateContextMenu()
{
    contextmenu->clear();
    contextmenu->clear();
    auto setup = new QAction("Setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TraceSmithChart::axisSetupDialog);
    contextmenu->addAction(setup);
    contextmenu->addSection("Traces");
    // Populate context menu
    for(auto t : traces) {
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
    if(t->isReflection()) {
        return true;
    } else {
        return false;
    }
}
