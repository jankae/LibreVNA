#include "tracemarker.h"
#include <QPainter>

TraceMarker::TraceMarker()
    : parentTrace(nullptr),
      frequency(1000000000),
      number(1),
      data(0)
{

}

TraceMarker::~TraceMarker()
{
    if(parentTrace) {
        parentTrace->removeMarker(this);
    }
    emit deleted(this);
}

void TraceMarker::assignTrace(Trace *t)
{
    if(parentTrace) {
        // remove connection from previous parent trace
        parentTrace->removeMarker(this);
        disconnect(parentTrace, &Trace::deleted, this, &TraceMarker::parentTraceDeleted);
        disconnect(parentTrace, &Trace::dataChanged, this, &TraceMarker::traceDataChanged);
        disconnect(parentTrace, &Trace::colorChanged, this, &TraceMarker::updateSymbol);
    }
    parentTrace = t;
    connect(parentTrace, &Trace::deleted, this, &TraceMarker::parentTraceDeleted);
    connect(parentTrace, &Trace::dataChanged, this, &TraceMarker::traceDataChanged);
    connect(parentTrace, &Trace::colorChanged, this, &TraceMarker::updateSymbol);
    constrainFrequency();
    updateSymbol();
    parentTrace->addMarker(this);
}

Trace *TraceMarker::trace()
{
    return parentTrace;
}

QString TraceMarker::readableData()
{
    auto db = 20*log10(abs(data));
    auto phase = arg(data);
    return QString::number(db, 'g', 4) + "db@" + QString::number(phase*180/M_PI, 'g', 4);
}

void TraceMarker::setFrequency(double freq)
{
    frequency = freq;
    constrainFrequency();
}

void TraceMarker::parentTraceDeleted(Trace *t)
{
    if(t == parentTrace) {
        delete this;
    }
}

void TraceMarker::traceDataChanged()
{
    // some data of the parent trace changed, check if marker data also changed
    auto tracedata = parentTrace->getData(frequency);
    if(tracedata != data) {
        data = tracedata;
        emit dataChanged(this);
    }
}

void TraceMarker::updateSymbol()
{
    constexpr int width = 15, height = 15;
    symbol = QPixmap(width, height);
    symbol.fill(Qt::transparent);
    QPainter p(&symbol);
    p.setRenderHint(QPainter::Antialiasing);
    QPointF points[] = {QPointF(0,0),QPointF(width,0),QPointF(width/2,height)};
    auto traceColor = parentTrace->color();
    p.setPen(traceColor);
    p.setBrush(traceColor);
    p.drawConvexPolygon(points, 3);
    auto brightness = traceColor.redF() * 0.299 + traceColor.greenF() * 0.587 + traceColor.blueF() * 0.114;
    p.setPen((brightness > 0.6) ? Qt::black : Qt::white);
    p.drawText(QRectF(0,0,width, height*2.0/3.0), Qt::AlignCenter, QString::number(number));
}

void TraceMarker::constrainFrequency()
{
    if(parentTrace && parentTrace->size() > 0)  {
        if(frequency > parentTrace->maxFreq()) {
            frequency = parentTrace->maxFreq();
        } else if(frequency < parentTrace->minFreq()) {
            frequency = parentTrace->minFreq();
        }
        traceDataChanged();
    }
}

int TraceMarker::getNumber() const
{
    return number;
}

std::complex<double> TraceMarker::getData() const
{
    return data;
}

QPixmap &TraceMarker::getSymbol()
{
    return symbol;
}

double TraceMarker::getFrequency() const
{
    return frequency;
}

