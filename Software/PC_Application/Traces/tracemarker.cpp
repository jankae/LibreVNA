#include "tracemarker.h"
#include <QPainter>
#include "CustomWidgets/siunitedit.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include "tracemarkermodel.h"
#include "unit.h"

using namespace std;

TraceMarker::TraceMarker(TraceMarkerModel *model, int number, TraceMarker *parent, QString descr)
    : editingFrequeny(false),
      model(model),
      parentTrace(nullptr),
      frequency(1000000000),
      number(number),
      data(0),
      type(Type::Manual),
      description(descr),
      delta(nullptr),
      parent(parent),
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
    if(!getSupportedTypes().count(type)) {
        // new trace does not support the current type
        setType(Type::Manual);
    }

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
    emit traceChanged(this);
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
        auto phase = arg(data);
        return QString::number(toDecibel(), 'g', 4) + "db@" + QString::number(phase*180/M_PI, 'g', 4);
    }
    case Type::Delta:
        if(!delta) {
            return "Invalid delta marker";
        } else {
            // calculate difference between markers
            auto freqDiff = frequency - delta->frequency;
            auto valueDiff = data / delta->data;
            auto phase = arg(valueDiff);
            return Unit::ToString(freqDiff, "Hz", " kMG") + " / " + QString::number(toDecibel(), 'g', 4) + "db@" + QString::number(phase*180/M_PI, 'g', 4);
        }
        break;
    case Type::Noise:
        return Unit::ToString(parentTrace->getNoise(frequency), "dbm/Hz", " ", 3);
    case Type::PeakTable:
        return "Found " + QString::number(helperMarkers.size()) + " peaks";
    case Type::Lowpass:
    case Type::Highpass:
        if(parentTrace->isReflection()) {
            return "Calculation not possible with reflection measurement";
        } else {
            auto insertionLoss = toDecibel();
            auto cutoff = helperMarkers[0]->toDecibel();
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
            auto insertionLoss = toDecibel();
            auto cutoffL = helperMarkers[0]->toDecibel();
            auto cutoffH = helperMarkers[1]->toDecibel();
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
    case Type::TOI: {
        auto avgFundamental = (helperMarkers[0]->toDecibel() + helperMarkers[1]->toDecibel()) / 2;
        auto avgDistortion = (helperMarkers[2]->toDecibel() + helperMarkers[3]->toDecibel()) / 2;
        auto TOI = (3 * avgFundamental - avgDistortion) / 2;
        return "Fundamental: " + Unit::ToString(avgFundamental, "dbm", " ", 3) + ", distortion: " + Unit::ToString(avgDistortion, "dbm", " ", 3) + ", TOI: "+Unit::ToString(TOI, "dbm", " ", 3);
    }
        break;
    case Type::PhaseNoise: {
        auto carrier = toDecibel();
        auto phasenoise = parentTrace->getNoise(helperMarkers[0]->frequency) - carrier;
        return Unit::ToString(phasenoise, "dbc/Hz", " ", 3)  +"@" + Unit::ToString(offset, "Hz", " kM", 4) + " offset (" + Unit::ToString(frequency, "Hz", " kMG", 6) + " carrier)";
    }
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
    case Type::Noise:
        return Unit::ToString(frequency, "Hz", " kMG", 6);
    case Type::Lowpass:
    case Type::Highpass:
    case Type::Bandpass:
        return Unit::ToString(cutoffAmplitude, "db", " ", 3);
    case Type::PeakTable:
        return Unit::ToString(peakThreshold, "db", " ", 3);
    case Type::TOI:
        return "none";
    case Type::PhaseNoise:
        return Unit::ToString(offset, "Hz", " kM", 4);
    default:
        return "Unhandled case";
    }
}

QString TraceMarker::readableType()
{
    if(parent) {
        return description;
    } else {
        return typeToString(type);
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
    if(isVisible()) {
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
    } else {
        symbol = QPixmap(1,1);
    }
    emit symbolChanged(this);
}

std::set<TraceMarker::Type> TraceMarker::getSupportedTypes()
{
    set<TraceMarker::Type> supported;
    if(parentTrace) {
        // all traces support some basic markers
        supported.insert(Type::Manual);
        supported.insert(Type::Maximum);
        supported.insert(Type::Minimum);
        supported.insert(Type::Delta);
        supported.insert(Type::PeakTable);
        if(parentTrace->isLive()) {
            switch(parentTrace->liveParameter()) {
            case Trace::LiveParameter::S11:
            case Trace::LiveParameter::S12:
            case Trace::LiveParameter::S21:
            case Trace::LiveParameter::S22:
                // special VNA marker types
                supported.insert(Type::Lowpass);
                supported.insert(Type::Highpass);
                supported.insert(Type::Bandpass);
                break;
            case Trace::LiveParameter::Port1:
            case Trace::LiveParameter::Port2:
                // special SA marker types
                supported.insert(Type::Noise);
                supported.insert(Type::TOI);
                supported.insert(Type::PhaseNoise);
                break;
            }
        }
    }
    return supported;
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
    emit beginRemoveHelperMarkers(this);
    for(auto m : helperMarkers) {
        delete m;
    }
    helperMarkers.clear();
    emit endRemoveHelperMarkers(this);
}

void TraceMarker::setType(TraceMarker::Type t)
{
    // remove any potential helper markers
    deleteHelperMarkers();
    type = t;
    using helper_descr = struct {
        QString suffix;
        QString description;
        Type type;
    };
    vector<helper_descr> required_helpers;
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
    case Type::Highpass:
        required_helpers = {{"c", "cutoff", Type::Manual}};
        break;
    case Type::Bandpass:
        required_helpers = {{"l", "lower cutoff", Type::Manual}, {"h", "higher cutoff", Type::Manual} ,{"c", "center", Type::Manual}};
        break;
    case Type::TOI:
        required_helpers = {{"p", "first peak", Type::Manual}, {"p", "second peak", Type::Manual}, {"l", "left intermodulation", Type::Manual}, {"r", "right intermodulation", Type::Manual}};
        break;
    case Type::PhaseNoise:
        required_helpers = {{"o", "Offset", Type::Noise}};
        break;
    default:
        break;
    }
    // create helper markers
    for(auto h : required_helpers) {
        auto helper = new TraceMarker(model, number, this, h.description);
        helper->suffix = h.suffix;
        helper->assignTrace(parentTrace);
        helper->setType(h.type);
        helperMarkers.push_back(helper);
    }
    updateSymbol();
    emit typeChanged(this);
    update();
}

double TraceMarker::toDecibel()
{
    return 20*log10(abs(data));
}

bool TraceMarker::isVisible()
{
    switch(type) {
    case Type::Manual:
    case Type::Delta:
    case Type::Maximum:
    case Type::Minimum:
    case Type::Noise:
    case Type::PhaseNoise:
        return true;
    default:
        return false;
    }
}

QString TraceMarker::getSuffix() const
{
    return suffix;
}

const std::vector<TraceMarker *> &TraceMarker::getHelperMarkers() const
{
    return helperMarkers;
}

TraceMarker *TraceMarker::helperMarker(unsigned int i)
{
    if(i < helperMarkers.size()) {
        return helperMarkers[i];
    } else {
        return nullptr;
    }
}

TraceMarker *TraceMarker::getParent() const
{
    return parent;
}

void TraceMarker::setNumber(int value)
{
    number = value;
    updateSymbol();
    for(auto h : helperMarkers) {
        h->setNumber(number);
    }
}

QWidget *TraceMarker::getTypeEditor(QAbstractItemDelegate *delegate)
{
    auto c = new QComboBox;
    for(auto t : getSupportedTypes()) {
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
    for(auto t : getSupportedTypes()) {
        if(c->currentText() == typeToString(t)) {
            if(type != t) {
                setType(t);
            }
        }
    }
    update();
}

SIUnitEdit *TraceMarker::getSettingsEditor()
{
    switch(type) {
    case Type::Manual:
    case Type::Maximum:
    case Type::Minimum:
    case Type::Delta:
    case Type::Noise:
    case Type::PhaseNoise:
    default:
        return new SIUnitEdit("Hz", " kMG", 6);
    case Type::Lowpass:
    case Type::Highpass:
    case Type::PeakTable:
        return new SIUnitEdit("db", " ", 3);
    case Type::TOI:
        return nullptr;
    }
}

void TraceMarker::adjustSettings(double value)
{
    switch(type) {
    case Type::Manual:
    case Type::Maximum:
    case Type::Minimum:
    case Type::Delta:
    case Type::Noise:
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
        break;
    case Type::PeakTable:
        peakThreshold = value;
        break;
    case Type::PhaseNoise:
        offset = value;
        break;
    }
    update();
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
    case Type::Noise:
        // nothing to do
        break;
    case Type::Maximum:
        setFrequency(parentTrace->findExtremumFreq(true));
        break;
    case Type::Minimum:
        setFrequency(parentTrace->findExtremumFreq(false));
        break;
    case Type::PeakTable: {
        deleteHelperMarkers();
        auto peaks = parentTrace->findPeakFrequencies(100, peakThreshold);
        char suffix = 'a';
        for(auto p : peaks) {
            auto helper = new TraceMarker(model, number, this);
            helper->suffix = suffix;
            helper->assignTrace(parentTrace);
            helper->setFrequency(p);
            suffix++;
            helperMarkers.push_back(helper);
        }
    }
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
    case Type::TOI: {
        auto peaks = parentTrace->findPeakFrequencies(2);
        if(peaks.size() != 2) {
            // error finding peaks, do nothing
            break;
        }
        // assign marker frequenies:
        // this marker is the left peak, first helper the right peak.
        // 2nd and 3rd helpers are left and right TOI peaks
        helperMarkers[0]->setFrequency(peaks[0]);
        helperMarkers[1]->setFrequency(peaks[1]);
        auto freqDiff = peaks[1] - peaks[0];
        helperMarkers[2]->setFrequency(peaks[0] - freqDiff);
        helperMarkers[3]->setFrequency(peaks[1] + freqDiff);
    }
        break;
    case Type::PhaseNoise:
        setFrequency(parentTrace->findExtremumFreq(true));
        helperMarkers[0]->setFrequency(frequency + offset);
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

bool TraceMarker::isMovable()
{
    if(parent) {
        // helper traces are never movable by the user
        return false;
    }
    switch(type) {
    case Type::Manual:
    case Type::Delta:
    case Type::Noise:
        return true;
    default:
        return false;
    }
}

QPixmap &TraceMarker::getSymbol()
{
    return symbol;
}

double TraceMarker::getFrequency() const
{
    return frequency;
}

