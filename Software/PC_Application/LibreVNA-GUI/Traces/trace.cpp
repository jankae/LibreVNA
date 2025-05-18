#include "trace.h"

#include "fftcomplex.h"
#include "Util/util.h"
#include "Marker/marker.h"
#include "traceaxis.h"
#include "tracemodel.h"
#include "Math/parser/mpParser.h"
#include "preferences.h"

#include <math.h>
#include <QDebug>
#include <QScrollBar>
#include <QSettings>
#include <functional>

using namespace std;
using namespace mup;

Trace::Trace(QString name, QColor color, QString live)
    : model(nullptr),
      _name(name),
      _color(color),
      source(Source::Live),
      hash(0),
      hashSet(false),
      JSONskipHash(false),
      _liveType(LivedataType::Overwrite),
      liveParam(live),
      fileParameter(0),
      mathUpdateBegin(0),
      mathUpdateEnd(0),
      vFactor(0.66),
      reflection(true),
      visible(true),
      paused(false),
      reference_impedance(50.0),
      domain(DataType::Frequency),
      deembeddingActive(false),
      deembedded_reference_impedance(50.0),
      lastMath(nullptr)
{
    settings.valid = false;
    MathInfo self = {.math = this, .enabled = true};
    mathOps.push_back(self);
    updateLastMath(mathOps.rbegin());

    lastMathUpdate = QTime::currentTime();
    mathCalcTimer.setSingleShot(true);
    connect(&mathCalcTimer, &QTimer::timeout, this, &Trace::calculateMath);

    fromLivedata(LivedataType::Overwrite, live);

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
    // delete math operations. The first math operation is the trace itself, only delete any additional operations
    while(mathOps.size() > 1) {
        removeMathOperation(mathOps.size()-1);
    }
}

void Trace::clear(bool force) {
    if(paused && !force) {
        return;
    }
    data.clear();
    deembeddingData.clear();
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
        if(data.size() <= (unsigned int) index) {
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

void Trace::addData(const Trace::Data &d, const DeviceDriver::SASettings &s, int index)
{
    settings.SA = s;
    settings.valid = true;
    auto domain = DataType::Frequency;
    if (s.freqStart == s.freqStop) {
        // in zerospan mode
        domain = DataType::TimeZeroSpan;
    }
    addData(d, domain, 50.0, index);
}

void Trace::addDeembeddingData(const Trace::Data &d, double reference_impedance, int index)
{
    bool wasAvailable = deembeddingAvailable();
    if(index >= 0) {
        // index position specified
        if(deembeddingData.size() <= (unsigned int) index) {
            deembeddingData.resize(index + 1);
        }
        deembeddingData[index] = d;
    } else {
        // no index given, determine position by X-coordinate

        // add or replace data in vector while keeping it sorted with increasing frequency
        auto lower = lower_bound(deembeddingData.begin(), deembeddingData.end(), d, [](const Data &lhs, const Data &rhs) -> bool {
            return lhs.x < rhs.x;
        });
        // calculate index now because inserting a sample into data might lead to reallocation -> arithmetic on lower not valid anymore
        index = lower - deembeddingData.begin();
        if(lower == deembeddingData.end()) {
            // highest frequency yet, add to vector
            deembeddingData.push_back(d);
        } else if(lower->x == d.x) {
            *lower = d;
        } else {
            // insert at this position
            deembeddingData.insert(lower, d);
        }
    }
    if(deembedded_reference_impedance != reference_impedance) {
        deembedded_reference_impedance = reference_impedance;
        if(deembeddingActive) {
            emit typeChanged(this);
        }
    }
    if(deembeddingActive) {
        emit outputSamplesChanged(index, index + 1);
    }
    if(!wasAvailable) {
        emit deembeddingChanged(this);
    }
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
    // check if parameter is a reflection measurement (e.i. S11/S22/S33/...)
    reflection = false;
    for (unsigned int i = 0; i * i <= parameter; i++) {
        if (parameter == i * t.ports() + i) {
            reflection = true;
            break;
        }
    }
    clearMathSources();
    source = Source::File;
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
            if(traceNum > (int) parameter) {
                // got all columns for the trace we are interested in
                break;
            }
            lastTraceName = traceName;
        }
        if(traceNum == (int) parameter) {
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
    if(traceNum < (int) parameter) {
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
    clearMathSources();
    source = Source::File;
    emit typeChanged(this);
    emit outputSamplesChanged(0, data.size());
    return lastTraceName;
}

void Trace::fillFromDatapoints(std::map<QString, Trace *> traceSet, const std::vector<DeviceDriver::VNAMeasurement> &data, bool deembedded)
{
    // remove all previous points
    for(auto m : traceSet) {
        if(!deembedded) {
            m.second->clear();
        } else {
            m.second->clearDeembedding();
        }
    }
    // add new points to traces
    for(auto d : data) {
        Trace::Data td;
        td.x = d.frequency;
        for(auto m : d.measurements) {
            td.y = m.second;
            QString measurement = m.first;
            if(traceSet.count(measurement)) {
                if(!deembedded) {
                    traceSet[measurement]->addData(td, DataType::Frequency);
                } else {
                    traceSet[measurement]->addDeembeddingData(td, d.Z0);
                }
            }
        }
    }
}

void Trace::fromLivedata(Trace::LivedataType type, QString param)
{
    clearMathSources();
    source = Source::Live;
    _liveType = type;
    liveParam = param;
    if(param.length() == 3 && param[0] == 'S' && param[1].isDigit() && param[1] == param[2]) {
        reflection = true;
    } else {
        reflection = false;
    }
    emit typeChanged(this);
}

void Trace::fromMath()
{
    source = Source::Math;
    clear();
    mathUpdateEnd = 0;
    mathUpdateBegin = numeric_limits<unsigned int>::max();
    updateMathTracePoints();
    scheduleMathCalculation(0, data.size());
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
    Q_UNUSED(m);
    // trigger replot by pretending that trace visibility also changed
    emit visibilityChanged(this);
}

const QString &Trace::getMathFormula() const
{
    return mathFormula;
}

void Trace::setMathFormula(const QString &newMathFormula)
{
    mathFormula = newMathFormula;
    scheduleMathCalculation(0, data.size());
}

QString Trace::getMathFormulaError() const
{
    if(mathFormula.isEmpty()) {
        return "Math formula must not be empty";
    }
    try {
        ParserX parser(pckCOMMON | pckUNIT | pckCOMPLEX);
        parser.SetExpr(mathFormula.toStdString());
        auto vars = parser.GetExprVar();
        for(auto var : vars) {
            auto varName = QString::fromStdString(var.first);
            // try to find variable name
            bool found = false;
            for(auto ms : mathSourceTraces) {
                if(ms.second == varName) {
                    found = true;
                    break;
                }
            }
            if(varName == "x") {
                found = true;
            }
            if(!found) {
                return "Unknown variable: "+varName;
            }
        }
    } catch (const ParserError &e) {
        // parser error occurred
        return "Parsing failed: " + QString::fromStdString(e.GetMsg());
    }
    // all variables used in the expression are set as math sources
    return "";
}

bool Trace::resolveMathSourceHashes()
{
    bool success = true;
    for(auto unresolved : mathSourceUnresolvedHashes) {
        if(!addMathSource(unresolved.first, unresolved.second)) {
            success = false;
        }
    }
    if(success) {
        mathSourceUnresolvedHashes.clear();
    }
    return success;
}

void Trace::updateMathTracePoints()
{
    if(paused) {
        // do not update when paused
        return;
    }
    if(!mathSourceTraces.size()) {
        return;
    }
    double startX = std::numeric_limits<double>::lowest();
    double stopX = std::numeric_limits<double>::max();
    double stepSize = std::numeric_limits<double>::max();
    auto domain = DataType::Invalid;
    for(auto t : mathSourceTraces) {
        if(domain == DataType::Invalid) {
            domain = t.first->outputType();
        } else {
            if(domain != t.first->outputType()) {
                // not all traces have the same domain, clear output and do not calculate
                data.resize(0);
                mathUpdateBegin = 0;
                mathUpdateEnd = 0;
                dataType = DataType::Invalid;
                emit outputTypeChanged(dataType);
                return;
            }
        }
        if(t.first->minX() > startX) {
            startX = t.first->minX();
        }
        if(t.first->maxX() < stopX) {
            stopX = t.first->maxX();
        }
        double traceStepSize = std::numeric_limits<double>::max();
        if(t.first->numSamples() > 1) {
            traceStepSize = (t.first->maxX() - t.first->minX()) / (t.first->numSamples() - 1);
        }
        if(traceStepSize < stepSize) {
            stepSize = traceStepSize;
        }
    }
    if(domain != this->domain) {
        this->domain = domain;
        emit typeChanged(this);
    }
    unsigned int samples = 0;
    if(stopX > startX) {
        samples = round((stopX - startX) / stepSize + 1);
    }
//    qDebug() << "Updated trace points, now"<<samples<<"points from"<<startX<<"to"<<stopX;
    if(samples != data.size()) {
        auto oldSize = data.size();
        data.resize(samples);
        if(oldSize < samples) {
            for(unsigned int i=oldSize;i<samples;i++) {
                data[i].x = startX + i * stepSize;
                data[i].y = numeric_limits<complex<double>>::quiet_NaN();
            }
        }
    }
    if(samples > 0 && (startX != data.front().x || stopX != data.back().x)) {
        mathUpdateBegin = 0;
        mathUpdateEnd = samples;
    }
}

void Trace::mathSourceTraceDeleted(Trace *t)
{
    if (mathSourceTraces.count(t)) {
        removeMathSource(t);
        updateMathTracePoints();
        scheduleMathCalculation(0, data.size());
    }
}

void Trace::scheduleMathCalculation(unsigned int begin, unsigned int end)
{
//    qDebug() << "Scheduling calculation from"<<begin<<"to"<<end;
    if(source != Source::Math) {
        return;
    }
    if(begin < mathUpdateBegin) {
        mathUpdateBegin = begin;
    }
    if(end > mathUpdateEnd) {
        mathUpdateEnd = end;
    }
    auto now = QTime::currentTime();
    if (lastMathUpdate.msecsTo(now) >= MinMathUpdateInterval) {
        calculateMath();
    } else {
        mathCalcTimer.start(MinMathUpdateInterval);
    }
}

void Trace::calculateMath()
{
    lastMathUpdate = QTime::currentTime();
    if(mathUpdateEnd <= mathUpdateBegin) {
        // nothing to do
        return;
    }
    if(mathUpdateBegin >= data.size() || mathUpdateEnd >= data.size() + 1) {
        qWarning() << "Not calculating math trace, out of limits. Requested from" << mathUpdateBegin << "to" << mathUpdateEnd <<" but data is of size" << data.size();
        return;
    }
    if(mathFormula.isEmpty()) {
        error("Expression is empty");
        return;
    }
    if(!isPaused()) {
        try {
            ParserX parser(pckCOMMON | pckUNIT | pckCOMPLEX);
            parser.SetExpr(mathFormula.toStdString());
            map<Trace*,Value> values;
            Value x;
            parser.DefineVar("x", Variable(&x));
            for(const auto &ts : mathSourceTraces) {
                values[ts.first] = Value();
                parser.DefineVar(ts.second.toStdString(), Variable(&values[ts.first]));
            }
            for(unsigned int i=mathUpdateBegin;i<mathUpdateEnd;i++) {
                x = data[i].x;
                for(auto &val : values) {
                    val.second = val.first->interpolatedSample(data[i].x).y;
                }
                Value res = parser.Eval();
                data[i].y = res.GetComplex();
            }
        } catch (const ParserError &e) {
            error(QString::fromStdString(e.GetMsg()));
            // parser error occurred
            for(unsigned int i=mathUpdateBegin;i<mathUpdateEnd;i++) {
                data[i].y = numeric_limits<complex<double>>::quiet_NaN();
            }
        }
        success();
        emit outputSamplesChanged(mathUpdateBegin, mathUpdateEnd + 1);
    }
    mathUpdateBegin = data.size();
    mathUpdateEnd = 0;
}

void Trace::clearMathSources()
{
    while(mathSourceTraces.size() > 0) {
        removeMathSource(mathSourceTraces.begin()->first);
    }
}

bool Trace::addMathSource(unsigned int hash, QString variableName)
{
    if(!model) {
        return false;
    }
    for(auto t : model->getTraces()) {
        if(t->toHash() == hash) {
            return addMathSource(t, variableName);
        }
    }
    return false;
}

void Trace::setReferenceImpedance(double value)
{
    reference_impedance = value;
}

bool Trace::mathDependsOn(Trace *t, bool onlyDirectDependency)
{
    if(mathSourceTraces.count(t)) {
        return true;
    }
    if(onlyDirectDependency) {
        return false;
    } else {
        // also check math source traces recursively
        for(auto m : mathSourceTraces) {
            if(m.first->mathDependsOn(t)) {
                return true;
            }
        }
        return false;
    }
}

bool Trace::canAddAsMathSource(Trace *t)
{
    if(t == this) {
        // can't add itself
        return false;
    }
    // check if we would create a loop of math traces depending on each other
    if(t->mathDependsOn(this)) {
        return false;
    }
    if(mathSourceTraces.size() == 0) {
        // no traces used as source yet, can add anything
        return true;
    } else {
        // can only add traces of the same domain
        if(mathSourceTraces.begin()->first->outputType() == t->outputType()) {
            return true;
        } else {
            return false;
        }
    }
}

bool Trace::addMathSource(Trace *t, QString variableName)
{
    if(mathSourceTraces.count(t)) {
        // this trace is already used as a math source
        mathSourceTraces[t] = variableName;
        return true;
    }
//    qDebug() << "Adding trace" << t << "as a math source to" << this << "as variable" << variableName;
    if(!canAddAsMathSource(t)) {
        return false;
    }
    mathSourceTraces[t] = variableName;
    connect(t, &Trace::deleted, this, &Trace::mathSourceTraceDeleted, Qt::UniqueConnection);
    connect(t, &Trace::dataChanged, this, [=](unsigned int begin, unsigned int end){
        updateMathTracePoints();
        auto startX = t->sample(begin).x;
        auto stopX = t->sample(end-1).x;

//        qDebug() << "Source update from"<<startX<<"to"<<stopX<<". Index from"<<begin<<"to"<<end;

        auto calcIndex = [&](double x) -> int {
            if(data.size() == 0) {
                return 0;
            }
            auto lower = lower_bound(data.begin(), data.end(), x, [](const Data &lhs, const double x) -> bool {
                return lhs.x < x;
            });
            if(lower == data.end()) {
                // actually beyond the last sample, return the index of the last anyway to avoid access past data
                return data.size() - 1;
            }
            return lower - data.begin();
        };

        scheduleMathCalculation(calcIndex(startX), calcIndex(stopX)+1);
    });
    updateMathTracePoints();
    scheduleMathCalculation(0, data.size());
    return true;
}

void Trace::removeMathSource(Trace *t)
{
//    qDebug() << "Removing trace" << t << "as a math source from" << this;
    mathSourceTraces.erase(t);
    disconnect(t, &Trace::deleted, this, &Trace::mathSourceTraceDeleted);
    disconnect(t, &Trace::dataChanged, this, nullptr);
}

QString Trace::getSourceVariableName(Trace *t)
{
    if(mathSourceTraces.count(t)) {
        return mathSourceTraces[t];
    } else {
        return QString();
    }
}

TraceModel *Trace::getModel() const
{
    return model;
}

void Trace::setModel(TraceModel *model)
{
    this->model = model;
}

double Trace::getReferenceImpedance() const
{
    if(deembeddingActive) {
        return deembedded_reference_impedance;
    } else {
        return reference_impedance;
    }
}

const std::vector<Trace::MathInfo>& Trace::getMathOperations() const
{
    return mathOps;
}

bool Trace::isSAParameter(QString param)
{
    return param.length() == 5 && param.startsWith("PORT") && param[4].isDigit();
}

bool Trace::isVNAParameter(QString param)
{
    if(param.length() == 3 && param[0] == 'S' && param[1].isDigit() && param[2].isDigit()) {
        // normal S parameter
        return true;
    }
    if(param.startsWith("RawPort")) {
        // raw receiver value
        return true;
    }
    return false;
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
    if(!JSONskipHash) {
        j["hash"] = toHash(true);
    }
    j["name"] = _name.toStdString();
    j["color"] = _color.name().toStdString();
    j["visible"] = visible;
    switch(source) {
    case Source::Live:
        j["type"] = "Live";
        j["parameter"] = liveParam.toStdString();
        j["livetype"] = _liveType;
        j["paused"] = paused;
        break;
    case Source::File:
        j["type"] = "File";
        j["filename"] = filename.toStdString();
        j["parameter"] = fileParameter;
        break;
    case Source::Math: {
        j["type"] = "Math";
        j["expression"] = mathFormula.toStdString();
        nlohmann::json jsources;
        for(auto ms : mathSourceTraces) {
            nlohmann::json jsource;
            jsource["trace"] = ms.first->toHash();
            jsource["variable"] = ms.second.toStdString();
            jsources.push_back(jsource);
        }
        j["sources"] = jsources;
    }
        break;
    case Source::Calibration:
        j["type"] = "Calibration";
        break;
    case Source::Last:
        break;
    }
    j["velocityFactor"] = vFactor;
    j["referenceImpedance"] = reference_impedance;
    j["reflection"] = reflection;

    auto &pref = Preferences::getInstance();
    if(pref.Debug.saveTraceData) {
        nlohmann::json jdata;
        for(const auto &d : data) {
            nlohmann::json jpoint;
            jpoint["x"] = d.x;
            jpoint["real"] = d.y.real();
            jpoint["imag"] = d.y.imag();
            jdata.push_back(jpoint);
        }
        j["data"] = jdata;
    }

    j["deembeddingActive"] = deembeddingActive;
    nlohmann::json jdedata;
    for(const auto &d : deembeddingData) {
        nlohmann::json jpoint;
        jpoint["x"] = d.x;
        jpoint["real"] = d.y.real();
        jpoint["imag"] = d.y.imag();
        jdedata.push_back(jpoint);
    }
    j["deembeddingData"] = jdedata;

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
    source = Source::Live;
    if(j.contains("hash")) {
        hash = j["hash"];
        hashSet = true;
    } else {
        hashSet = false;
    }
    _name = QString::fromStdString(j.value("name", "Missing name"));
    _color = QColor(QString::fromStdString(j.value("color", "yellow")));
    visible = j.value("visible", true);
    auto type = QString::fromStdString(j.value("type", "Live"));
    if(j.contains("data")) {
        // Trace data is contained in the json, load now
        clear();
        for(auto jpoint : j["data"]) {
            Data d;
            d.x = jpoint.value("x", 0.0);
            d.y = complex<double>(jpoint.value("real", 0.0), jpoint.value("imag", 0.0));
            data.push_back(d);
        }
    }
    if(j.contains("deembeddingData")) {
        // Deembedded data is contained in the json, load now
        clearDeembedding();
        for(auto jpoint : j["deembeddingData"]) {
            Data d;
            d.x = jpoint.value("x", 0.0);
            d.y = complex<double>(jpoint.value("real", 0.0), jpoint.value("imag", 0.0));
            deembeddingData.push_back(d);
        }
    }
    deembeddingActive = j.value("deembeddingActive", false);

    if(type == "Live") {
        if(j.contains("parameter")) {
            if(j["parameter"].type() == nlohmann::json::value_t::string) {
                liveParam = QString::fromStdString(j.value("parameter", "S11"));
            } else {
                // old parameter setting is stored as a number
                auto index = j.value("parameter", 0);
                const QString parameterList[] = {"S11", "S12", "S21", "S22", "PORT1", "PORT2"};
                if(index < 6) {
                    liveParam = parameterList[index];
                } else {
                    liveParam = "S11";
                }
            }
        }

        _liveType = j.value("livetype", LivedataType::Overwrite);
        paused = j.value("paused", false);
    } else if(type == "Touchstone" || type == "File") {
        source = Source::File;
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
            qWarning() << "Failed to create from file:" << QString::fromStdString(e.what());
        }
    } else if(type == "Math") {
        source = Source::Math;
        mathFormula = QString::fromStdString(j.value("expression", ""));
        if(j.contains("sources")) {
            for(auto js : j["sources"]) {
                auto hash = js.value("trace", 0);
                QString varName = QString::fromStdString(js.value("variable", ""));
                if(!addMathSource(hash, varName)) {
                    qWarning() << "Unable to find requested math source trace ( hash:"<<hash<<"), probably not loaded yet";
                    mathSourceUnresolvedHashes[hash] = varName;
                }
            }
        }
        fromMath();
    } else if(type == "Calibration") {
        source = Source::Calibration;
        // data has already been loaded if present in the file
    }
    vFactor = j.value("velocityFactor", 0.66);
    reference_impedance = j.value("referenceImpedance", 50.0);
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
    // indicate successful loading of trace data
    success();
}

unsigned int Trace::toHash(bool forceUpdate)
{
    if(!hashSet || forceUpdate) {
        // taking the easy way: create the json string and hash it (already contains all necessary information)
        // This is slower than it could be, but this function is only used when loading setups, so this isn't a big problem
        JSONskipHash = true;
        std::string json_string = toJSON().dump();
        JSONskipHash = false;
        hash = std::hash<std::string>{}(json_string);
        hashSet = true;
    }
    return hash;
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

std::vector<DeviceDriver::VNAMeasurement> Trace::assembleDatapoints(std::map<QString, Trace *> traceSet)
{
    vector<DeviceDriver::VNAMeasurement> ret;

    // Sanity check traces
    unsigned int samples = traceSet.begin()->second->size();
    auto impedance = traceSet.begin()->second->getReferenceImpedance();
    vector<double> freqs;
    for(auto m : traceSet) {
        const Trace *t = m.second;
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
        DeviceDriver::VNAMeasurement d;
        for(auto m : traceSet) {
            QString measurement = m.first;
            const Trace *t = m.second;
            d.measurements[measurement] = t->sample(i).y;
        }
        d.pointNum = i;
        d.frequency = freqs[i];
        d.Z0 = impedance;
        ret.push_back(d);
    }
    return ret;
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
        emit lastMathChanged();
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

void Trace::setCalibration()
{
    source = Source::Calibration;
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

bool Trace::canBePaused()
{
    switch(source) {
    case Source::Live:
        return true;
    case Source::File:
        return false;
    case Source::Calibration:
        return false;
    case Source::Math:
        // depends on the math, if it depends on any live data, it may be paused
        for(auto ms : mathSourceTraces) {
            if(ms.first->canBePaused()) {
                return true;
            }
        }
        return false;
    default:
        return false;
    }
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

bool Trace::isDeembeddingActive()
{
    return deembeddingActive;
}

bool Trace::deembeddingAvailable()
{
    return deembeddingData.size() > 0;
}

void Trace::setDeembeddingActive(bool active)
{
    if(active == deembeddingActive) {
        // no change
        return;
    }
    deembeddingActive = active;
    if(deembeddingAvailable()) {
        if(active) {
            emit outputSamplesChanged(0, deembeddingData.size());
        } else {
            emit outputSamplesChanged(0, data.size());
        }
    }
    emit deembeddingChanged(this);
}

void Trace::clearDeembedding()
{
    deembeddingData.clear();
    setDeembeddingActive(false);
}

double Trace::minX()
{
    if(lastMath == this) {
        return TraceMath::minX();
    } else {
        return lastMath->minX();
    }
}

double Trace::maxX()
{
    if(lastMath == this) {
        return TraceMath::maxX();
    } else {
        return lastMath->maxX();
    }
}

double Trace::findExtremum(bool max, double xmin, double xmax)
{
    double compare = max ? numeric_limits<double>::min() : numeric_limits<double>::max();
    double freq = 0.0;
    for(auto sample : lastMath->getData()) {
        if(sample.x < xmin || sample.x > xmax) {
            continue;
        }
        double amplitude = abs(sample.y);
        if((max && (amplitude > compare)) || (!max && (amplitude < compare))) {
            // higher/lower extremum found
            compare = amplitude;
            freq = sample.x;
        }
    }
    return freq;
}

std::vector<double> Trace::findPeakFrequencies(unsigned int maxPeaks, double minLevel, double minValley, double xmin, double xmax, bool negativePeaks)
{
    if(outputType() != DataType::Frequency) {
        // not in frequency domain
        return vector<double>();
    }
    if(negativePeaks) {
        minLevel = -minLevel;
    }
    using peakInfo = struct peakinfo {
        double frequency;
        double level_dbm;
    };
    vector<peakInfo> peaks;
    double frequency = 0.0;
    double max_dbm = -200.0;
    double min_dbm = 200.0;
    for(auto d : lastMath->getData()) {
        if(d.x < xmin || d.x > xmax) {
            continue;
        }
        double dbm = Util::SparamTodB(d.y);
        if(negativePeaks) {
            dbm = -dbm;
        }
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

Trace::Data Trace::getSample(unsigned int index)
{
    if(deembeddingActive && deembeddingAvailable()) {
        if(index < deembeddingData.size()) {
            return deembeddingData[index];
        } else {
            TraceMath::Data d;
            d.x = 0;
            d.y = 0;
            return d;
        }
    } else {
        return TraceMath::getSample(index);
    }
}

Trace::Data Trace::getInterpolatedSample(double x)
{
    if(deembeddingActive && deembeddingAvailable()) {
        Data ret;
        if(deembeddingData.size() == 0 || x < deembeddingData.front().x || x > deembeddingData.back().x) {
            ret.y = std::numeric_limits<std::complex<double>>::quiet_NaN();
            ret.x = std::numeric_limits<double>::quiet_NaN();
        } else {
            auto it = lower_bound(deembeddingData.begin(), deembeddingData.end(), x, [](const Data &lhs, const double x) -> bool {
                return lhs.x < x;
            });
            if(it->x == x) {
                ret = *it;
            } else {
                // no exact match, needs to interpolate
                auto high = *it;
                it--;
                auto low = *it;
                double alpha = (x - low.x) / (high.x - low.x);
                ret.y = low.y * (1 - alpha) + high.y * alpha;
                ret.x = x;
            }
        }
        return ret;
    } else {
        return TraceMath::getInterpolatedSample(x);
    }
}

unsigned int Trace::numSamples()
{
    if(deembeddingActive && deembeddingAvailable()) {
        return deembeddingData.size();
    } else {
        return TraceMath::numSamples();
    }
}

std::vector<Trace::Data> Trace::getData()
{
    if(deembeddingActive && deembeddingAvailable()) {
        return deembeddingData;
    } else {
        return TraceMath::getData();
    }
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
    if(source != Trace::Source::Live || !settings.valid || !liveParam.startsWith("PORT") || outputType() != DataType::Frequency) {
        // data not suitable for noise calculation
        return std::numeric_limits<double>::quiet_NaN();
    }
    // convert to dbm
    auto dbm = Util::SparamTodB(lastMath->getInterpolatedSample(frequency).y);
    // convert to 1Hz bandwidth
    dbm -= 10*log10(settings.SA.RBW);
    return dbm;
}

double Trace::getGroupDelay(double frequency)
{
    if(!isVNAParameter(liveParam) || lastMath->getDataType() != DataType::Frequency) {
        // data not suitable for group delay calculation
        return std::numeric_limits<double>::quiet_NaN();
    }

    // get index that matches frequency best
    unsigned int sample = index(frequency);

    auto &p = Preferences::getInstance();
    const unsigned int requiredSamples = p.Acquisition.groupDelaySamples;
    if(size() < requiredSamples) {
        // unable to calculate
        return std::numeric_limits<double>::quiet_NaN();

    }
    // needs at least some samples before/after current sample for calculating the derivative.
    // For samples too far at either end of the trace, return group delay of "inner" trace sample instead
    if(sample < requiredSamples / 2) {
        sample = requiredSamples / 2;
    } else if(sample >= size() - requiredSamples / 2) {
        sample = size() - requiredSamples / 2 - 1;
    }

    // got enough samples at either end to calculate derivative.
    // acquire phases of the required samples
    std::vector<double> phases;
    phases.reserve(requiredSamples);
    for(unsigned int index = sample - requiredSamples / 2;index <= sample + requiredSamples / 2;index++) {
        phases.push_back(arg(this->sample(index).y));
    }
    // make sure there are no phase jumps
    Util::unwrapPhase(phases);
    // calculate linearRegression to get derivative
    double B_0, B_1;
    Util::linearRegression(phases, B_0, B_1);
    // B_1 now contains the derived phase vs. the sample. Scale by frequency to get group delay
    double freq_step = this->sample(sample).x - this->sample(sample - 1).x;
    return -B_1 / (2.0*M_PI * freq_step);
}

int Trace::index(double x)
{
    int ret;
    lastMath->dataMutex.lock();
    auto lower = lower_bound(lastMath->data.begin(), lastMath->data.end(), x, [](const Data &lhs, const double x) -> bool {
        return lhs.x < x;
    });
    if(lower == lastMath->data.end()) {
        // actually beyond the last sample, return the index of the last anyway to avoid access past data
        ret = lastMath->data.size() - 1;
    }
    ret = lower - lastMath->data.begin();
    lastMath->dataMutex.unlock();
    return ret;
}
