#include "timegate.h"
#include <QWidget>
#include <QDialog>
#include "ui_timegatedialog.h"
#include "ui_timegateexplanationwidget.h"
#include "preferences.h"
#include <QPainter>
#include "Util/util.h"
#include "Traces/fftcomplex.h"
#include "unit.h"
#include <QMouseEvent>

Math::TimeGate::TimeGate()
{
    bandpass = true;
//    if(input->rData().size()) {
//        center = input->rData().back().x/2;
//        span = center / 2;
//    } else {
        center = 10e-9;
        span = 2e-9;
//    }
    connect(&window, &WindowFunction::changed, this, &TimeGate::updateFilter);
}

TraceMath::DataType Math::TimeGate::outputType(TraceMath::DataType inputType)
{
    if(inputType == DataType::Time) {
        return DataType::Time;
    } else {
        return DataType::Invalid;
    }
}

QString Math::TimeGate::description()
{
    QString d = "Time gate (";
    if(bandpass) {
        d.append("bandpass");
    } else {
        d.append("notch");
    }
    d.append(") center: "+Unit::ToString(center, "s", "pnum ", 3)+", span: "+Unit::ToString(span, "s", "pnum ", 3));
    d += ", window: " + window.getDescription();
    return d;
}

void Math::TimeGate::edit()
{
    if(dataType == DataType::Invalid) {
        return;
    }

    auto d = new QDialog();
    auto ui = new Ui::TimeGateDialog();
    ui->setupUi(d);
    ui->graph->setGate(this);
    ui->windowBox->setLayout(new QVBoxLayout);
    ui->windowBox->layout()->addWidget(window.createEditor());

    ui->start->setUnit("s");
    ui->start->setPrefixes("pnum ");
    ui->start->setValue(center - span / 2);

    ui->stop->setUnit("s");
    ui->stop->setPrefixes("pnum ");
    ui->stop->setValue(center + span / 2);

    ui->center->setUnit("s");
    ui->center->setPrefixes("pnum ");
    ui->center->setValue(center);

    ui->span->setUnit("s");
    ui->span->setPrefixes("pnum ");
    ui->span->setValue(center);

    if(bandpass) {
        ui->type->setCurrentIndex(0);
    } else {
        ui->type->setCurrentIndex(1);
    }

    ui->graph->setFocus();

    connect(ui->span, &SIUnitEdit::valueChanged, this, &TimeGate::setSpan);
    connect(this, &TimeGate::spanChanged, ui->span, &SIUnitEdit::setValueQuiet);
    connect(ui->center, &SIUnitEdit::valueChanged, this, &TimeGate::setCenter);
    connect(this, &TimeGate::centerChanged, ui->center, &SIUnitEdit::setValueQuiet);
    connect(ui->start, &SIUnitEdit::valueChanged, this, &TimeGate::setStart);
    connect(this, &TimeGate::startChanged, ui->start, &SIUnitEdit::setValueQuiet);
    connect(ui->stop, &SIUnitEdit::valueChanged, this, &TimeGate::setStop);
    connect(this, &TimeGate::stopChanged, ui->stop, &SIUnitEdit::setValueQuiet);

    connect(ui->type, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
        bandpass = index == 0;
        updateFilter();
    });

    connect(this, &TimeGate::outputSamplesChanged, ui->graph, qOverload<>(&QWidget::update));
    connect(this, &TimeGate::filterUpdated, ui->graph, qOverload<>(&QWidget::update));

    updateFilter();

    d->show();
}

QWidget *Math::TimeGate::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::TimeGateExplanationWidget;
    ui->setupUi(w);
    return w;
}

nlohmann::json Math::TimeGate::toJSON()
{
    nlohmann::json j;
    j["center"] = center;
    j["span"] = span;
    j["bandpass"] = bandpass;
    j["window"] = window.toJSON();
    return j;
}

void Math::TimeGate::fromJSON(nlohmann::json j)
{
    if(j.contains("window")) {
        window.fromJSON(j["window"]);
    }
    bandpass = j.value("bandpass", true);
    center = j.value("center", center);
    span = j.value("span", span);
    updateFilter();
}

void Math::TimeGate::setStart(double start)
{
    if(input && input->rData().size() > 0 && start < input->rData().front().x) {
        start = input->rData().back().x;
    }

    double stop = center + span / 2;
    if(start < stop) {
        span = stop - start;
        center = (stop + start) / 2;
        updateFilter();
    }
    emit centerChanged(center);
    emit startChanged(center - span / 2);
    emit spanChanged(span);
}

void Math::TimeGate::setStop(double stop)
{
    if(input && input->rData().size() > 0 && stop > input->rData().back().x) {
        stop = input->rData().back().x;
    }

    double start = center - span / 2;
    if(stop > start) {
        span = stop - start;
        center = (start + stop) / 2;
        updateFilter();
    }
    emit centerChanged(center);
    emit stopChanged(center + span / 2);
    emit spanChanged(span);
}

void Math::TimeGate::setCenter(double center)
{
    this->center = center;
    updateFilter();
    emit centerChanged(center);
    emit startChanged(center - span / 2);
    emit stopChanged(center + span / 2);
}

void Math::TimeGate::setSpan(double span)
{
    this->span = span;
    updateFilter();
    emit spanChanged(span);
    emit startChanged(center - span / 2);
    emit stopChanged(center + span / 2);
}

double Math::TimeGate::getStart()
{
    return center - span / 2;
}

double Math::TimeGate::getStop()
{
    return center + span / 2;
}

void Math::TimeGate::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    if(data.size() != input->rData().size()) {
        data.resize(input->rData().size());
        updateFilter();
    }
    for(auto i = begin;i<end;i++) {
        data[i] = input->rData()[i];
        data[i].y *= filter[i];
    }
    emit outputSamplesChanged(begin, end);
    if(input->rData().size() > 0) {
        success();
    } else {
        warning("No input data");
    }
}

void Math::TimeGate::updateFilter()
{
    if(!input) {
        return;
    }
    std::vector<std::complex<double>> buf;
    filter.clear();
    buf.resize(input->rData().size() * 2);
    if(!buf.size()) {
        return;
    }
    auto maxX = input->rData().back().x;
    auto minX = input->rData().front().x;

//    auto c1 = center - span / 2;
//    auto c2 = center + span / 2;

//    if(c1 < minX) {
//        c1 = minX;
//    }
//    if(c2 > maxX) {
//        c2 = maxX;
//    }
    auto wc1 = Util::Scale<double>(center - span / 2, minX, maxX, 0, 1);
    auto wc2 = Util::Scale<double>(center + span / 2, minX, maxX, 0, 1);

    // create ideal filter coefficients
    for(unsigned int i=0;i<buf.size();i++) {
        int n = i - buf.size() / 2;
        if(n == 0) {
            buf[i] = wc2 - wc1;
        } else {
            buf[i] = (sin(M_PI * wc2 * n) - sin(M_PI * wc1 * n)) / (n * M_PI);
        }
        if(!bandpass) {
            if(n == 0) {
                buf[i] = 1.0 - buf[i];
            } else {
                buf[i] = sin(M_PI * n) / M_PI - buf[i];
            }
        }
    }

    window.apply(buf);
    Fft::shift(buf, true);
    Fft::transform(buf, false);

    filter.resize(buf.size());
    for(unsigned int i=0;i<buf.size() / 2;i++) {
        filter[i] = abs(buf[i]);
    }
    emit filterUpdated();

    // needs to update output samples, pretend that input samples have changed
    inputSamplesChanged(0, input->rData().size());
}

Math::TimeGateGraph::TimeGateGraph(QWidget *parent)
    : QWidget(parent)
{
    gate = nullptr;
    grabbedStop = false;
    grabbedStart = false;
    setMouseTracking(true);
}

QPoint Math::TimeGateGraph::plotValueToPixel(double x, double y)
{
    if(!gate->getInput() || !gate->getInput()->rData().size()) {
        return QPoint(0, 0);
    }

    auto input = gate->getInput()->rData();
    auto minX = input.front().x;
    auto maxX = input.back().x;

    int plotLeft = 0;
    int plotRight = size().width();
    int plotTop = 0;
    int plotBottom = size().height();

    QPoint p;
    p.setX(Util::Scale<double>(x, minX, maxX, plotLeft, plotRight));
    p.setY(Util::Scale<double>(y, -120, 20, plotBottom, plotTop));
    return p;
}

QPointF Math::TimeGateGraph::pixelToPlotValue(QPoint p)
{
    if(!gate->getInput() || !gate->getInput()->rData().size()) {
        return QPointF(0.0, 0.0);
    }

    auto input = gate->getInput()->rData();
    auto minX = input.front().x;
    auto maxX = input.back().x;

    int plotLeft = 0;
    int plotRight = size().width();
    int plotTop = 0;
    int plotBottom = size().height();

    QPointF ret;
    ret.setX(Util::Scale<double>(p.x(), plotLeft, plotRight, minX, maxX));
    ret.setY(Util::Scale<double>(p.y(), plotBottom, plotTop, -120, 20));
    return ret;
}

void Math::TimeGateGraph::paintEvent(QPaintEvent *event)
{
    if(!gate) {
        return;
    }
    // grab input data
    auto input = gate->getInput()->rData();

    Q_UNUSED(event)
    auto pref = Preferences::getInstance();
    QPainter p(this);
    // fill background
    p.setBackground(QBrush(pref.General.graphColors.background));
    p.fillRect(0, 0, width(), height(), QBrush(pref.General.graphColors.background));

    // plot trace
    auto pen = QPen(Qt::green, 1);
    pen.setCosmetic(true);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    for(unsigned int i=1;i<input.size();i++) {
        auto last = input[i-1];
        auto now = input[i];

        auto y_last = Unit::dB(last.y);
        auto y_now = Unit::dB(now.y);

        if(std::isnan(y_last) || std::isnan(y_now) || std::isinf(y_last) || std::isinf(y_now)) {
            continue;
        }

        // scale to plot coordinates
        auto p1 = plotValueToPixel(last.x, y_last);
        auto p2 = plotValueToPixel(now.x, y_now);
        // draw line
        p.drawLine(p1, p2);
    }
    // plot filter shape
    auto filter = gate->rFilter();
    pen = QPen(Qt::red, 1);
    p.setPen(pen);
    for(unsigned int i=1;i<filter.size();i++) {
        auto x_last = input[i-1].x;
        auto x_now = input[i].x;

        auto f_last = Unit::dB(filter[i-1]);
        auto f_now = Unit::dB(filter[i]);

        if(std::isnan(f_last) || std::isnan(f_now) || std::isinf(f_last) || std::isinf(f_now)) {
            continue;
        }

        // scale to plot coordinates
        auto p1 = plotValueToPixel(x_last, f_last);
        auto p2 = plotValueToPixel(x_now, f_now);

        // draw line
        p.drawLine(p1, p2);
    }
}

void Math::TimeGateGraph::mousePressEvent(QMouseEvent *event)
{
    if(!gate) {
        return;
    }
    grabbedStop = false;
    grabbedStart = false;
    auto startX = plotValueToPixel(gate->getStart(), 0).x();
    auto stopX = plotValueToPixel(gate->getStop(), 0).x();
    auto distStart = abs(event->pos().x() - startX);
    auto distStop = abs(event->pos().x() - stopX);
    if(distStart < distStop && distStart < catchDistance) {
        grabbedStart = true;
    } else if(distStop < distStart && distStop < catchDistance) {
        grabbedStop = true;
    } else if(distStop == distStart && distStop < catchDistance) {
        // could happen if start/stop are close to each with respect to input range
        if(event->pos().x() > stopX) {
            grabbedStop = true;
        } else {
            grabbedStart = true;
        }
    }
}

void Math::TimeGateGraph::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    grabbedStop = false;
    grabbedStart = false;
}

void Math::TimeGateGraph::mouseMoveEvent(QMouseEvent *event)
{
    if(!gate) {
        return;
    }
    auto value = pixelToPlotValue(event->pos());
    if(grabbedStop) {
        gate->setStop(value.x());
    } else if(grabbedStart) {
        gate->setStart(value.x());
    } else {
        // nothing grabbed but might be above start/stop -> check and change cursor
        auto startX = plotValueToPixel(gate->getStart(), 0).x();
        auto stopX = plotValueToPixel(gate->getStop(), 0).x();
        if(abs(event->pos().x() - startX) < catchDistance || abs(event->pos().x() - stopX) < catchDistance) {
            setCursor(Qt::SizeHorCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}
