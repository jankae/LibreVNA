#include "trace.h"
#include <math.h>
#include "fftcomplex.h"

using namespace std;

Trace::Trace(QString name, QColor color, LiveParameter live)
    : _name(name),
      _color(color),
      _liveType(LivedataType::Overwrite),
      _liveParam(live),
      reflection(true),
      visible(true),
      paused(false),
      touchstone(false),
      calibration(false),
      lastMath(nullptr)
{
    MathInfo self = {.math = this, .enabled = true};
    mathOps.push_back(self);
    updateLastMath(mathOps.rbegin());

    self.enabled = false;
    dataType = DataType::Frequency;
}

Trace::~Trace()
{
    emit deleted(this);
}

void Trace::clear() {
    if(paused) {
        return;
    }
    data.clear();
    settings.valid = false;
    warning("No data");
    emit cleared(this);
    emit outputSamplesChanged(0, 0);
}

void Trace::addData(const Trace::Data& d) {
    // add or replace data in vector while keeping it sorted with increasing frequency
    auto lower = lower_bound(data.begin(), data.end(), d, [](const Data &lhs, const Data &rhs) -> bool {
        return lhs.x < rhs.x;
    });
    if(lower == data.end()) {
        // highest frequency yet, add to vector
        data.push_back(d);
    } else if(lower->x == d.x) {
        switch(_liveType) {
        case LivedataType::Overwrite:
            // replace this data element
            *lower = d;
            break;
        case LivedataType::MaxHold:
            // replace this data element
            if(abs(d.y) > abs(lower->y)) {
                *lower = d;
            }
            break;
        case LivedataType::MinHold:
            // replace this data element
            if(abs(d.y) < abs(lower->y)) {
                *lower = d;
            }
            break;
        }

    } else {
        // insert at this position
        data.insert(lower, d);
    }
    success();
    emit outputSamplesChanged(lower - data.begin(), lower - data.begin() + 1);
}

void Trace::addData(const Trace::Data &d, const Protocol::SweepSettings &s)
{
    settings.VNA = s;
    settings.valid = true;
    addData(d);
}

void Trace::addData(const Trace::Data &d, const Protocol::SpectrumAnalyzerSettings &s)
{
    settings.SA = s;
    settings.valid = true;
    addData(d);
}

void Trace::setName(QString name) {
    _name = name;
    emit nameChanged();
}

void Trace::fillFromTouchstone(Touchstone &t, unsigned int parameter, QString filename)
{
    if(parameter >= t.ports()*t.ports()) {
        throw runtime_error("Parameter for touchstone out of range");
    }
    clear();
    setTouchstoneParameter(parameter);
    setTouchstoneFilename(filename);
    for(unsigned int i=0;i<t.points();i++) {
        auto tData = t.point(i);
        Data d;
        d.x = tData.frequency;
        d.y = t.point(i).S[parameter];
        addData(d);
    }
    // check if parameter is square (e.i. S11/S22/S33/...)
    parameter++;
    bool isSquare = false;
    for (unsigned int i = 1; i * i <= parameter; i++) {

        // If (i * i = n)
        if ((parameter % i == 0) && (parameter / i == i)) {
            isSquare = true;
            break;
        }
    }
    if(isSquare == 1) {
        reflection = true;
    } else {
        reflection = false;
    }
    touchstone = true;
    emit typeChanged(this);
}

void Trace::fromLivedata(Trace::LivedataType type, LiveParameter param)
{
    touchstone = false;
    _liveType = type;
    _liveParam = param;
    if(param == LiveParameter::S11 || param == LiveParameter::S22) {
        reflection = true;
    } else {
        reflection = false;
    }
    emit typeChanged(this);
}

void Trace::setColor(QColor color) {
    if(_color != color) {
        _color = color;
        emit colorChanged(this);
    }
}

void Trace::addMarker(TraceMarker *m)
{
    markers.insert(m);
    emit markerAdded(m);
}

void Trace::removeMarker(TraceMarker *m)
{
    markers.erase(m);
    emit markerRemoved(m);
}

const std::vector<Trace::MathInfo>& Trace::getMathOperations() const
{
    return mathOps;
}

void Trace::updateLastMath(vector<MathInfo>::reverse_iterator start)
{
    TraceMath *newLast = nullptr;
    for(auto it = start;it != mathOps.rend();it++) {
        if(it->enabled) {
            newLast = it->math;
            break;
        }
    }
    Q_ASSERT(newLast != nullptr);
    if(newLast != lastMath) {
        if(lastMath != nullptr) {
            disconnect(lastMath, &TraceMath::outputSamplesChanged, this, nullptr);
        }
        lastMath = newLast;
        // relay signals of end of math chain
        connect(lastMath, &TraceMath::outputSamplesChanged, this, &Trace::dataChanged);
        emit typeChanged(this);
        emit outputSamplesChanged(0, data.size());
    }
}

void Trace::setReflection(bool value)
{
    reflection = value;
}

QString Trace::description()
{
    return name() + ": measured data";
}

void Trace::setCalibration(bool value)
{
    calibration = value;
}

std::set<TraceMarker *> Trace::getMarkers() const
{
    return markers;
}

void Trace::setVisible(bool visible)
{
    if(visible != this->visible) {
        this->visible = visible;
        emit visibilityChanged(this);
    }
}

bool Trace::isVisible()
{
    return visible;
}

void Trace::pause()
{
    paused = true;
}

void Trace::resume()
{
    paused = false;
}

bool Trace::isPaused()
{
    return paused;
}

bool Trace::isTouchstone()
{
    return touchstone;
}

bool Trace::isCalibration()
{
    return calibration;
}

bool Trace::isLive()
{
    return !isCalibration() && !isTouchstone();
}

bool Trace::isReflection()
{
    return reflection;
}

bool Trace::mathEnabled()
{
    return lastMath != this;
}

bool Trace::hasMathOperations()
{
    return mathOps.size() > 1;
}

void Trace::enableMath(bool enable)
{
    auto start = enable ? mathOps.rbegin() : make_reverse_iterator(mathOps.begin() + 1);
    updateLastMath(start);
}

void Trace::addMathOperation(TraceMath *math)
{
    MathInfo info = {.math = math, .enabled = true};
    math->assignInput(lastMath);
    mathOps.push_back(info);
    updateLastMath(mathOps.rbegin());
}

void Trace::removeMathOperation(unsigned int index)
{
    if(index < 1 || index >= mathOps.size()) {
        return;
    }
    if(mathOps[index].enabled) {
        enableMathOperation(index, false);
    }
    delete mathOps[index].math;
    mathOps.erase(mathOps.begin() + index);
}

void Trace::swapMathOrder(unsigned int index)
{
    if(index < 1 || index + 1 >= mathOps.size()) {
        return;
    }
    // store enable state and disable prior to swap (can reuse enable/disable function to handle input assignment)
    bool index_enabled = mathOps[index].enabled;
    bool next_enabled = mathOps[index].enabled;
    enableMathOperation(index, false);
    enableMathOperation(index + 1, false);
    // actually swap the information
    swap(mathOps[index], mathOps[index+1]);
    // restore enable state
    enableMathOperation(index, next_enabled);
    enableMathOperation(index + 1, index_enabled);
}

void Trace::enableMathOperation(unsigned int index, bool enable)
{
    if(index < 1 || index >= mathOps.size()) {
        return;
    }
    if(mathOps[index].enabled != enable) {
        // find the next and previous operations that are enabled
        unsigned int next_index = index + 1;
        for(;next_index<mathOps.size();next_index++) {
            if(mathOps[next_index].enabled) {
                break;
            }
        }
        unsigned int prev_index = index - 1;
        for(;prev_index>0;prev_index--) {
            if(mathOps[prev_index].enabled) {
                break;
            }
        }
        if(enable) {
            // assign the previous enabled operation as the input for this operation
            mathOps[index].math->assignInput(mathOps[prev_index].math);
            // if another operation was active after index, reassign its input to index
            if(next_index < mathOps.size()) {
                mathOps[next_index].math->assignInput(mathOps[index].math);
            }
        } else {
            // this operation gets disabled, reassign possible operation after it
            if(next_index < mathOps.size()) {
                mathOps[next_index].math->assignInput(mathOps[prev_index].math);
            }
            mathOps[index].math->removeInput();
        }
        mathOps[index].enabled = enable;
        updateLastMath(mathOps.rbegin());
    }
}

unsigned int Trace::size()
{
    return lastMath->numSamples();
}

double Trace::minX()
{
    if(lastMath->numSamples() > 0) {
        return lastMath->rData().front().x;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

double Trace::maxX()
{
    if(lastMath->numSamples() > 0) {
        return lastMath->rData().back().x;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

double Trace::findExtremumFreq(bool max)
{
    if(lastMath->getDataType() != DataType::Frequency) {
        // not in frequency domain
        return numeric_limits<double>::quiet_NaN();
    }
    double compare = max ? numeric_limits<double>::min() : numeric_limits<double>::max();
    double freq = 0.0;
    for(auto sample : lastMath->rData()) {
        double amplitude = abs(sample.y);
        if((max && (amplitude > compare)) || (!max && (amplitude < compare))) {
            // higher/lower extremum found
            compare = amplitude;
            freq = sample.x;
        }
    }
    return freq;
}

std::vector<double> Trace::findPeakFrequencies(unsigned int maxPeaks, double minLevel, double minValley)
{
    if(lastMath->getDataType() != DataType::Frequency) {
        // not in frequency domain
        return vector<double>();
    }
    using peakInfo = struct peakinfo {
        double frequency;
        double level_dbm;
    };
    vector<peakInfo> peaks;
    double frequency = 0.0;
    double max_dbm = -200.0;
    double min_dbm = 200.0;
    for(auto d : lastMath->rData()) {
        double dbm = 20*log10(abs(d.y));
        if((dbm >= max_dbm) && (min_dbm <= dbm - minValley)) {
            // potential peak frequency
            frequency = d.x;
            max_dbm = dbm;
        }
        if(dbm <= min_dbm) {
            min_dbm = dbm;
        }
        if((dbm <= max_dbm - minValley) && (max_dbm >= minLevel) && frequency) {
            // peak was high enough and dropped below minValley afterwards
            peakInfo peak;
            peak.frequency = frequency;
            peak.level_dbm = max_dbm;
            peaks.push_back(peak);
            // reset
            frequency = 0.0;
            max_dbm = -200.0;
            min_dbm = dbm;
        }
    }
    if(peaks.size() > maxPeaks) {
        // found more peaks than requested, remove excess peaks
        // sort with descending peak level
        sort(peaks.begin(), peaks.end(), [](peakInfo higher, peakInfo lower) {
           return higher.level_dbm >= lower.level_dbm;
        });
        // only keep the requested number of peaks
        peaks.resize(maxPeaks);
        // sort again with ascending frequencies
        sort(peaks.begin(), peaks.end(), [](peakInfo lower, peakInfo higher) {
           return higher.frequency >= lower.frequency;
        });
    }
    vector<double> frequencies;
    for(auto p : peaks) {
        frequencies.push_back(p.frequency);
    }
    return frequencies;
}

Trace::Data Trace::sample(unsigned int index, SampleType type)
{
    auto data = lastMath->getSample(index);
    if(type == SampleType::TimeStep) {
        // exchange impulse data with step data
        data.y = lastMath->getStepResponse(index);
    }
    return data;
}

QString Trace::getTouchstoneFilename() const
{
    return touchstoneFilename;
}

void Trace::setTouchstoneFilename(const QString &value)
{
    touchstoneFilename = value;
}

unsigned int Trace::getTouchstoneParameter() const
{
    return touchstoneParameter;
}

double Trace::getNoise(double frequency)
{
    if(!isLive() || !settings.valid || (_liveParam != LiveParameter::Port1 && _liveParam != LiveParameter::Port2) || lastMath->getDataType() != DataType::Frequency) {
        // data not suitable for noise calculation
        return std::numeric_limits<double>::quiet_NaN();
    }
    // convert to dbm
    auto dbm = 20*log10(abs(lastMath->getInterpolatedSample(frequency).y));
    // convert to 1Hz bandwidth
    dbm -= 10*log10(settings.SA.RBW);
    return dbm;
}

int Trace::index(double x)
{
    auto lower = lower_bound(lastMath->rData().begin(), lastMath->rData().end(), x, [](const Data &lhs, const double x) -> bool {
        return lhs.x < x;
    });
    return lower - lastMath->rData().begin();
}

void Trace::setTouchstoneParameter(int value)
{
    touchstoneParameter = value;
}
