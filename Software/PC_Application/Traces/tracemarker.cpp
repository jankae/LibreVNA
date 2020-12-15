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
    : editingFrequency(false),
      model(model),
      parentTrace(nullptr),
      position(1000000000),
      number(number),
      data(0),
      type(Type::Manual),
      description(descr),
      delta(nullptr),
      parent(parent),
      cutoffAmplitude(-3.0),
      peakThreshold(-40.0),
      offset(10000)
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
        disconnect(parentTrace, &Trace::deleted, this, nullptr);
    }
    parentTrace = t;
    if(!getSupportedTypes().count(type)) {
        // new trace does not support the current type
        setType(Type::Manual);
    }

    connect(parentTrace, &Trace::deleted, this, &TraceMarker::parentTraceDeleted);
    connect(parentTrace, &Trace::dataChanged, this, &TraceMarker::traceDataChanged);
    connect(parentTrace, &Trace::colorChanged, this, &TraceMarker::updateSymbol);
    connect(parentTrace, &Trace::typeChanged, [=](){
        emit domainChanged();
        checkDeltaMarker();
    });
    constrainPosition();
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
    if(!parentTrace) {
        return "";
    }
    if(position < parentTrace->minX() || position > parentTrace->maxX()) {
        return "";
    }
    if(isTimeDomain()) {
        switch(type) {
        case Type::Manual: {
            QString ret;
            auto impulse = data.real();
            auto step = parentTrace->sample(parentTrace->index(position), Trace::SampleType::TimeStep).y.real();
            ret += "Impulse:"+Unit::ToString(impulse, "", "m ", 3);
            if(!isnan(step)) {
                ret += " Step:"+Unit::ToString(step, "", "m ", 3);
                if(abs(step) < 1.0) {
                    auto impedance = 50.0 * (1.0 + step) / (1.0 - step);
                    ret += " Impedance:"+Unit::ToString(impedance, "Ω", "m kM", 3);
                }
            }
            return ret;
        }
        case Type::Delta: {
            if(!delta || !delta->isTimeDomain()) {
                return "Invalid delta marker";
            }
            // calculate difference between markers
            auto impulse = data.real() - delta->data.real();
            QString ret;
            auto timeDiff = position - delta->position;
            auto distanceDiff = parentTrace->timeToDistance(position) - delta->parentTrace->timeToDistance(delta->position);
            ret += "Δ:"+Unit::ToString(timeDiff, "s", "fpnum ", 4) + "/" + Unit::ToString(distanceDiff, "m", "m k", 4);
            ret += " ΔImpulse:"+Unit::ToString(impulse, "", "m ", 3);
            auto step = parentTrace->sample(parentTrace->index(position), Trace::SampleType::TimeStep).y.real();
            auto stepDelta = delta->parentTrace->sample(delta->parentTrace->index(delta->position), Trace::SampleType::TimeStep).y.real();
            if(!isnan(step) && !isnan(stepDelta)) {
                auto stepDiff = step - stepDelta;
                ret += " ΔStep:"+Unit::ToString(stepDiff, "", "m ", 3);
                if(abs(step) < 1.0 && abs(stepDelta) < 1.0) {
                    auto impedance = 50.0 * (1.0 + step) / (1.0 - step);
                    auto impedanceDelta = 50.0 * (1.0 + stepDelta) / (1.0 - stepDelta);
                    auto impedanceDiff = impedance - impedanceDelta;
                    ret += " ΔImpedance:"+Unit::ToString(impedanceDiff, "Ω", "m kM", 3);
                }
            }
            return ret;
        }
        default:
            return "Invalid type";
        }
    } else {
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum: {
                auto phase = arg(data);
                return QString::number(toDecibel(), 'g', 4) + "db@" + QString::number(phase*180/M_PI, 'g', 4);
            }
        case Type::Delta:
            if(!delta || delta->isTimeDomain()) {
                return "Invalid delta marker";
            } else {
                // calculate difference between markers
                auto freqDiff = position - delta->position;
                auto valueDiff = data / delta->data;
                auto phase = arg(valueDiff);
                auto db = 20*log10(abs(valueDiff));
                return Unit::ToString(freqDiff, "Hz", " kMG") + " / " + QString::number(db, 'g', 4) + "db@" + QString::number(phase*180/M_PI, 'g', 4);
            }
            break;
        case Type::Noise:
            return Unit::ToString(parentTrace->getNoise(position), "dbm/Hz", " ", 3);
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
                ret += Unit::ToString(helperMarkers[0]->position, "Hz", " kMG", 4);
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
                auto bandwidth = helperMarkers[1]->position - helperMarkers[0]->position;
                auto center = helperMarkers[2]->position;
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
            auto phasenoise = parentTrace->getNoise(helperMarkers[0]->position) - carrier;
            return Unit::ToString(phasenoise, "dbc/Hz", " ", 3)  +"@" + Unit::ToString(offset, "Hz", " kM", 4) + " offset (" + Unit::ToString(position, "Hz", " kMG", 6) + " carrier)";
        }
        default:
            return "Unknown marker type";
        }
    }
}

QString TraceMarker::readableSettings()
{
    if(isTimeDomain()) {
        switch(type) {
        case Type::Manual:
        case Type::Delta:
            return Unit::ToString(position, "s", "fpnum ", 4) + "/" + Unit::ToString(parentTrace->timeToDistance(position), "m", "um k", 4);
        default:
            return "Unhandled case";
        }
    } else {
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
        case Type::Noise:
            return Unit::ToString(position, "Hz", " kMG", 6);
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
}

QString TraceMarker::tooltipSettings()
{
    if(isTimeDomain()) {
        switch(type) {
        case Type::Manual:
        case Type::Delta:
            return "Time/Distance";
        default:
            return QString();
        }
    } else {
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
        case Type::Noise:
            return "Marker frequency";
        case Type::Lowpass:
        case Type::Highpass:
        case Type::Bandpass:
            return "Cutoff amplitude (relativ to peak)";
        case Type::PeakTable:
            return "Peak threshold";
        case Type::PhaseNoise:
            return "Frequency offset";
        default:
            return QString();
        }
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

void TraceMarker::setPosition(double pos)
{
    position = pos;
    constrainPosition();
}

void TraceMarker::parentTraceDeleted(Trace *t)
{
    if(t == parentTrace) {
        delete this;
    }
}

void TraceMarker::traceDataChanged()
{
    complex<double> newdata;
    if(!parentTrace || parentTrace->numSamples() == 0) {
        // no data, invalidate
        newdata = numeric_limits<complex<double>>::quiet_NaN();
    } else {
        if(position < parentTrace->minX() || position > parentTrace->maxX()) {
            // this normally should not happen because the position is constrained to the trace X range.
            // However, when loading a setup, the trace might have been just created and essentially empty
            newdata = numeric_limits<complex<double>>::quiet_NaN();
        } else {
            // some data of the parent trace changed, check if marker data also changed
            auto sampleType = isTimeDomain() ? Trace::SampleType::TimeImpulse : Trace::SampleType::Frequency;
            newdata = parentTrace->sample(parentTrace->index(position), sampleType).y;
        }
    }
    if (newdata != data) {
        data = newdata;
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

void TraceMarker::checkDeltaMarker()
{
    if(type != Type::Delta) {
        // not a delta marker, nothing to do
        return;
    }
    // Check if type of delta marker is still okay
    if(!delta || delta->isTimeDomain() != isTimeDomain()) {
        // not the same domain anymore, adjust delta
        assignDeltaMarker(bestDeltaCandidate());
    }
}

void TraceMarker::deltaDeleted()
{
    // the delta marker of this marker has been deleted, find new match
    delta = nullptr;
    qDebug() << "assigned delta deleted";
    assignDeltaMarker(bestDeltaCandidate());
    update();
}

std::set<TraceMarker::Type> TraceMarker::getSupportedTypes()
{
    set<TraceMarker::Type> supported;
    if(parentTrace) {
        if(isTimeDomain()) {
            // only basic markers in time domain
            supported.insert(Type::Manual);
            supported.insert(Type::Delta);
        } else {
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
    }
    return supported;
}

void TraceMarker::constrainPosition()
{
    if(parentTrace) {
        if(parentTrace->size() > 0)  {
            if(position > parentTrace->maxX()) {
                position = parentTrace->maxX();
            } else if(position < parentTrace->minX()) {
                position = parentTrace->minX();
            }
        }
        traceDataChanged();
    }
}

TraceMarker *TraceMarker::bestDeltaCandidate()
{
    TraceMarker *match = nullptr;
    // invalid delta marker assigned, attempt to find a matching marker
    for(int pass = 0;pass < 3;pass++) {
        for(auto m : model->getMarkers()) {
            if(m->isTimeDomain() != isTimeDomain()) {
                // markers are not on the same domain
                continue;
            }
            if(pass == 0 && m->parentTrace != parentTrace) {
                // ignore markers on different traces in first pass
                continue;
            }
            if(pass <= 1 && m == this) {
                // ignore itself on second pass
                continue;
            }
            match = m;
            break;
        }
        if(match) {
            break;
        }
    }
    return match;
}

void TraceMarker::assignDeltaMarker(TraceMarker *m)
{
    if(type != Type::Delta) {
        // ignore
        return;
    }
    if(delta) {
        disconnect(delta, &TraceMarker::dataChanged, this, &TraceMarker::update);
    }
    delta = m;
    if(delta && delta != this) {
        // this marker has to be updated when the delta marker changes
        connect(delta, &TraceMarker::rawDataChanged, this, &TraceMarker::update);
        connect(delta, &TraceMarker::domainChanged, this, &TraceMarker::checkDeltaMarker);
        connect(delta, &TraceMarker::deleted, this, &TraceMarker::deltaDeleted);
    }
    emit assignedDeltaChanged(this);
}

void TraceMarker::deleteHelperMarkers()
{
    if(helperMarkers.size() > 0) {
        emit beginRemoveHelperMarkers(this);
        for(auto m : helperMarkers) {
            delete m;
        }
        helperMarkers.clear();
        emit endRemoveHelperMarkers(this);
    }
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
        assignDeltaMarker(bestDeltaCandidate());
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

TraceMarker::Type TraceMarker::getType() const
{
    return type;
}

QString TraceMarker::getSuffix() const
{
    return suffix;
}

nlohmann::json TraceMarker::toJSON()
{
    nlohmann::json j;
    j["trace"] = parentTrace->toHash();
    j["type"] = typeToString(type).toStdString();
    j["number"] = number;
    j["position"] = position;
    switch(type) {
    case Type::Delta:
        j["delta_marker"] = delta->toHash();
        break;
    case Type::PeakTable:
        j["peak_threshold"] = peakThreshold;
        break;
    case Type::Lowpass:
    case Type::Highpass:
    case Type::Bandpass:
        j["cutoff"] = cutoffAmplitude;
        break;
    case Type::PhaseNoise:
        j["offset"] = offset;
        break;
    default:
        // other types have no settings
        break;
    }
    return j;
}

void TraceMarker::fromJSON(nlohmann::json j)
{
    if(!j.contains("trace")) {
        throw runtime_error("Marker has no trace assigned");
    }
    number = j.value("number", 1);
    position = j.value("position", 0.0);

    unsigned int hash = j["trace"];
    // find correct trace
    bool found = false;
    for(auto t : model->getModel().getTraces()) {
        if(t->toHash() == hash) {
            found = true;
            assignTrace(t);
            break;
        }
    }
    if(!found) {
        throw runtime_error("Unable to find trace with hash " + to_string(hash));
    }
    auto typeString = QString::fromStdString(j.value("type", "Manual"));
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(typeToString((Type) i) == typeString) {
            setType((Type) i);
            break;
        }
    }
    switch(type) {
    case Type::Delta:
        // can't assign delta marker here, because it might not have been created (if it was below this marker in the table).
        // Instead it will be correctly assigned in TraceMarkerModel::fromJSON()
        break;
    case Type::PeakTable:
        peakThreshold = j.value("peak_threshold", -40);
        break;
    case Type::Lowpass:
    case Type::Highpass:
    case Type::Bandpass:
        cutoffAmplitude = j.value("cutoff", -3.0);
        break;
    case Type::PhaseNoise:
        j.value("offset", 10000);
        break;
    default:
        // other types have no settings
        break;
    }
    update();
}

unsigned int TraceMarker::toHash()
{
    // taking the easy way: create the json string and hash it (already contains all necessary information)
    // This is slower than it could be, but this function is only used when loading setups, so this isn't a big problem
    std::string json_string = toJSON().dump();
    return hash<std::string>{}(json_string);
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
    if(isTimeDomain()) {
        switch(type) {
        case Type::Manual:
        case Type::Delta:
            return new SIUnitEdit("", "fpnum k", 6);
        default:
            return nullptr;
        }
    } else {
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
}

void TraceMarker::adjustSettings(double value)
{
    if(isTimeDomain()) {
        switch(type) {
        case Type::Manual:
        case Type::Delta: {
            // check if entered position is time or distance
            if(value > parentTrace->sample(parentTrace->size() - 1).x) {
                // entered a distance, convert to time
                setPosition(parentTrace->distanceToTime(value));
            } else {
                // entered a time, can set directly
                setPosition(value);
            }
        }
        default:
            break;
        }
    } else {
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
        case Type::Noise:
            setPosition(value);
            break;
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
        default:
            break;
        }
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
        setPosition(parentTrace->findExtremumFreq(true));
        break;
    case Type::Minimum:
        setPosition(parentTrace->findExtremumFreq(false));
        break;
    case Type::PeakTable: {
        deleteHelperMarkers();
        auto peaks = parentTrace->findPeakFrequencies(100, peakThreshold);
        char suffix = 'a';
        for(auto p : peaks) {
            auto helper = new TraceMarker(model, number, this);
            helper->suffix = suffix;
            helper->assignTrace(parentTrace);
            helper->setPosition(p);
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
            setPosition(peakFreq);
            // find the cutoff frequency
            auto index = parentTrace->index(peakFreq);
            auto peakAmplitude = 20*log10(abs(parentTrace->sample(index).y));
            auto cutoff = peakAmplitude + cutoffAmplitude;
            int inc = type == Type::Lowpass ? 1 : -1;
            while(index >= 0 && index < (int) parentTrace->size()) {
                auto amplitude = 20*log10(abs(parentTrace->sample(index).y));
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
            helperMarkers[0]->setPosition(parentTrace->sample(index).x);
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
            setPosition(peakFreq);
            // find the cutoff frequencies
            auto index = parentTrace->index(peakFreq);
            auto peakAmplitude = 20*log10(abs(parentTrace->sample(index).y));
            auto cutoff = peakAmplitude + cutoffAmplitude;

            auto low_index = index;
            while(low_index >= 0) {
                auto amplitude = 20*log10(abs(parentTrace->sample(low_index).y));
                if(amplitude <= cutoff) {
                    break;
                }
                low_index--;
            }
            if(low_index < 0) {
                low_index = 0;
            }
            // set position of cutoff marker
            helperMarkers[0]->setPosition(parentTrace->sample(low_index).x);

            auto high_index = index;
            while(high_index < (int) parentTrace->size()) {
                auto amplitude = 20*log10(abs(parentTrace->sample(high_index).y));
                if(amplitude <= cutoff) {
                    break;
                }
                high_index++;
            }
            if(high_index >= (int) parentTrace->size()) {
                high_index = parentTrace->size() - 1;
            }
            // set position of cutoff marker
            helperMarkers[1]->setPosition(parentTrace->sample(high_index).x);
            // set center marker inbetween cutoff markers
            helperMarkers[2]->setPosition((helperMarkers[0]->position + helperMarkers[1]->position) / 2);
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
        helperMarkers[0]->setPosition(peaks[0]);
        helperMarkers[1]->setPosition(peaks[1]);
        auto freqDiff = peaks[1] - peaks[0];
        helperMarkers[2]->setPosition(peaks[0] - freqDiff);
        helperMarkers[3]->setPosition(peaks[1] + freqDiff);
    }
        break;
    case Type::PhaseNoise:
        setPosition(parentTrace->findExtremumFreq(true));
        helperMarkers[0]->setPosition(position + offset);
        break;
    case Type::Last:
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

double TraceMarker::getPosition() const
{
    return position;
}

bool TraceMarker::isTimeDomain()
{
    if(parentTrace) {
        if(parentTrace->outputType() == Trace::DataType::Time) {
            return true;
        }
    }
    return false;
}

