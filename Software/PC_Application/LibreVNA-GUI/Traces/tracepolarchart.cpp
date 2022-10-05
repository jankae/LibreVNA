#include "tracepolarchart.h"

#include "ui_polarchartdialog.h"
#include "preferences.h"
#include "unit.h"
#include "appwindow.h"

#include <QFileDialog>
#include <QPainter>

using namespace std;

TracePolarChart::TracePolarChart(TraceModel &model, QWidget *parent)
    : TracePolar(model, parent)
{
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

    ui->displayStartFreq->setUnit("Hz");
    ui->displayStartFreq->setPrefixes(" kMG");
    ui->displayStartFreq->setPrecision(6);
    ui->displayStartFreq->setValue(fmin);
    ui->displayStopFreq->setUnit("Hz");
    ui->displayStopFreq->setPrefixes(" kMG");
    ui->displayStopFreq->setPrecision(6);
    ui->displayStopFreq->setValue(fmax);

    connect(ui->displayFreqOverride, &QCheckBox::toggled, [=](bool active){
        ui->displayModeFreq->setEnabled(!active);
        ui->displayStartFreq->setEnabled(active);
        ui->displayStopFreq->setEnabled(active);
    });
    ui->displayFreqOverride->setChecked(manualFrequencyRange);
    emit ui->displayFreqOverride->toggled(manualFrequencyRange);

    ui->zoomReflection->setPrecision(3);
    ui->zoomFactor->setPrecision(3);
    ui->offsetRealAxis->setPrecision(3);
    ui->zoomReflection->setValue(edgeReflection);
    ui->zoomFactor->setValue(1.0/edgeReflection);
    ui->offsetRealAxis->setValue(dx);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
       limitToSpan = ui->displayModeFreq->currentIndex() == 1;
       limitToEdge = ui->displayModeRefl->currentIndex() == 1;
       manualFrequencyRange = ui->displayFreqOverride->isChecked();
       fmin = ui->displayStartFreq->value();
       fmax = ui->displayStopFreq->value();
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

void TracePolarChart::draw(QPainter &p) {
    auto& pref = Preferences::getInstance();

    p.setRenderHint(QPainter::Antialiasing);
    auto w = p.window();
    p.save();
    p.translate(w.width()/2, w.height()/2);
    auto scale = qMin(w.height(), w.width()) / (2.0 * polarCoordMax);
    p.scale(scale, scale);

    transform = p.transform();
    p.restore();

    auto minimumVisibleFrequency = std::numeric_limits<double>::lowest();
    auto maximumVisibleFrequency = std::numeric_limits<double>::max();
    if(manualFrequencyRange) {
        minimumVisibleFrequency = fmin;
        maximumVisibleFrequency = fmax;
    } else if(limitToSpan) {
        minimumVisibleFrequency = sweep_fmin;
        maximumVisibleFrequency = sweep_fmax;
    }

    auto drawArc = [&](PolarArc a) {
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
    drawArc(PolarArc(QPointF(0.0, 0.0), edgeReflection, 0, 2*M_PI));

    constexpr int Circles = 6;
    pen = QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine);
    pen.setCosmetic(true);
    p.setPen(pen);
    for(int i=1;i<Circles;i++) {
        auto radius = (double) i / Circles;
        drawArc(PolarArc(QPointF(0.0 + dx,0), radius, 0, 2*M_PI));
    }

    auto constraintLineToCircle = [&](PolarArc cir) { // PolarArc
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
        constraintLineToCircle(PolarArc(QPointF(0,0), edgeReflection, 0, angle)); // PolarArc
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
            if ((trace->getDataType() == Trace::DataType::Frequency) && (last.x < minimumVisibleFrequency || now.x > maximumVisibleFrequency)) {
                continue;
            }
            if(isnan(now.y.real())) {
                break;
            }

            last = dataAddDx(last);
            now = dataAddDx(now);

            // scale to size of smith diagram
            QPointF p1 = dataToPixel(last);
            QPointF p2 = dataToPixel(now);

            if(limitToEdge && (abs(last.y) > edgeReflection || abs(now.y) > edgeReflection)) {
                // partially outside of visible area, constrain
                if(!TracePolar::constrainLineToCircle(p1, p2, transform.map(QPointF(0,0)), polarCoordMax * scale)) {
                    // completely out of visible area
                    continue;
                }
            }

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
                if (m->getPosition() < minimumVisibleFrequency || m->getPosition() > maximumVisibleFrequency) {
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
        // adjust coords due to shifted restore
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

bool TracePolarChart::dropSupported(Trace *t)
{
    switch(t->outputType()) {
    case Trace::DataType::Frequency:
        return true;
    default:
        return false;
    }
}

bool TracePolarChart::supported(Trace *t)
{
    return dropSupported(t);
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
