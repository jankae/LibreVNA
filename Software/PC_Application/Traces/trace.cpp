#include "trace.h"
#include <math.h>
#include "fftcomplex.h"

using namespace std;

Trace::Trace(QString name, QColor color, LiveParameter live)
    : tdr_users(0),
      _name(name),
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
    emit outputSamplesChanged(lower - data.begin(), lower - data.begin() + 1);
    if(lower == data.begin()) {
        // received the first point, which means the last sweep just finished
        if(tdr_users) {
            updateTimeDomainData();
        }
    }
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
//#include <iostream>
//#include <chrono>

void Trace::updateTimeDomainData()
{
    if(data.size() < 2) {
        // can't compute anything
        timeDomain.clear();
        return;
    }
//    using namespace std::chrono;
//    auto starttime = duration_cast< milliseconds >(
//        system_clock::now().time_since_epoch()
//    ).count();
    auto steps = size();
    auto firstStep = minFreq();
    if(firstStep == 0) {
        // zero as first step would result in infinite number of points, skip and start with second
        firstStep = lastMath->rData()[1].x;
        steps--;
    }
    if(firstStep * steps != maxFreq()) {
        // data is not available with correct frequency spacing, calculate required steps
        steps = maxFreq() / firstStep;
    }
    const double PI = 3.141592653589793238463;
    // reserve vector for negative frequenies and DC as well
    vector<complex<double>> frequencyDomain(2*steps + 1);
    // copy frequencies, use the flipped conjugate for negative part
    for(unsigned int i = 1;i<=steps;i++) {
        auto S = getData(firstStep * i);
        constexpr double alpha0 = 0.54;
        auto hamming = alpha0 - (1.0 - alpha0) * -cos(PI * i / steps);
        S *= hamming;
        frequencyDomain[2 * steps - i + 1] = conj(S);
        frequencyDomain[i] = S;
    }
    // use simple extrapolation from lowest two points to extract DC value
    auto abs_DC = 2.0 * abs(frequencyDomain[1]) - abs(frequencyDomain[2]);
    auto phase_DC = 2.0 * arg(frequencyDomain[1]) - arg(frequencyDomain[2]);
    frequencyDomain[0] = polar(abs_DC, phase_DC);

    auto fft_bins = frequencyDomain.size();
    timeDomain.clear();
    timeDomain.resize(fft_bins);
    const double fs = 1.0 / (firstStep * fft_bins);
    double last_step = 0.0;

    Fft::transform(frequencyDomain, true);
    constexpr double c = 299792458;
    for(unsigned int i = 0;i<fft_bins;i++) {
        TimedomainData t;
        t.time = fs * i;
        t.distance = t.time * c * 0.66; // TODO user settable velocity factor
        if(isReflection()) {
            t.distance /= 2;
        }
        t.impulseResponse = real(frequencyDomain[i]) / fft_bins;
        t.stepResponse = last_step;
        if(abs(t.stepResponse) < 1.0) {
            t.impedance = 50.0 * (1+t.stepResponse) / (1-t.stepResponse);
        } else {
            t.impedance = numeric_limits<double>::quiet_NaN();
        }
        last_step += t.impulseResponse;
        timeDomain[i] = t;
    }
//    auto duration = duration_cast< milliseconds >(
//        system_clock::now().time_since_epoch()
//    ).count() - starttime;
    //    cout << "TDR: " << this << " (took " << duration << "ms)" <<endl;
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
    }
}

void Trace::setReflection(bool value)
{
    reflection = value;
}

void Trace::addTDRinterest()
{
    if(tdr_users == 0) {
        // no recent time domain data available, calculate now
        updateTimeDomainData();
    }
    tdr_users++;
    if(tdr_users == 1) {
        emit changedTDRstate(true);
    }
}

void Trace::removeTDRinterest()
{
    if(tdr_users > 0) {
        tdr_users--;
        if(tdr_users == 0) {
            emit changedTDRstate(false);
        }
    }
}

Trace::TimedomainData Trace::getTDR(double position)
{
    TimedomainData ret = {};
    if(!TDRactive() || position < 0) {
        return ret;
    }
    int index = 0;
    bool exact = false;
    double alpha = 0.0;
    if(position <= timeDomain.back().time) {
        auto lower = lower_bound(timeDomain.begin(), timeDomain.end(), position, [](const TimedomainData &lhs, const double pos) -> bool {
            return lhs.time < pos;
        });
        index = lower - timeDomain.begin();
        if(timeDomain.at(index).time == position) {
            exact = true;
        } else {
            alpha = (position - timeDomain.at(index-1).time) / (timeDomain.at(index).time - timeDomain.at(index-1).time);
        }
    } else {
        if(position > timeDomain.back().distance) {
            // too high, invalid position
            return ret;
        }
        auto lower = lower_bound(timeDomain.begin(), timeDomain.end(), position, [](const TimedomainData &lhs, const double pos) -> bool {
            return lhs.distance < pos;
        });
        index = lower - timeDomain.begin();
        if(timeDomain.at(index).distance == position) {
            exact = true;
        } else {
            alpha = (position - timeDomain.at(index-1).distance) / (timeDomain.at(index).distance - timeDomain.at(index-1).distance);
        }
    }
    if(exact) {
        return timeDomain.at(index);
    } else {
        // need to interpolate
        auto low = timeDomain.at(index-1);
        auto high = timeDomain.at(index);
        ret.time = low.time * (1.0 - alpha) + high.time * alpha;
        ret.distance = low.distance * (1.0 - alpha) + high.distance * alpha;
        ret.stepResponse = low.stepResponse * (1.0 - alpha) + high.stepResponse * alpha;
        ret.impulseResponse = low.impulseResponse * (1.0 - alpha) + high.impulseResponse * alpha;
        ret.impedance = low.impedance * (1.0 - alpha) + high.impedance * alpha;
        return ret;
    }
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

double Trace::minFreq()
{
    if(size() > 0) {
        return data.front().x;
    } else {
        return 0.0;
    }
}

double Trace::maxFreq()
{
    if(size() > 0) {
        return data.back().x;
    } else {
        return 0.0;
    }
}

double Trace::findExtremumFreq(bool max)
{
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

Trace::Data Trace::sample(unsigned int index)
{
    return lastMath->getSample(index);
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

std::complex<double> Trace::getData(double frequency)
{
    if(lastMath->numSamples() == 0 || frequency < minFreq() || frequency > maxFreq()) {
        return std::numeric_limits<std::complex<double>>::quiet_NaN();
    }

    auto i = index(frequency);
    if(lastMath->getSample(i).x == frequency) {
        return lastMath->getSample(i).y;
    } else {
        // no exact frequency match, needs to interpolate
        auto high = lastMath->getSample(i);
        auto low = lastMath->getSample(i - 1);
        double alpha = (frequency - low.x) / (high.x - low.x);
        return low.y * (1 - alpha) + high.y * alpha;
    }
}

double Trace::getNoise(double frequency)
{
    if(!isLive() || !settings.valid || (_liveParam != LiveParameter::Port1 && _liveParam != LiveParameter::Port2)) {
        // data not suitable for noise calculation
        return std::numeric_limits<double>::quiet_NaN();
    }
    // convert to dbm
    auto dbm = 20*log10(abs(getData(frequency)));
    // convert to 1Hz bandwidth
    dbm -= 10*log10(settings.SA.RBW);
    return dbm;
}

int Trace::index(double frequency)
{
    auto lower = lower_bound(lastMath->rData().begin(), lastMath->rData().end(), frequency, [](const Data &lhs, const double freq) -> bool {
        return lhs.x < freq;
    });
    return lower - lastMath->rData().begin();
}

void Trace::setTouchstoneParameter(int value)
{
    touchstoneParameter = value;
}
