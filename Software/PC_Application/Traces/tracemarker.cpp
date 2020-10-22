#include "tracemarker.h"
#include <QPainter>
#include "CustomWidgets/siunitedit.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include "tracemarkermodel.h"
#include "unit.h"

TraceMarker::TraceMarker(TraceMarkerModel *model, int number)
    : editingFrequeny(false),
      model(model),
      parentTrace(nullptr),
      frequency(1000000000),
      number(number),
      data(0),
      type(Type::Manual),
      delta(nullptr),
      cutoffAmplitude(-3.0)
{

}

TraceMarker::~TraceMarker()
{
    if(parentTrace) {
        parentTrace->removeMarker(this);
    }
    deleteHelperMarkers();
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
    for(auto m : helperMarkers) {
        m->assignTrace(t);
    }
    update();
}

Trace *TraceMarker::trace()
{
    return parentTrace;
}

QString TraceMarker::readableData()
{
    switch(type) {
    case Type::Manual:
    case Type::Maximum:
    case Type::Minimum: {
        auto db = 20*log10(abs(data));
        auto phase = arg(data);
        return QString::number(db, 'g', 4) + "db@" + QString::number(phase*180/M_PI, 'g', 4);
    }
    case Type::Delta:
        if(!delta) {
            return "Invalid delta marker";
        } else {
            // calculate difference between markers
            auto freqDiff = frequency - delta->frequency;
            auto valueDiff = data / delta->data;
            auto db = 20*log10(abs(valueDiff));
            auto phase = arg(valueDiff);
            return Unit::ToString(freqDiff, "Hz", " kMG") + " / " + QString::number(db, 'g', 4) + "db@" + QString::number(phase*180/M_PI, 'g', 4);
        }
    case Type::Lowpass:
    case Type::Highpass:
        if(parentTrace->isReflection()) {
            return "Calculation not possible with reflection measurement";
        } else {
            auto insertionLoss = 20*log10(abs(data));
            auto cutoff = 20*log10(abs(helperMarkers[0]->data));
            QString ret = "fc: ";
            if(cutoff > insertionLoss + cutoffAmplitude) {
                // the trace never dipped below the specified cutoffAmplitude, exact cutoff frequency unknown
                ret += type == Type::Lowpass ? ">" : "<";
            }
            ret += Unit::ToString(helperMarkers[0]->frequency, "Hz", " kMG", 4);
            ret += ", Ins.Loss: >=" + QString::number(-insertionLoss, 'g', 4) + "db";
            return ret;
        }
        break;
    case Type::Bandpass:
        if(parentTrace->isReflection()) {
            return "Calculation not possible with reflection measurement";
        } else {
            auto insertionLoss = 20*log10(abs(data));
            auto cutoffL = 20*log10(abs(helperMarkers[0]->data));
            auto cutoffH = 20*log10(abs(helperMarkers[1]->data));
            auto bandwidth = helperMarkers[1]->frequency - helperMarkers[0]->frequency;
            auto center = helperMarkers[2]->frequency;
            QString ret = "fc: ";
            if(cutoffL > insertionLoss + cutoffAmplitude || cutoffH > insertionLoss + cutoffAmplitude) {
                // the trace never dipped below the specified cutoffAmplitude, center and exact bandwidth unknown
                ret += "?, BW: >";
            } else {
                ret += Unit::ToString(center, "Hz", " kMG", 5)+ ", BW: ";
            }
            ret += Unit::ToString(bandwidth, "Hz", " kMG", 4);
            ret += ", Ins.Loss: >=" + QString::number(-insertionLoss, 'g', 4) + "db";
            return ret;
        }
        break;
    default:
        return "Unknown marker type";
    }
}

QString TraceMarker::readableSettings()
{
    switch(type) {
    case Type::Manual:
    case Type::Maximum:
    case Type::Minimum:
    case Type::Delta:
        return Unit::ToString(frequency, "Hz", " kMG", 6);
    case Type::Lowpass:
    case Type::Highpass:
    case Type::Bandpass:
        return Unit::ToString(cutoffAmplitude, "db", " ", 3);
    default:
        return "Invalid";
    }
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
        update();
        emit rawDataChanged();
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
    p.drawText(QRectF(0,0,width, height*2.0/3.0), Qt::AlignCenter, QString::number(number) + suffix);
    emit symbolChanged(this);
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

void TraceMarker::assignDeltaMarker(TraceMarker *m)
{
    if(delta) {
        disconnect(delta, &TraceMarker::dataChanged, this, &TraceMarker::update);
    }
    delta = m;
    if(delta && delta != this) {
        // this marker has to be updated when the delta marker changes
        connect(delta, &TraceMarker::rawDataChanged, this, &TraceMarker::update);
        connect(delta, &TraceMarker::deleted, [=](){
            delta = nullptr;
            update();
        });
    }
}

void TraceMarker::deleteHelperMarkers()
{
    for(auto m : helperMarkers) {
        delete m;
    }
    helperMarkers.clear();
}

void TraceMarker::setNumber(int value)
{
    number = value;
    updateSymbol();
}

QWidget *TraceMarker::getTypeEditor(QAbstractItemDelegate *delegate)
{
    auto c = new QComboBox;
    for(auto t : getTypes()) {
        c->addItem(typeToString(t));
        if(type == t) {
            // select this item
            c->setCurrentIndex(c->count() - 1);
        }
    }
    if(type == Type::Delta) {
        // add additional spinbox to choose corresponding delta marker
        auto w = new QWidget;
        auto layout = new QHBoxLayout;
        layout->addWidget(c);
        c->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->setContentsMargins(0,0,0,0);
        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget(new QLabel("to"));
        auto spinbox = new QSpinBox;
        if(delta) {
            spinbox->setValue(delta->number);
        }
        connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), [=](int newval){
           bool found = false;
           for(auto m : model->getMarkers()) {
                if(m->number == newval) {
                    assignDeltaMarker(m);
                    found = true;
                    break;
                }
           }
           if(!found) {
               assignDeltaMarker(nullptr);
           }
           update();
        });
        spinbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->addWidget(spinbox);
        w->setLayout(layout);
        c->setObjectName("Type");
        if(delegate){
            connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
                emit delegate->commitData(w);
            });
        }
        return w;
    } else {
        // no delta marker, simply return the combobox
        connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
            emit delegate->commitData(c);
        });
        return c;
    }
}

void TraceMarker::updateTypeFromEditor(QWidget *w)
{
    QComboBox *c;
    if(type == Type::Delta) {
        c = w->findChild<QComboBox*>("Type");
    } else {
        c = (QComboBox*) w;
    }
    for(auto t : getTypes()) {
        if(c->currentText() == typeToString(t)) {
            if(type != t) {
                // remove any potential helper markers
                deleteHelperMarkers();
                type = t;
                switch(type) {
                case Type::Delta:
                    if(!delta) {
                        // invalid delta marker assigned, attempt to find a matching marker
                        for(int pass = 0;pass < 3;pass++) {
                            for(auto m : model->getMarkers()) {
                                if(pass == 0 && m->parentTrace != parentTrace) {
                                    // ignore markers on different traces in first pass
                                    continue;
                                }
                                if(pass <= 1 && m == this) {
                                    // ignore itself on second pass
                                    continue;
                                }
                                assignDeltaMarker(m);
                                break;
                            }
                            if(delta) {
                                break;
                            }
                        }
                    }
                    break;
                case Type::Lowpass:
                case Type::Highpass: {
                    // Create helper marker for cutoff frequency
                    auto cutoff = new TraceMarker(model);
                    cutoff->suffix = "c";
                    // same trace as this one
                    cutoff->assignTrace(parentTrace);
                    helperMarkers.push_back(cutoff);
                }
                    break;
                case Type::Bandpass: {
                    // Create helper markers for cutoff frequency
                    auto lower = new TraceMarker(model);
                    lower->suffix = "l";
                    // same trace as this one
                    lower->assignTrace(parentTrace);
                    helperMarkers.push_back(lower);
                    auto higher = new TraceMarker(model);
                    higher->suffix = "h";
                    // same trace as this one
                    higher->assignTrace(parentTrace);
                    helperMarkers.push_back(higher);
                    auto center = new TraceMarker(model);
                    center->suffix = "c";
                    // same trace as this one
                    center->assignTrace(parentTrace);
                    helperMarkers.push_back(center);
                }
                    break;
                default:
                    break;
                }
                emit typeChanged(this);
                update();
            }
        }
    }
}

SIUnitEdit *TraceMarker::getSettingsEditor()
{
    switch(type) {
    case Type::Manual:
    case Type::Maximum:
    case Type::Minimum:
    case Type::Delta:
    default:
        return new SIUnitEdit("Hz", " kMG");
    case Type::Lowpass:
    case Type::Highpass:
        return new SIUnitEdit("db", " ");
    }
}

void TraceMarker::adjustSettings(double value)
{
    switch(type) {
    case Type::Manual:
    case Type::Maximum:
    case Type::Minimum:
    case Type::Delta:
    default:
        setFrequency(value);
        /* no break */
    case Type::Lowpass:
    case Type::Highpass:
    case Type::Bandpass:
        if(value > 0.0) {
            value = -value;
        }
        cutoffAmplitude = value;
    }
}

void TraceMarker::update()
{
    if(!parentTrace->size()) {
        // empty trace, nothing to do
        return;
    }
    switch(type) {
    case Type::Manual:
    case Type::Delta:
        // nothing to do
        break;
    case Type::Maximum:
        setFrequency(parentTrace->findExtremumFreq(true));
        break;
    case Type::Minimum:
        setFrequency(parentTrace->findExtremumFreq(false));
        break;
    case Type::Lowpass:
    case Type::Highpass:
        if(parentTrace->isReflection()) {
            // lowpass/highpass calculation only works with transmission measurement
            break;
        } else {
            // find the maximum
            auto peakFreq = parentTrace->findExtremumFreq(true);
            // this marker shows the insertion loss
            setFrequency(peakFreq);
            // find the cutoff frequency
            auto index = parentTrace->index(peakFreq);
            auto peakAmplitude = 20*log10(abs(parentTrace->sample(index).S));
            auto cutoff = peakAmplitude + cutoffAmplitude;
            int inc = type == Type::Lowpass ? 1 : -1;
            while(index >= 0 && index < (int) parentTrace->size()) {
                auto amplitude = 20*log10(abs(parentTrace->sample(index).S));
                if(amplitude <= cutoff) {
                    break;
                }
                index += inc;
            }
            if(index < 0) {
                index = 0;
            } else if(index >= (int) parentTrace->size()) {
                index = parentTrace->size() - 1;
            }
            // set position of cutoff marker
            helperMarkers[0]->setFrequency(parentTrace->sample(index).frequency);
        }
        break;
    case Type::Bandpass:
        if(parentTrace->isReflection()) {
            // lowpass/highpass calculation only works with transmission measurement
            break;
        } else {
            // find the maximum
            auto peakFreq = parentTrace->findExtremumFreq(true);
            // this marker shows the insertion loss
            setFrequency(peakFreq);
            // find the cutoff frequencies
            auto index = parentTrace->index(peakFreq);
            auto peakAmplitude = 20*log10(abs(parentTrace->sample(index).S));
            auto cutoff = peakAmplitude + cutoffAmplitude;

            auto low_index = index;
            while(low_index >= 0) {
                auto amplitude = 20*log10(abs(parentTrace->sample(low_index).S));
                if(amplitude <= cutoff) {
                    break;
                }
                low_index--;
            }
            if(low_index < 0) {
                low_index = 0;
            }
            // set position of cutoff marker
            helperMarkers[0]->setFrequency(parentTrace->sample(low_index).frequency);

            auto high_index = index;
            while(high_index < (int) parentTrace->size()) {
                auto amplitude = 20*log10(abs(parentTrace->sample(high_index).S));
                if(amplitude <= cutoff) {
                    break;
                }
                high_index++;
            }
            if(high_index >= (int) parentTrace->size()) {
                high_index = parentTrace->size() - 1;
            }
            // set position of cutoff marker
            helperMarkers[1]->setFrequency(parentTrace->sample(high_index).frequency);
            // set center marker inbetween cutoff markers
            helperMarkers[2]->setFrequency((helperMarkers[0]->frequency + helperMarkers[1]->frequency) / 2);
        }
        break;
    }
    emit dataChanged(this);
}

Trace *TraceMarker::getTrace() const
{
    return parentTrace;
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

