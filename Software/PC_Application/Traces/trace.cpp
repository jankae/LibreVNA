#include "trace.h"

#include "fftcomplex.h"
#include "Util/util.h"
#include "Marker/marker.h"
#include "traceaxis.h"

#include <math.h>
#include <QDebug>
#include <QScrollBar>
#include <QSettings>
#include <functional>

using namespace std;

Trace::Trace(QString name, QColor color, LiveParameter live)
    : _name(name),
      _color(color),
      _liveType(LivedataType::Overwrite),
      _liveParam(live),
      vFactor(0.66),
      reflection(true),
      reference_impedance(50.0),
      visible(true),
      paused(false),
      createdFromFile(false),
      calibration(false),
      domain(DataType::Frequency),
      lastMath(nullptr)
{
    MathInfo self = {.math = this, .enabled = true};
    mathOps.push_back(self);
    updateLastMath(mathOps.rbegin());

    self.enabled = false;
    dataType = DataType::Frequency;
    connect(this, &Trace::typeChanged, [=](){
        dataType = domain;
        emit outputTypeChanged(dataType);
    });
    connect(this, &Trace::outputSamplesChanged, [=](unsigned int begin, unsigned int end){
        Q_UNUSED(end);
        // some samples changed, delete unwrapped phases from here until the end
        if(unwrappedPhase.size() > begin) {
            unwrappedPhase.resize(begin);
        }
    });
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

void Trace::addData(const Trace::Data& d, DataType domain, double reference_impedance, int index) {
    if(this->domain != domain) {
        clear();
        this->domain = domain;
        emit typeChanged(this);
    }
    if(index >= 0) {
        // index position specified
        if(data.size() <= index) {
            data.resize(index + 1);
        }
        data[index] = d;
    } else {
        // no index given, determine position by X-coordinate

        // add or replace data in vector while keeping it sorted with increasing frequency
        auto lower = lower_bound(data.begin(), data.end(), d, [](const Data &lhs, const Data &rhs) -> bool {
            return lhs.x < rhs.x;
        });
        // calculate index now because inserting a sample into data might lead to reallocation -> arithmetic on lower not valid anymore
        index = lower - data.begin();
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
            default: break;
            }
        } else {
            // insert at this position
            data.insert(lower, d);
        }
    }
    if(this->reference_impedance != reference_impedance) {
        this->reference_impedance = reference_impedance;
        emit typeChanged(this);
    }
    success();
    emit outputSamplesChanged(index, index + 1);
}

void Trace::addData(const Trace::Data &d, const Protocol::SpectrumAnalyzerSettings &s, int index)
{
    settings.SA = s;
    settings.valid = true;
    auto domain = DataType::Frequency;
    if (s.f_start == s.f_stop) {
        // in zerospan mode
        domain = DataType::TimeZeroSpan;
    }
    addData(d, domain, 50.0, index);
}

void Trace::setName(QString name) {
    _name = name;
    emit nameChanged();
}

void Trace::setVelocityFactor(double v)
{
    vFactor = v;
}

void Trace::fillFromTouchstone(Touchstone &t, unsigned int parameter)
{
    if(parameter >= t.ports()*t.ports()) {
        throw runtime_error("Parameter for touchstone out of range");
    }
    clear();
    domain = DataType::Frequency;
    fileParameter = parameter;
    filename = t.getFilename();
    for(unsigned int i=0;i<t.points();i++) {
        auto tData = t.point(i);
        Data d;
        d.x = tData.frequency;
        d.y = t.point(i).S[parameter];
        addData(d, DataType::Frequency);
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
    createdFromFile = true;
    reference_impedance = t.getReferenceImpedance();
    emit typeChanged(this);
    emit outputSamplesChanged(0, data.size());
}

QString Trace::fillFromCSV(CSV &csv, unsigned int parameter)
{
    // find correct column
    int traceNum = -1;
    unsigned int i=1;
    QString lastTraceName = "";
    bool hasImagValues;
    std::map<YAxis::Type, int> columnMapping;
    for(;i<csv.columns();i++) {
        auto header = csv.getHeader(i);
        auto splitIndex = header.lastIndexOf("_");
        if(splitIndex == -1) {
            // no "_", not following naming format of CSV export, skip
            continue;
        }
        auto traceName = header.left(splitIndex);
        auto yaxistype = header.right(header.size() - splitIndex - 1);
        if(traceName != lastTraceName) {
            traceNum++;
            if(traceNum > parameter) {
                // got all columns for the trace we are interested in
                break;
            }
            lastTraceName = traceName;
        }
        if(traceNum == parameter) {
            // this is the trace we are looking for, get axistype and add to mapping

            // handle legacy column naming, translate to new naming
            if(yaxistype == "real") {
                yaxistype = YAxis::TypeToName(YAxis::Type::Real);
            } else if(yaxistype == "imag") {
                yaxistype = YAxis::TypeToName(YAxis::Type::Imaginary);
            }

            columnMapping[YAxis::TypeFromName(yaxistype)] = i;
        }
    }
    if(traceNum < parameter) {
        throw runtime_error("Not enough traces in CSV file");
    }
    if(columnMapping.size() == 0) {
        throw runtime_error("No data for trace in CSV file");
    }

    clear();
    fileParameter = parameter;
    filename = csv.getFilename();
    auto xColumn = csv.getColumn(0);
    if(csv.getHeader(0).compare("time", Qt::CaseInsensitive) == 0) {
        domain = DataType::Time;
    } else if(csv.getHeader(0).compare("power", Qt::CaseInsensitive) == 0) {
        domain = DataType::Power;
    } else if(csv.getHeader(0).compare("time (zero span)", Qt::CaseInsensitive) == 0) {
        domain = DataType::TimeZeroSpan;
    } else {
        domain = DataType::Frequency;
    }
    for(unsigned int i=0;i<xColumn.size();i++) {
        std::map<YAxis::Type, double> data;
        for(auto map : columnMapping) {
            data[map.first] = csv.getColumn(map.second)[i];
        }
        Data d;
        d.x = xColumn[i];
        d.y = YAxis::reconstructValueFromYAxisType(data);
        addData(d, domain);
    }
    reflection = false;
    createdFromFile = true;
    emit typeChanged(this);
    emit outputSamplesChanged(0, data.size());
    return lastTraceName;
}

void Trace::fillFromDatapoints(Trace &S11, Trace &S12, Trace &S21, Trace &S22, const std::vector<VNAData> &data)
{
    S11.clear();
    S12.clear();
    S21.clear();
    S22.clear();
    for(auto d : data) {
        Trace::Data td;
        td.x = d.frequency;
        td.y = d.S.m11;
        S11.addData(td, DataType::Frequency);
        td.y = d.S.m12;
        S12.addData(td, DataType::Frequency);
        td.y = d.S.m21;
        S21.addData(td, DataType::Frequency);
        td.y = d.S.m22;
        S22.addData(td, DataType::Frequency);
    }
}

void Trace::fromLivedata(Trace::LivedataType type, LiveParameter param)
{
    createdFromFile = false;
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

void Trace::addMarker(Marker *m)
{
    markers.insert(m);
    connect(m, &Marker::dataFormatChanged, this, &Trace::markerFormatChanged);
    connect(m, &Marker::visibilityChanged, this, &Trace::markerVisibilityChanged);
    emit markerAdded(m);
}

void Trace::removeMarker(Marker *m)
{
    disconnect(m, &Marker::dataFormatChanged, this, &Trace::markerFormatChanged);
    disconnect(m, &Marker::visibilityChanged, this, &Trace::markerVisibilityChanged);
    markers.erase(m);
    emit markerRemoved(m);
}

void Trace::markerVisibilityChanged(Marker *m)
{
    // trigger replot by pretending that trace visibility also changed
    emit visibilityChanged(this);
}

double Trace::getReferenceImpedance() const
{
    return reference_impedance;
}

const std::vector<Trace::MathInfo>& Trace::getMathOperations() const
{
    return mathOps;
}

double Trace::velocityFactor()
{
    return vFactor;
}

double Trace::timeToDistance(double time)
{
    double c = 299792458;
    auto distance = time * c * velocityFactor();
    if(isReflection()) {
        distance /= 2.0;
    }
    return distance;
}

double Trace::distanceToTime(double distance)
{
    double c = 299792458;
    auto time = distance / (c * velocityFactor());
    if(isReflection()) {
        time *= 2.0;
    }
    return time;
}

nlohmann::json Trace::toJSON()
{
    nlohmann::json j;
    if(isCalibration()) {
        // calibration traces can't be saved
        return j;
    }
    j["name"] = _name.toStdString();
    j["color"] = _color.name().toStdString();
    j["visible"] = visible;
    if(isLive()) {
        j["type"] = "Live";
        j["parameter"] = _liveParam;
        j["livetype"] = _liveType;
        j["paused"] = paused;
    } else if(isFromFile()) {
        j["type"] = "File";
        j["filename"] = filename.toStdString();
        j["parameter"] = fileParameter;
    }
    j["velocityFactor"] = vFactor;
    j["reflection"] = reflection;

    nlohmann::json mathList;
    for(auto m : mathOps) {
        if(m.math->getType() == Type::Last) {
            // this is an invalid type reserved for the trace itself, skip
            continue;
        }
        nlohmann::json jm;
        auto info = TraceMath::getInfo(m.math->getType());
        jm["operation"] = info.name.toStdString();
        jm["enabled"] = m.enabled;
        jm["settings"] = m.math->toJSON();
        mathList.push_back(jm);
    }
    j["math"] = mathList;
    j["math_enabled"] = mathEnabled();

    return j;
}

void Trace::fromJSON(nlohmann::json j)
{
    createdFromFile = false;
    calibration = false;
    _name = QString::fromStdString(j.value("name", "Missing name"));
    _color = QColor(QString::fromStdString(j.value("color", "yellow")));
    visible = j.value("visible", true);
    auto type = QString::fromStdString(j.value("type", "Live"));
    if(type == "Live") {
        _liveParam = j.value("parameter", LiveParameter::S11);
        _liveType = j.value("livetype", LivedataType::Overwrite);
        paused = j.value("paused", false);
    } else if(type == "Touchstone" || type == "File") {
        auto filename = QString::fromStdString(j.value("filename", ""));
        fileParameter = j.value("parameter", 0);
        try {
            if(filename.endsWith(".csv")) {
                auto csv = CSV::fromFile(filename);
                fillFromCSV(csv, fileParameter);
            } else {
                // has to be a touchstone file
                Touchstone t = Touchstone::fromFile(filename.toStdString());
                fillFromTouchstone(t, fileParameter);
            }
        } catch (const exception &e) {
            std::string what = e.what();
            throw runtime_error("Failed to create from file:" + what);
        }
    }
    vFactor = j.value("velocityFactor", 0.66);
    reflection = j.value("reflection", false);
    for(auto jm : j["math"]) {
        QString operation = QString::fromStdString(jm.value("operation", ""));
        if(operation.isEmpty()) {
            qWarning() << "Skipping empty math operation";
            continue;
        }
        // attempt to find the type of operation
        TraceMath::Type type = Type::Last;
        for(unsigned int i=0;i<(int) Type::Last;i++) {
            auto info = TraceMath::getInfo((Type) i);
            if(info.name == operation) {
                // found the correct operation
                type = (Type) i;
                break;
            }
        }
        if(type == Type::Last) {
            // unable to find this operation
            qWarning() << "Unable to create math operation:" << operation;
            continue;
        }
        qDebug() << "Creating math operation of type:" << operation;
        auto op = TraceMath::createMath(type)[0];
        if(jm.contains("settings")) {
            op->fromJSON(jm["settings"]);
        }
        MathInfo info;
        info.enabled = jm.value("enabled", true);
        info.math = op;
        op->assignInput(lastMath);
        mathOps.push_back(info);
        updateLastMath(mathOps.rbegin());
    }
    enableMath(j.value("math_enabled", true));
}

unsigned int Trace::toHash()
{
    // taking the easy way: create the json string and hash it (already contains all necessary information)
    // This is slower than it could be, but this function is only used when loading setups, so this isn't a big problem
    std::string json_string = toJSON().dump();
    return hash<std::string>{}(json_string);
}

std::vector<Trace *> Trace::createFromTouchstone(Touchstone &t)
{
    qDebug() << "Creating traces from touchstone...";
    std::vector<Trace*> traces;
    for(unsigned int i=0;i<t.ports()*t.ports();i++) {
        auto trace = new Trace();
        trace->fillFromTouchstone(t, i);
        unsigned int sink = i / t.ports() + 1;
        unsigned int source = i % t.ports() + 1;
        trace->setName("S"+QString::number(sink)+QString::number(source));
        traces.push_back(trace);
    }
    return traces;
}

std::vector<Trace *> Trace::createFromCSV(CSV &csv)
{
    qDebug() << "Creating traces from csv...";
    std::vector<Trace*> traces;
    auto n = csv.columns();
    if(n >= 2) {
        try {
            // This will throw once no more column is available, can use infinite loop
            unsigned int param = 0;
            while(1) {
                auto t = new Trace();
                auto name = t->fillFromCSV(csv, param);
                t->setName(name);
                param++;
                traces.push_back(t);
            }
        } catch (const exception &e) {
            // nothing to do, this simply means we reached the end of the csv columns
        }
    } else {
        qWarning() << "Unable to parse, not enough columns";
    }
    return traces;
}

std::vector<VNAData> Trace::assembleDatapoints(const Trace &S11, const Trace &S12, const Trace &S21, const Trace &S22)
{
    vector<VNAData> ret;

    // Sanity check traces
    unsigned int samples = S11.size();
    auto impedance = S11.getReferenceImpedance();
    vector<const Trace*> traces;
    traces.push_back(&S11);
    traces.push_back(&S12);
    traces.push_back(&S21);
    traces.push_back(&S22);
    vector<double> freqs;
    for(const auto t : traces) {
        if(t->size() != samples) {
            qWarning() << "Selected traces do not have the same size";
            return ret;
        }
        if(t->getReferenceImpedance() != impedance) {
            qWarning() << "Selected traces do not have the same reference impedance";
            return ret;
        }
        if(t->outputType() != Trace::DataType::Frequency) {
            qWarning() << "Selected trace not in frequency domain";
            return ret;
        }
        if(freqs.empty()) {
            // Create frequency vector
            for(unsigned int i=0;i<samples;i++) {
                freqs.push_back(t->sample(i).x);
            }
        } else {
            // Compare with frequency vector
            for(unsigned int i=0;i<samples;i++) {
                if(t->sample(i).x != freqs[i]) {
                    qWarning() << "Selected traces do not have identical frequency points";
                    return ret;
                }
            }
        }
    }

    // Checks passed, assemble datapoints
    for(unsigned int i=0;i<samples;i++) {
        Protocol::Datapoint d;
        d.real_S11 = real(S11.sample(i).y);
        d.imag_S11 = imag(S11.sample(i).y);
        d.real_S12 = real(S12.sample(i).y);
        d.imag_S12 = imag(S12.sample(i).y);
        d.real_S21 = real(S21.sample(i).y);
        d.imag_S21 = imag(S21.sample(i).y);
        d.real_S22 = real(S22.sample(i).y);
        d.imag_S22 = imag(S22.sample(i).y);
        d.pointNum = i;
        d.frequency = freqs[i];
        ret.push_back(d);
    }
    return ret;
}

Trace::LiveParameter Trace::ParameterFromString(QString s)
{
    s = s.toUpper();
    if(s == "S11") {
        return LiveParameter::S11;
    } else if(s == "S12") {
        return LiveParameter::S12;
    } else if(s == "S21") {
        return LiveParameter::S21;
    } else if(s == "S22") {
        return LiveParameter::S22;
    } else if(s == "PORT1") {
        return LiveParameter::Port1;
    } else if(s == "PORT2") {
        return LiveParameter::Port2;
    } else {
        return LiveParameter::Invalid;
    }
}

QString Trace::ParameterToString(LiveParameter p)
{
    switch(p) {
    case Trace::LiveParameter::S11: return "S11";
    case Trace::LiveParameter::S12: return "S12";
    case Trace::LiveParameter::S21: return "S21";
    case Trace::LiveParameter::S22: return "S22";
    case Trace::LiveParameter::Port1: return "Port1";
    case Trace::LiveParameter::Port2: return "Port2";
    default: return "Invalid";
    }
}

bool Trace::isVNAParameter(Trace::LiveParameter p)
{
    switch(p) {
    case Trace::LiveParameter::S11:
    case Trace::LiveParameter::S12:
    case Trace::LiveParameter::S21:
    case Trace::LiveParameter::S22:
        return true;
    case Trace::LiveParameter::Port1:
    case Trace::LiveParameter::Port2:
    default:
        return false;
    }
}

bool Trace::isSAParamater(Trace::LiveParameter p)
{
    switch(p) {
    case Trace::LiveParameter::S11:
    case Trace::LiveParameter::S12:
    case Trace::LiveParameter::S21:
    case Trace::LiveParameter::S22:
        return false;
    case Trace::LiveParameter::Port1:
    case Trace::LiveParameter::Port2:
        return true;
    default:
        return false;
    }
}

Trace::LivedataType Trace::TypeFromString(QString s)
{
    s = s.toUpper();
    if(s == "OVERWRITE") {
        return LivedataType::Overwrite;
    } else if(s == "MAXHOLD") {
        return LivedataType::MaxHold;
    } else if(s == "MINHOLD") {
        return LivedataType::MinHold;
    } else {
        return LivedataType::Invalid;
    }
}

QString Trace::TypeToString(Trace::LivedataType t)
{
    switch(t) {
    case Trace::LivedataType::Overwrite: return "Overwrite";
    case Trace::LivedataType::MaxHold: return "MaxHold";
    case Trace::LivedataType::MinHold: return "MinHold";
    default: return "Invalid";
    }
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

TraceMath::DataType Trace::outputType(TraceMath::DataType inputType)
{
    Q_UNUSED(inputType);
    return domain;
}

QString Trace::description()
{
    return name() + ": measured data";
}

void Trace::setCalibration(bool value)
{
    calibration = value;
}

std::set<Marker *> Trace::getMarkers() const
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
    if(!paused) {
        paused = true;
        emit pauseChanged();
    }
}

void Trace::resume()
{
    if(paused) {
        paused = false;
        emit pauseChanged();
    }
}

bool Trace::isPaused()
{
    return paused;
}

bool Trace::isFromFile()
{
    return createdFromFile;
}

bool Trace::isCalibration()
{
    return calibration;
}

bool Trace::isLive()
{
    return !isCalibration() && !isFromFile();
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

void Trace::addMathOperations(std::vector<TraceMath *> maths)
{
    TraceMath *input = lastMath;
    for(auto m : maths) {
        MathInfo info = {.math = m, .enabled = true};
        m->assignInput(input);
        input = m;
        mathOps.push_back(info);
    }
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

unsigned int Trace::size() const
{
    return lastMath->numSamples();
}

double Trace::minX()
{
    if(lastMath->numSamples() > 0) {
        return lastMath->rData().front().x;
    } else {
        return numeric_limits<double>::max();
    }
}

double Trace::maxX()
{
    if(lastMath->numSamples() > 0) {
        return lastMath->rData().back().x;
    } else {
        return numeric_limits<double>::lowest();
    }
}

double Trace::findExtremum(bool max)
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
        double dbm = Util::SparamTodB(d.y);
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

Trace::Data Trace::sample(unsigned int index, bool getStepResponse) const
{
    auto data = lastMath->getSample(index);
    if(outputType() == Trace::DataType::Time && getStepResponse) {
        // exchange impulse data with step data
        data.y = lastMath->getStepResponse(index);
    }
    return data;
}

double Trace::getUnwrappedPhase(unsigned int index)
{
    if(index >= size()) {
        return 0.0;
    } else if(index >= unwrappedPhase.size()) {
        // unwrapped phase not available for this entry, calculate
        // copy wrapped phases first
        unsigned int start_index = unwrappedPhase.size();
        unwrappedPhase.resize(index + 1);
        for(unsigned int i=start_index;i<=index;i++) {
            unwrappedPhase[i] = arg(lastMath->getSample(i).y);
        }
        // unwrap the updated part
        if(start_index > 0) {
            start_index--;
        }
        Util::unwrapPhase(unwrappedPhase, start_index);
    }
    return unwrappedPhase[index];
}

Trace::Data Trace::interpolatedSample(double x)
{
    auto data = lastMath->getInterpolatedSample(x);
    return data;
}

QString Trace::getFilename() const
{
    return filename;
}

unsigned int Trace::getFileParameter() const
{
    return fileParameter;
}

double Trace::getNoise(double frequency)
{
    if(!isLive() || !settings.valid || (_liveParam != LiveParameter::Port1 && _liveParam != LiveParameter::Port2) || lastMath->getDataType() != DataType::Frequency) {
        // data not suitable for noise calculation
        return std::numeric_limits<double>::quiet_NaN();
    }
    // convert to dbm
    auto dbm = Util::SparamTodB(lastMath->getInterpolatedSample(frequency).y);
    // convert to 1Hz bandwidth
    dbm -= 10*log10(settings.SA.RBW);
    return dbm;
}

int Trace::index(double x)
{
    auto lower = lower_bound(lastMath->rData().begin(), lastMath->rData().end(), x, [](const Data &lhs, const double x) -> bool {
        return lhs.x < x;
    });
    if(lower == lastMath->rData().end()) {
        // actually beyond the last sample, return the index of the last anyway to avoid access past data
        return lastMath->rData().size() - 1;
    }
    return lower - lastMath->rData().begin();
}
