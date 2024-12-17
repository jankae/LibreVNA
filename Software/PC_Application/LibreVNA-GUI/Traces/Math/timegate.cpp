#include "timegate.h"

#include "ui_timegatedialog.h"
#include "ui_timegateexplanationwidget.h"
#include "preferences.h"
#include "Util/util.h"
#include "Traces/fftcomplex.h"
#include "Util/util.h"
#include "unit.h"
#include "appwindow.h"

#include <QWidget>
#include <QDialog>
#include <QPainter>
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
    connect(d, &QDialog::finished, [=](){
        delete ui;
    });
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

    if(AppWindow::showGUI()) {
        d->show();
    }
}

QWidget *Math::TimeGate::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::TimeGateExplanationWidget;
    ui->setupUi(w);
    connect(w, &QWidget::destroyed, [=](){
        delete ui;
    });
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
    if(!input) {
        return;
    }
    auto inputData = input->getData();
    if(inputData.size() > 0 && start < inputData.front().x) {
        start = inputData.back().x;
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
    if(!input) {
        return;
    }
    auto inputData = input->getData();
    if(inputData.size() > 0 && stop > inputData.back().x) {
        stop = inputData.back().x;
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
    std::vector<Data> inputData;
    if(input) {
        inputData = input->getData();
    }
    if(data.size() != inputData.size()) {
        dataMutex.lock();
        data.resize(inputData.size());
        dataMutex.unlock();
        updateFilter();
    }
    dataMutex.lock();
    for(auto i = begin;i<end;i++) {
        data[i] = inputData[i];
        data[i].y *= filter[i];
    }
    dataMutex.unlock();
    emit outputSamplesChanged(begin, end);
    if(inputData.size() > 0) {
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
    auto inputData = input->getData();
    std::vector<std::complex<double>> buf;
    filter.clear();
    buf.resize(inputData.size() * 2);
    if(!buf.size()) {
        return;
    }
    auto maxX = inputData.back().x;
    auto minX = inputData.front().x;

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

    filter.resize(buf.size() / 2);
    for(unsigned int i=0;i<buf.size() / 2;i++) {
        filter[i] = abs(buf[i]);
    }
    emit filterUpdated();

    // needs to update output samples, pretend that input samples have changed
    inputSamplesChanged(0, inputData.size());
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
    if(!gate->getInput()) {
        return QPoint(0, 0);
    }

    auto inputData = gate->getInput()->getData();

    if(!inputData.size()) {
        return QPoint(0, 0);
    }

    auto minX = inputData.front().x;
    auto maxX = inputData.back().x;

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
    if(!gate->getInput()) {
        return QPointF(0.0, 0.0);
    }

    auto inputData = gate->getInput()->getData();

    if(!inputData.size()) {
        return QPointF(0.0, 0.0);
    }

    auto minX = inputData.front().x;
    auto maxX = inputData.back().x;

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
    if(!gate || !gate->getInput()) {
        return;
    }
    // grab input data
    auto inputData = gate->getInput()->getData();

    Q_UNUSED(event)
    auto& pref = Preferences::getInstance();
    QPainter p(this);
    // fill background
    p.setBackground(QBrush(pref.Graphs.Color.background));
    p.fillRect(0, 0, width(), height(), QBrush(pref.Graphs.Color.background));

    if(!inputData.size()) {
        // no data yet, nothing to plot
        return;
    }

    // plot trace
    auto pen = QPen(Qt::green, 1);
    pen.setCosmetic(true);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);

    auto minX = inputData.front().x;
    auto maxX = inputData.back().x;

    int plotLeft = 0;
    int plotRight = size().width();
    int plotTop = 0;
    int plotBottom = size().height();

    QPoint p1, p2;

    // limit amount of displayed points to keep GUI snappy
    auto increment = inputData.size() / 500;
    if(!increment) {
        increment = 1;
    }

    for(unsigned int i=increment;i<inputData.size();i+=increment) {
        auto last = inputData[i-increment];
        auto now = inputData[i];

        auto y_last = Util::SparamTodB(last.y);
        auto y_now = Util::SparamTodB(now.y);

        if(std::isnan(y_last) || std::isnan(y_now) || std::isinf(y_last) || std::isinf(y_now)) {
            continue;
        }

        // scale to plot coordinates
        p1.setX(Util::Scale<double>(last.x, minX, maxX, plotLeft, plotRight));
        p1.setY(Util::Scale<double>(y_last, -120, 20, plotBottom, plotTop));
        p2.setX(Util::Scale<double>(now.x, minX, maxX, plotLeft, plotRight));
        p2.setY(Util::Scale<double>(y_now, -120, 20, plotBottom, plotTop));
        // draw line
        p.drawLine(p1, p2);
    }
    // plot filter shape
    auto filter = gate->rFilter();
    pen = QPen(Qt::red, 1);
    p.setPen(pen);
    for(unsigned int i=increment;i<filter.size() && i<inputData.size();i+=increment) {
        auto x_last = inputData[i-increment].x;
        auto x_now = inputData[i].x;

        auto f_last = Util::SparamTodB(filter[i-increment]);
        auto f_now = Util::SparamTodB(filter[i]);

        if(std::isnan(f_last) || std::isnan(f_now) || std::isinf(f_last) || std::isinf(f_now)) {
            continue;
        }

        // scale to plot coordinates
        p1.setX(Util::Scale<double>(x_last, minX, maxX, plotLeft, plotRight));
        p1.setY(Util::Scale<double>(f_last, -120, 20, plotBottom, plotTop));
        p2.setX(Util::Scale<double>(x_now, minX, maxX, plotLeft, plotRight));
        p2.setY(Util::Scale<double>(f_now, -120, 20, plotBottom, plotTop));
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
