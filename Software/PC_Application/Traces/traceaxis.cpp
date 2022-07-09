#include "traceaxis.h"
#include "Util/util.h"

#include <cmath>

using namespace std;

static void createEvenlySpacedTicks(vector<double>& ticks, double start, double stop, double step) {
    ticks.clear();
    if(start > stop) {
        swap(start, stop);
    }
    step = abs(step);
    constexpr unsigned int maxTicks = 100;
    for(double tick = start; tick - stop < numeric_limits<double>::epsilon() && ticks.size() <= maxTicks;tick+= step) {
        ticks.push_back(tick);
    }
}

static double createAutomaticTicks(vector<double>& ticks, double start, double stop, int minDivisions) {
    Q_ASSERT(stop > start);
    ticks.clear();
    double max_div_step = (stop - start) / minDivisions;
    int zeros = floor(log10(max_div_step));
    double decimals_shift = pow(10, zeros);
    max_div_step /= decimals_shift;
    if(max_div_step >= 5) {
        max_div_step = 5;
    } else if(max_div_step >= 2) {
        max_div_step = 2;
    } else {
        max_div_step = 1;
    }
    auto div_step = max_div_step * decimals_shift;
    // round min up to next multiple of div_step
    auto start_div = ceil(start / div_step) * div_step;
    for(double tick = start_div;tick <= stop;tick += div_step) {
        ticks.push_back(tick);
    }
    return div_step;
}

static void createLogarithmicTicks(vector<double>& ticks, double start, double stop, int minDivisions) {
    // enforce usable log settings
    if(start <= 0) {
        start = 1.0;
    }
    if(stop <= start) {
        stop = start + 1.0;
    }
    ticks.clear();

    auto decades = log10(stop) - log10(start);
    double max_div_decade = minDivisions / decades;
    int zeros = floor(log10(max_div_decade));
    double decimals_shift = pow(10, zeros);
    max_div_decade /= decimals_shift;
    if(max_div_decade < 2) {
        max_div_decade = 2;
    } else if(max_div_decade < 5) {
        max_div_decade = 5;
    } else {
        max_div_decade = 10;
    }
    auto step = pow(10, floor(log10(start))+1) / (max_div_decade * decimals_shift);
    // round min up to next multiple of div_step
    auto div = ceil(start / step) * step;
    if(floor(log10(div)) != floor(log10(start))) {
        // first div is already at the next decade
        step *= 10;
    }
    do {
        ticks.push_back(div);
        if(ticks.size() > 1 && div != step && floor(log10(div)) != floor(log10(div - step))) {
            // reached a new decade with this switch
            step *= 10;
            div = step;
        } else {
            div += step;
        }
    } while(div <= stop);
}

YAxis::YAxis()
{
    type = Type::Magnitude;
}

double YAxis::sampleToCoordinate(Trace::Data data, Trace *t, unsigned int sample)
{
    switch(type) {
    case YAxis::Type::Magnitude:
        return Util::SparamTodB(data.y);
    case YAxis::Type::MagnitudedBuV:
        return Util::dBmTodBuV(Util::SparamTodB(data.y));
    case YAxis::Type::MagnitudeLinear:
        return abs(data.y);
    case YAxis::Type::Phase:
        return Util::SparamToDegree(data.y);
    case YAxis::Type::UnwrappedPhase:
        if(!t) {
            return 0.0;
        }
        return t->getUnwrappedPhase(sample) * 180.0 / M_PI;
    case YAxis::Type::VSWR:
        return Util::SparamToVSWR(data.y);
    case YAxis::Type::Real:
        return data.y.real();
    case YAxis::Type::Imaginary:
        return data.y.imag();
    case YAxis::Type::SeriesR:
        return Util::SparamToResistance(data.y, t->getReferenceImpedance());
    case YAxis::Type::Reactance:
        return Util::SparamToImpedance(data.y, t->getReferenceImpedance()).imag();
    case YAxis::Type::Capacitance:
        return Util::SparamToCapacitance(data.y, data.x, t->getReferenceImpedance());
    case YAxis::Type::Inductance:
        return Util::SparamToInductance(data.y, data.x);
    case YAxis::Type::QualityFactor:
        return Util::SparamToQualityFactor(data.y);
    case YAxis::Type::GroupDelay: {
        constexpr int requiredSamples = 5;
        if(!t || t->size() < requiredSamples) {
            // unable to calculate
            return 0.0;

        }
        // needs at least some samples before/after current sample for calculating the derivative.
        // For samples too far at either end of the trace, return group delay of "inner" trace sample instead
        if(sample < requiredSamples / 2) {
            return sampleToCoordinate(data, t, requiredSamples / 2);
        } else if(sample >= t->size() - requiredSamples / 2) {
            return sampleToCoordinate(data, t, t->size() - requiredSamples / 2 - 1);
        } else {
            // got enough samples at either end to calculate derivative.
            // acquire phases of the required samples
            std::vector<double> phases;
            phases.reserve(requiredSamples);
            for(unsigned int index = sample - requiredSamples / 2;index <= sample + requiredSamples / 2;index++) {
                phases.push_back(arg(t->sample(index).y));
            }
            // make sure there are no phase jumps
            Util::unwrapPhase(phases);
            // calculate linearRegression to get derivative
            double B_0, B_1;
            Util::linearRegression(phases, B_0, B_1);
            // B_1 now contains the derived phase vs. the sample. Scale by frequency to get group delay
            double freq_step = t->sample(sample).x - t->sample(sample - 1).x;
            return -B_1 / (2.0*M_PI * freq_step);
        }
    }
    case YAxis::Type::ImpulseReal:
        return real(data.y);
    case YAxis::Type::ImpulseMag:
        return Util::SparamTodB(data.y);
    case YAxis::Type::Step:
        if(!t) {
            return 0.0;
        }
        return t->sample(sample, true).y.real();
    case YAxis::Type::Impedance: {
        if(!t) {
            return 0.0;
        }
        double step = t->sample(sample, true).y.real();
        if(abs(step) < 1.0) {
            return Util::SparamToImpedance(step, t->getReferenceImpedance()).real();
        }
    }
        break;
    case YAxis::Type::Disabled:
    case YAxis::Type::Last:
        // no valid axis
        break;
    }
    return 0.0;
}

void YAxis::set(Type type, bool log, bool autorange, double min, double max, double div)
{
    this->type = type;
    this->log = log;
    this->autorange = autorange;
    this->rangeMin = min;
    this->rangeMax = max;
    this->rangeDiv = div;
    if(type != Type::Disabled) {
        updateTicks();
    }
}

QString YAxis::TypeToName(Type type)
{
    switch(type) {
    case Type::Disabled: return "Disabled";
    case Type::Magnitude: return "Magnitude";
    case Type::MagnitudedBuV: return "Magnitude (dBuV)";
    case Type::MagnitudeLinear: return "Magnitude (linear)";
    case Type::Phase: return "Phase";
    case Type::UnwrappedPhase: return "Unwrapped Phase";
    case Type::VSWR: return "VSWR";
    case Type::Real: return "Real";
    case Type::Imaginary: return "Imaginary";
    case Type::SeriesR: return "Resistance";
    case Type::Reactance: return "Reactance";
    case Type::Capacitance: return "Capacitance";
    case Type::Inductance: return "Inductance";
    case Type::QualityFactor: return "Quality Factor";
    case Type::GroupDelay: return "Group delay";
    case Type::ImpulseReal: return "Impulse Response (Real)";
    case Type::ImpulseMag: return "Impulse Response (Magnitude)";
    case Type::Step: return "Step Response";
    case Type::Impedance: return "Impedance";
    case Type::Last: return "Unknown";
    }
    return "Missing case";
}

YAxis::Type YAxis::TypeFromName(QString name)
{
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(TypeToName((Type) i) == name) {
            return (Type) i;
        }
    }
    // not found, use default
    return Type::Magnitude;

}

QString YAxis::Unit(Type type, TraceModel::DataSource source)
{
    if(source == TraceModel::DataSource::VNA) {
        switch(type) {
        case Type::Magnitude: return "dB";
        case Type::MagnitudeLinear: return "";
        case Type::Phase: return "°";
        case Type::UnwrappedPhase: return "°";
        case Type::VSWR: return "";
        case Type::ImpulseReal: return "";
        case Type::ImpulseMag: return "dB";
        case Type::Step: return "";
        case Type::Impedance: return "Ω";
        case Type::GroupDelay: return "s";
        case Type::Disabled:
        case Type::Real:
        case Type::Imaginary:
        case Type::QualityFactor:
            return "";
        case Type::SeriesR: return "Ω";
        case Type::Reactance: return "Ω";
        case Type::Capacitance: return "F";
        case Type::Inductance: return "H";
        default: return "";
        }
    } else if(source == TraceModel::DataSource::SA) {
        switch(type) {
        case Type::Magnitude: return "dBm";
        case Type::MagnitudedBuV: return "dBuV";
        default: return "";
        }
    }
    return "";
}

QString YAxis::Prefixes(Type type, TraceModel::DataSource source)
{
    if(source == TraceModel::DataSource::VNA) {
        switch(type) {
        case Type::Magnitude: return " ";
        case Type::MagnitudeLinear: return "num ";
        case Type::Phase: return " ";
        case Type::UnwrappedPhase: return " ";
        case Type::VSWR: return " ";
        case Type::ImpulseReal: return "pnum kMG";
        case Type::ImpulseMag: return " ";
        case Type::Step: return "pnum kMG";
        case Type::Impedance: return "m kM";
        case Type::GroupDelay: return "pnum ";
        case Type::Disabled: return " ";
        case Type::Real: return "pnum ";
        case Type::Imaginary: return "pnum ";
        case Type::QualityFactor: return " ";
        case Type::SeriesR: return "m kM";
        case Type::Reactance: return "m kM";
        case Type::Capacitance: return "pnum ";
        case Type::Inductance: return "pnum ";
        default: return " ";
        }
    } else if(source == TraceModel::DataSource::SA) {
        switch(type) {
        case Type::Magnitude: return " ";
        case Type::MagnitudedBuV: return " ";
        default: return " ";
        }
    }
    return " ";
}

QString YAxis::TypeToName()
{
    return TypeToName(type);
}

QString YAxis::Unit(TraceModel::DataSource source)
{
    return Unit(type, source);
}

QString YAxis::Prefixes(TraceModel::DataSource source)
{
    return Prefixes(type, source);
}

YAxis::Type YAxis::getType() const
{
    return type;
}

std::set<YAxis::Type> YAxis::getSupported(XAxis::Type type, TraceModel::DataSource source)
{
    std::set<YAxis::Type> ret = {YAxis::Type::Disabled};
    if(source == TraceModel::DataSource::VNA) {
        switch(type) {
        case XAxis::Type::Frequency:
        case XAxis::Type::Power:
        case XAxis::Type::TimeZeroSpan:
            ret.insert(YAxis::Type::Magnitude);
            ret.insert(YAxis::Type::MagnitudeLinear);
            ret.insert(YAxis::Type::Phase);
            ret.insert(YAxis::Type::UnwrappedPhase);
            ret.insert(YAxis::Type::VSWR);
            ret.insert(YAxis::Type::Real);
            ret.insert(YAxis::Type::Imaginary);
            ret.insert(YAxis::Type::SeriesR);
            ret.insert(YAxis::Type::Reactance);
            ret.insert(YAxis::Type::Capacitance);
            ret.insert(YAxis::Type::Inductance);
            ret.insert(YAxis::Type::QualityFactor);
            ret.insert(YAxis::Type::GroupDelay);
            break;
        case XAxis::Type::Time:
        case XAxis::Type::Distance:
            ret.insert(YAxis::Type::ImpulseReal);
            ret.insert(YAxis::Type::ImpulseMag);
            ret.insert(YAxis::Type::Step);
            ret.insert(YAxis::Type::Impedance);
            break;
        default:
            break;
        }
    } else if(source == TraceModel::DataSource::SA) {
        switch(type) {
        case XAxis::Type::Frequency:
            ret.insert(YAxis::Type::Magnitude);
            ret.insert(YAxis::Type::MagnitudedBuV);
            break;
        default:
            break;
        }
    }
    return ret;
}

std::complex<double> YAxis::reconstructValueFromYAxisType(std::map<YAxis::Type, double> yaxistypes)
{
    std::complex<double> ret = std::numeric_limits<std::complex<double>>::quiet_NaN();
    if(yaxistypes.count(Type::Real)) {
        ret.real(yaxistypes[Type::Real]);
        if(yaxistypes.count(Type::Imaginary)) {
            ret.imag(yaxistypes[Type::Imaginary]);
        } else {
            ret.imag(0.0);
        }
    } else if(yaxistypes.count(Type::Magnitude) || yaxistypes.count(Type::MagnitudedBuV) || yaxistypes.count(Type::MagnitudeLinear)) {
        double maglin, phase;
        if(yaxistypes.count(Type::MagnitudeLinear)) {
            maglin = yaxistypes[Type::MagnitudeLinear];
        } else if(yaxistypes.count(Type::Magnitude)) {
            maglin = Util::dBToMagnitude(yaxistypes[Type::Magnitude]);
        } else {
            auto dBm = Util::dBuVTodBm(yaxistypes[Type::MagnitudedBuV]);
            maglin = Util::dBToMagnitude(dBm);
        }
        if(yaxistypes.count(Type::Phase)) {
            phase = yaxistypes[Type::Phase];
        } else {
            phase = 0.0;
        }
        ret = polar<double>(maglin, phase / 180.0 * M_PI);
    }
    return ret;
}

bool XAxis::isSupported(XAxis::Type type, TraceModel::DataSource source)
{
    if(source == TraceModel::DataSource::VNA) {
        // all X axis types are supported
        return true;
    } else if(source == TraceModel::DataSource::SA) {
        if (type == XAxis::Type::Frequency) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void Axis::updateTicks()
{
    if(log) {
        createLogarithmicTicks(ticks, rangeMin, rangeMax, 20);
    } else if(autorange) {
        if(rangeMin >= rangeMax) {
            // problem, min must be less than max
            rangeMin -= 1.0;
            rangeMax += 1.0;
        }
        rangeDiv = createAutomaticTicks(ticks, rangeMin, rangeMax, 8);
    } else {
        createEvenlySpacedTicks(ticks, rangeMin, rangeMax, rangeDiv);
    }
}

const std::vector<double> &Axis::getTicks() const
{
    return ticks;
}

double Axis::getRangeDiv() const
{
    return rangeDiv;
}

double Axis::getRangeMax() const
{
    return rangeMax;
}

double Axis::getRangeMin() const
{
    return rangeMin;
}

bool Axis::getAutorange() const
{
    return autorange;
}

bool Axis::getLog() const
{
    return log;
}

XAxis::XAxis()
{
    type = Type::Frequency;
}

double XAxis::sampleToCoordinate(Trace::Data data, Trace *t, unsigned int sample)
{
    Q_UNUSED(sample)
    switch(type) {
    case Type::Distance:
        if(!t) {
            return 0.0;
        }
        return t->timeToDistance(data.x);
    default:
        return data.x;
    }
}

void XAxis::set(Type type, bool log, bool autorange, double min, double max, double div)
{
    this->type = type;
    this->log = log;
    this->autorange = autorange;
    this->rangeMin = min;
    this->rangeMax = max;
    this->rangeDiv = div;
    updateTicks();
}

QString XAxis::TypeToName(Type type)
{
    switch(type) {
    case Type::Frequency: return "Frequency";
    case Type::Time: return "Time";
    case Type::Distance: return "Distance";
    case Type::Power: return "Power";
    case Type::TimeZeroSpan: return "Time (Zero Span)";
    default: return "Unknown";
    }
}

XAxis::Type XAxis::TypeFromName(QString name)
{
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(TypeToName((Type) i) == name) {
            return (Type) i;
        }
    }
    // not found, use default
    return Type::Frequency;
}

QString XAxis::Unit(Type type)
{
    switch(type) {
    case Type::Frequency: return "Hz";
    case Type::Time: return "s";
    case Type::Distance: return "m";
    case Type::Power: return "dBm";
    case Type::TimeZeroSpan: return "s";
    default: return "";
    }
}

QString XAxis::TypeToName()
{
    return TypeToName(type);
}

QString XAxis::Unit()
{
    return Unit(type);
}

XAxis::Type XAxis::getType() const
{
    return type;
}

Axis::Axis()
{
    log = false;
    autorange = true;
    rangeMin = -1.0;
    rangeMax = 1.0;
    rangeDiv = 1.0;
    ticks.clear();
}

double Axis::transform(double value, double to_low, double to_high)
{
    return Util::Scale(value, rangeMin, rangeMax, to_low, to_high, log);
}

double Axis::inverseTransform(double value, double to_low, double to_high)
{
    return Util::Scale(value, to_low, to_high, rangeMin, rangeMax, false, log);
}
