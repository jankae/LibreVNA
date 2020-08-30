#include "calibration.h"
#include <algorithm>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>

using namespace std;

Calibration::Calibration()
{
    // Creator vectors for measurements
    measurements[Measurement::Port1Open].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port1Short].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port1Load].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port2Open].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port2Short].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port2Load].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Isolation].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Through].datapoints = vector<Protocol::Datapoint>();

    type = Type::None;
}

void Calibration::clearMeasurements()
{
    for(auto m : measurements) {
        m.second.datapoints.clear();
    }
}

void Calibration::clearMeasurement(Calibration::Measurement type)
{
    measurements[type].datapoints.clear();
    measurements[type].timestamp = QDateTime();
}

void Calibration::addMeasurement(Calibration::Measurement type, Protocol::Datapoint &d)
{
    measurements[type].datapoints.push_back(d);
    measurements[type].timestamp = QDateTime::currentDateTime();
}

bool Calibration::calculationPossible(Calibration::Type type)
{
    std::vector<Measurement> requiredMeasurements;
    switch(type) {
    case Type::Port1SOL:
        requiredMeasurements.push_back(Measurement::Port1Open);
        requiredMeasurements.push_back(Measurement::Port1Short);
        requiredMeasurements.push_back(Measurement::Port1Load);
        break;
    case Type::Port2SOL:
        requiredMeasurements.push_back(Measurement::Port2Open);
        requiredMeasurements.push_back(Measurement::Port2Short);
        requiredMeasurements.push_back(Measurement::Port2Load);
        break;
    case Type::FullSOLT:
        requiredMeasurements.push_back(Measurement::Port1Open);
        requiredMeasurements.push_back(Measurement::Port1Short);
        requiredMeasurements.push_back(Measurement::Port1Load);
        requiredMeasurements.push_back(Measurement::Port2Open);
        requiredMeasurements.push_back(Measurement::Port2Short);
        requiredMeasurements.push_back(Measurement::Port2Load);
        requiredMeasurements.push_back(Measurement::Through);
        break;
    case Type::None:
        return false;
    }
    return SanityCheckSamples(requiredMeasurements);
}

bool Calibration::constructErrorTerms(Calibration::Type type)
{
    if(!calculationPossible(type)) {
        return false;
    }
    if(minFreq < kit.minFreq() || maxFreq > kit.maxFreq()) {
        // Calkit does not support complete calibration range
        QMessageBox::critical(nullptr, "Unable to perform calibration", "The calibration kit does not support the complete span. Please choose a different calibration kit or a narrower span.");
        return false;
    }
    switch(type) {
    case Type::Port1SOL:
        constructPort1SOL();
        break;
    case Type::Port2SOL:
        constructPort2SOL();
        break;
    case Type::FullSOLT:
        construct12TermPoints();
        break;
    case Type::None:
        break;
    }
    this->type = type;
    return true;
}

void Calibration::resetErrorTerms()
{
    type = Type::None;
    points.clear();
}

void Calibration::construct12TermPoints()
{
    std::vector<Measurement> requiredMeasurements;
    requiredMeasurements.push_back(Measurement::Port1Open);
    requiredMeasurements.push_back(Measurement::Port1Short);
    requiredMeasurements.push_back(Measurement::Port1Load);
    requiredMeasurements.push_back(Measurement::Port2Open);
    requiredMeasurements.push_back(Measurement::Port2Short);
    requiredMeasurements.push_back(Measurement::Port2Load);
    requiredMeasurements.push_back(Measurement::Through);
    if(!SanityCheckSamples(requiredMeasurements)) {
        throw runtime_error("Missing/wrong calibration measurement");
    }
    requiredMeasurements.push_back(Measurement::Isolation);
    bool isolation_measured = SanityCheckSamples(requiredMeasurements);

    // If we get here the calibration measurements are all okay
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port1Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port1Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S11_open = complex<double>(measurements[Measurement::Port1Open].datapoints[i].real_S11, measurements[Measurement::Port1Open].datapoints[i].imag_S11);
        auto S11_short = complex<double>(measurements[Measurement::Port1Short].datapoints[i].real_S11, measurements[Measurement::Port1Short].datapoints[i].imag_S11);
        auto S11_load = complex<double>(measurements[Measurement::Port1Load].datapoints[i].real_S11, measurements[Measurement::Port1Load].datapoints[i].imag_S11);
        auto S22_open = complex<double>(measurements[Measurement::Port2Open].datapoints[i].real_S22, measurements[Measurement::Port2Open].datapoints[i].imag_S22);
        auto S22_short = complex<double>(measurements[Measurement::Port2Short].datapoints[i].real_S22, measurements[Measurement::Port2Short].datapoints[i].imag_S22);
        auto S22_load = complex<double>(measurements[Measurement::Port2Load].datapoints[i].real_S22, measurements[Measurement::Port2Load].datapoints[i].imag_S22);
        auto S21_isolation = complex<double>(0,0);
        auto S12_isolation = complex<double>(0,0);
        if(isolation_measured) {
            S21_isolation = complex<double>(measurements[Measurement::Isolation].datapoints[i].real_S21, measurements[Measurement::Isolation].datapoints[i].imag_S21);
            S12_isolation = complex<double>(measurements[Measurement::Isolation].datapoints[i].real_S12, measurements[Measurement::Isolation].datapoints[i].imag_S12);
        }
        auto S11_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S11, measurements[Measurement::Through].datapoints[i].imag_S11);
        auto S21_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S21, measurements[Measurement::Through].datapoints[i].imag_S21);
        auto S22_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S22, measurements[Measurement::Through].datapoints[i].imag_S22);
        auto S12_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S12, measurements[Measurement::Through].datapoints[i].imag_S12);

        auto actual = kit.toReflection(p.frequency);
        // Forward calibration
        computeSOL(S11_short, S11_open, S11_load, p.fe00, p.fe11, p.fe10e01, actual.Open, actual.Short, actual.Load);
        p.fe30 = S21_isolation;
        // See page 17 of http://www2.electron.frba.utn.edu.ar/~jcecconi/Bibliografia/04%20-%20Param_S_y_VNA/Network_Analyzer_Error_Models_and_Calibration_Methods.pdf
        // Formulas for S11M and S21M solved for e22 and e10e32
        auto deltaS = actual.ThroughS11*actual.ThroughS22 - actual.ThroughS21 * actual.ThroughS12;
        p.fe22 = ((S11_through - p.fe00)*(1.0 - p.fe11 * actual.ThroughS11)-actual.ThroughS11*p.fe10e01)
                / ((S11_through - p.fe00)*(actual.ThroughS22-p.fe11*deltaS)-deltaS*p.fe10e01);
        p.fe10e32 = (S21_through - p.fe30)*(1.0 - p.fe11*actual.ThroughS11 - p.fe22*actual.ThroughS22 + p.fe11*p.fe22*deltaS) / actual.ThroughS21;
        // Reverse calibration
        computeSOL(S22_short, S22_open, S22_load, p.re33, p.re22, p.re23e32, actual.Open, actual.Short, actual.Load);
        p.re03 = S12_isolation;
        p.re11 = ((S22_through - p.re33)*(1.0 - p.re22 * actual.ThroughS22)-actual.ThroughS22*p.re23e32)
                / ((S22_through - p.re33)*(actual.ThroughS11-p.re22*deltaS)-deltaS*p.re23e32);
        p.re23e01 = (S12_through - p.re03)*(1.0 - p.re11*actual.ThroughS11 - p.re22*actual.ThroughS22 + p.re11*p.re22*deltaS) / actual.ThroughS12;
        points.push_back(p);
    }
}

void Calibration::constructPort1SOL()
{
    std::vector<Measurement> requiredMeasurements;
    requiredMeasurements.push_back(Measurement::Port1Open);
    requiredMeasurements.push_back(Measurement::Port1Short);
    requiredMeasurements.push_back(Measurement::Port1Load);
    if(!SanityCheckSamples(requiredMeasurements)) {
        throw runtime_error("Missing/wrong calibration measurement");
    }

    // If we get here the calibration measurements are all okay
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port1Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port1Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S11_open = complex<double>(measurements[Measurement::Port1Open].datapoints[i].real_S11, measurements[Measurement::Port1Open].datapoints[i].imag_S11);
        auto S11_short = complex<double>(measurements[Measurement::Port1Short].datapoints[i].real_S11, measurements[Measurement::Port1Short].datapoints[i].imag_S11);
        auto S11_load = complex<double>(measurements[Measurement::Port1Load].datapoints[i].real_S11, measurements[Measurement::Port1Load].datapoints[i].imag_S11);
        // OSL port1
        auto actual = kit.toReflection(p.frequency);
        // See page 19 of http://www2.electron.frba.utn.edu.ar/~jcecconi/Bibliografia/04%20-%20Param_S_y_VNA/Network_Analyzer_Error_Models_and_Calibration_Methods.pdf
        computeSOL(S11_short, S11_open, S11_load, p.fe00, p.fe11, p.fe10e01, actual.Open, actual.Short, actual.Load);
        // All other calibration coefficients to ideal values
        p.fe30 = 0.0;
        p.fe22 = 0.0;
        p.fe10e32 = 1.0;
        p.re33 = 0.0;
        p.re22 = 0.0;
        p.re23e32 = 1.0;
        p.re03 = 0.0;
        p.re11 = 0.0;
        p.re23e01 = 1.0;
        points.push_back(p);
    }
}

void Calibration::constructPort2SOL()
{
    std::vector<Measurement> requiredMeasurements;
    requiredMeasurements.push_back(Measurement::Port2Open);
    requiredMeasurements.push_back(Measurement::Port2Short);
    requiredMeasurements.push_back(Measurement::Port2Load);
    if(!SanityCheckSamples(requiredMeasurements)) {
        throw runtime_error("Missing/wrong calibration measurement");
    }

    // If we get here the calibration measurements are all okay
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port2Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port2Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S22_open = complex<double>(measurements[Measurement::Port2Open].datapoints[i].real_S22, measurements[Measurement::Port2Open].datapoints[i].imag_S22);
        auto S22_short = complex<double>(measurements[Measurement::Port2Short].datapoints[i].real_S22, measurements[Measurement::Port2Short].datapoints[i].imag_S22);
        auto S22_load = complex<double>(measurements[Measurement::Port2Load].datapoints[i].real_S22, measurements[Measurement::Port2Load].datapoints[i].imag_S22);
        // OSL port2
        auto actual = kit.toReflection(p.frequency);
        // See page 19 of http://www2.electron.frba.utn.edu.ar/~jcecconi/Bibliografia/04%20-%20Param_S_y_VNA/Network_Analyzer_Error_Models_and_Calibration_Methods.pdf
        computeSOL(S22_short, S22_open, S22_load, p.re33, p.re22, p.re23e32, actual.Open, actual.Short, actual.Load);
        // All other calibration coefficients to ideal values
        p.fe30 = 0.0;
        p.fe22 = 0.0;
        p.fe10e32 = 1.0;
        p.fe00 = 0.0;
        p.fe11 = 0.0;
        p.fe10e01 = 1.0;
        p.re03 = 0.0;
        p.re11 = 0.0;
        p.re23e01 = 1.0;
        points.push_back(p);
    }
}

void Calibration::correctMeasurement(Protocol::Datapoint &d)
{
    if(type == Type::None) {
        // No calibration data, do nothing
        return;
    }
    // Convert measurements to complex variables
    auto S11m = complex<double>(d.real_S11, d.imag_S11);
    auto S21m = complex<double>(d.real_S21, d.imag_S21);
    auto S22m = complex<double>(d.real_S22, d.imag_S22);
    auto S12m = complex<double>(d.real_S12, d.imag_S12);

    // find correct entry
    auto p = getCalibrationPoint(d);

    // equations from page 20 of http://www2.electron.frba.utn.edu.ar/~jcecconi/Bibliografia/04%20-%20Param_S_y_VNA/Network_Analyzer_Error_Models_and_Calibration_Methods.pdf
    auto denom = (1.0 + (S11m - p.fe00) / p.fe10e01 * p.fe11) * (1.0 + (S22m - p.re33) / p.re23e32 * p.re22)
            - (S21m - p.fe30) / p.fe10e32 * (S12m - p.re03) / p.re23e01 * p.fe22 * p.re11;
    auto S11 = ((S11m - p.fe00) / p.fe10e01 * (1.0 + (S22m - p.re33) / p.re23e32 * p.re22)
            - p.fe22 * (S21m - p.fe30) / p.fe10e32 * (S12m - p.re03) / p.re23e01) / denom;
    auto S21 = ((S21m - p.fe30) / p.fe10e32 * (1.0 + (S22m - p.re33) / p.re23e32 * (p.re22 - p.fe22))) / denom;
    auto S22 = ((S22m - p.re33) / p.re23e32 * (1.0 + (S11m - p.fe00) / p.fe10e01 * p.fe11)
            - p.re11 * (S21m - p.fe30) / p.fe10e32 * (S12m - p.re03) / p.re23e01) / denom;
    auto S12 = ((S12m - p.re03) / p.re23e01 * (1.0 + (S11m - p.fe00) / p.fe10e01 * (p.fe11 - p.re11))) / denom;

    d.real_S11 = S11.real();
    d.imag_S11 = S11.imag();
    d.real_S12 = S12.real();
    d.imag_S12 = S12.imag();
    d.real_S21 = S21.real();
    d.imag_S21 = S21.imag();
    d.real_S22 = S22.real();
    d.imag_S22 = S22.imag();
}

Calibration::InterpolationType Calibration::getInterpolation(Protocol::SweepSettings settings)
{
    if(!points.size()) {
        return InterpolationType::NoCalibration;
    }
    if(settings.f_start < points.front().frequency || settings.f_stop > points.back().frequency) {
        return InterpolationType::Extrapolate;
    }
    // Either exact or interpolation, check individual frequencies
    uint32_t f_step = (settings.f_stop - settings.f_start) / (settings.points - 1);
    for(uint64_t f = settings.f_start; f <= settings.f_stop; f += f_step) {
        if(find_if(points.begin(), points.end(), [&f](const Point& p){
            return abs(f - p.frequency) < 100;
        }) == points.end()) {
            return InterpolationType::Interpolate;
        }
    }
    // if we get here all frequency points were matched
    if(points.front().frequency == settings.f_start && points.back().frequency == settings.f_stop) {
        return InterpolationType::Unchanged;
    } else {
        return InterpolationType::Exact;
    }
}

QString Calibration::MeasurementToString(Calibration::Measurement m)
{
    switch(m) {
    case Measurement::Port1Open:
        return "Port 1 Open";
    case Measurement::Port1Short:
        return "Port 1 Short";
    case Measurement::Port1Load:
        return "Port 1 Load";
    case Measurement::Port2Open:
        return "Port 2 Open";
    case Measurement::Port2Short:
        return "Port 2 Short";
    case Measurement::Port2Load:
        return "Port 2 Load";
    case Measurement::Through:
        return "Through";
    case Measurement::Isolation:
        return "Isolation";
    default:
        return "Unknown";
    }
}

QString Calibration::TypeToString(Calibration::Type t)
{
    switch(t) {
    case Type::Port1SOL: return "Port 1"; break;
    case Type::Port2SOL: return "Port 2"; break;
    case Type::FullSOLT: return "SOLT"; break;
    default: return "None"; break;
    }
}

const std::vector<Calibration::Type> Calibration::Types()
{
    const std::vector<Calibration::Type> ret = {Type::Port1SOL, Type::Port2SOL, Type::FullSOLT};
    return ret;
}

const std::vector<Calibration::Measurement> Calibration::Measurements(Calibration::Type type)
{
    switch(type) {
    case Type::FullSOLT:
    case Type::None:
        return {Measurement::Port1Short, Measurement::Port1Open, Measurement::Port1Load, Measurement::Port2Short, Measurement::Port2Open, Measurement::Port2Load, Measurement::Through, Measurement::Isolation};
        break;
    case Type::Port1SOL:
        return {Measurement::Port1Short, Measurement::Port1Open, Measurement::Port1Load};
        break;
    case Type::Port2SOL:
        return {Measurement::Port2Short, Measurement::Port2Open, Measurement::Port2Load};
        break;
    default:
        return {};
        break;
    }
}

Calibration::MeasurementInfo Calibration::getMeasurementInfo(Calibration::Measurement m)
{
    MeasurementInfo info;
    switch(m) {
    case Measurement::Port1Short:
        info.name = "Port 1 short";
        info.prerequisites = "Short standard connected to port 1, port 2 open";
        break;
    case Measurement::Port1Open:
        info.name = "Port 1 open";
        info.prerequisites = "Open standard connected to port 1, port 2 open";
        break;
    case Measurement::Port1Load:
        info.name = "Port 1 load";
        info.prerequisites = "Load standard connected to port 1, port 2 open";
        break;
    case Measurement::Port2Short:
        info.name = "Port 2 short";
        info.prerequisites = "Port 1 open, short standard connected to port 2";
        break;
    case Measurement::Port2Open:
        info.name = "Port 2 open";
        info.prerequisites = "Port 1 open, open standard connected to port 2";
        break;
    case Measurement::Port2Load:
        info.name = "Port 2 load";
        info.prerequisites = "Port 1 open, load standard connected to port 2";
        break;
    case Measurement::Through:
        info.name = "Through";
        info.prerequisites = "Port 1 connected to port 2 via through standard";
        break;
    case Measurement::Isolation:
        info.name = "Isolation";
        info.prerequisites = "Both ports terminated into 50 ohm";
    }
    info.points = measurements[m].datapoints.size();
    if(info.points > 0) {
        info.fmin = measurements[m].datapoints.front().frequency;
        info.fmax = measurements[m].datapoints.back().frequency;
        info.points = measurements[m].datapoints.size();
    }
    info.timestamp = measurements[m].timestamp;
    return info;
}

std::vector<Trace *> Calibration::getErrorTermTraces()
{
    std::vector<Trace*> traces;
    const QString traceNames[12] = {"e00", "F_e11", "e10e01", "e10e32", "F_e22", "e30", "e33", "R_e11", "e23e32", "e23e01", "R_e22", "e03"};
    constexpr bool reflection[12] = {true, true, false, false, true, false, true, true, false, false, true, false};
    for(int i=0;i<12;i++) {
        auto t = new Trace(traceNames[i], Qt::red);
        t->setCalibration(true);
        t->setReflection(reflection[i]);
        traces.push_back(t);
    }
    for(auto p : points) {
        Trace::Data d;
        d.frequency = p.frequency;
        for(int i=0;i<12;i++) {
            switch(i) {
            case 0: d.S = p.fe00; break;
            case 1: d.S = p.fe11; break;
            case 2: d.S = p.fe10e01; break;
            case 3: d.S = p.fe10e32; break;
            case 4: d.S = p.fe22; break;
            case 5: d.S = p.fe30; break;
            case 6: d.S = p.re33; break;
            case 7: d.S = p.re11; break;
            case 8: d.S = p.re23e32; break;
            case 9: d.S = p.re23e01; break;
            case 10: d.S = p.re22; break;
            case 11: d.S = p.re03; break;
            }
            traces[i]->addData(d);
        }
    }
    return traces;
}

bool Calibration::openFromFile(QString filename)
{
    if(filename.isEmpty()) {
        filename = QFileDialog::getOpenFileName(nullptr, "Load calibration data", "", "Calibration files (*.cal)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return false;
        }
    }

    // attempt to load associated calibration kit first (needs to be available when performing calibration)
    auto calkit_file = filename;
    auto dotPos = calkit_file.lastIndexOf('.');
    if(dotPos >= 0) {
        calkit_file.truncate(dotPos);
    }
    calkit_file.append(".calkit");
    try {
        kit = Calkit::fromFile(calkit_file.toStdString());
    } catch (runtime_error e) {
        QMessageBox::warning(nullptr, "Missing calibration kit", "The calibration kit file associated with the selected calibration could not be parsed. The calibration might not be accurate. (" + QString(e.what()) + ")");
    }

    ifstream file;
    file.open(filename.toStdString());
    try {
        file >> *this;
    } catch(runtime_error e) {
        QMessageBox::warning(nullptr, "File parsing error", e.what());
        return false;
    }

    return true;
}

bool Calibration::saveToFile(QString filename)
{
    if(filename.isEmpty()) {
        filename = QFileDialog::getSaveFileName(nullptr, "Save calibration data", "", "Calibration files (*.cal)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return false;
        }
    }
    // strip any potential file name extension and set default
    auto dotPos = filename.lastIndexOf('.');
    if(dotPos >= 0) {
        filename.truncate(dotPos);
    }
    auto calibration_file = filename;
    calibration_file.append(".cal");
    ofstream file;
    file.open(calibration_file.toStdString());
    file << *this;

    auto calkit_file = filename;
    calkit_file.append(".calkit");
    kit.toFile(calkit_file.toStdString());

    return true;
}

ostream& operator<<(ostream &os, const Calibration &c)
{
    for(auto m : c.measurements) {
        if(m.second.datapoints.size() > 0) {
            os << c.MeasurementToString(m.first).toStdString() << endl;
            os << m.second.timestamp.toSecsSinceEpoch() << endl;
            os << m.second.datapoints.size() << endl;
            for(auto p : m.second.datapoints) {
                os << p.pointNum << " " << p.frequency << " ";
                os << p.imag_S11 << " " << p.real_S11 << " " << p.imag_S21 << " " << p.real_S21 << " " << p.imag_S12 << " " << p.real_S12 << " " << p.imag_S22 << " " << p.real_S22;
                os << endl;
            }
        }
    }
    os << Calibration::TypeToString(c.getType()).toStdString() << endl;
    return os;
}

istream& operator >>(istream &in, Calibration &c)
{
    std::string line;
    while(getline(in, line)) {
        for(auto m : Calibration::Measurements()) {
            if(Calibration::MeasurementToString(m) == QString::fromStdString(line)) {
                // this is the correct measurement
                c.clearMeasurement(m);
                uint timestamp;
                in >> timestamp;
                c.measurements[m].timestamp = QDateTime::fromSecsSinceEpoch(timestamp);
                unsigned int points;
                in >> points;
                for(unsigned int i=0;i<points;i++) {
                    Protocol::Datapoint p;
                    in >> p.pointNum >> p.frequency;
                    in >> p.imag_S11 >> p.real_S11 >> p.imag_S21 >> p.real_S21 >> p.imag_S12 >> p.real_S12 >> p.imag_S22 >> p.real_S22;
                    c.measurements[m].datapoints.push_back(p);
                    if(in.eof() || in.bad() || in.fail()) {
                        c.clearMeasurement(m);
                        throw runtime_error("Failed to parse measurement \"" + line + "\", aborting calibration data import.");
                    }
                }
                break;
            }
        }
        for(auto t : Calibration::Types()) {
            if(Calibration::TypeToString(t) == QString::fromStdString(line)) {
                // try to apply this calibration type
                if(c.calculationPossible(t)) {
                    c.constructErrorTerms(t);
                } else {
                    throw runtime_error("Incomplete calibration data, the requested \"" + line + "\"-Calibration could not be performed.");
                }
            }
            break;
        }
    }
    return in;
}

bool Calibration::SanityCheckSamples(std::vector<Calibration::Measurement> &requiredMeasurements)
{
    // sanity check measurements, all need to be of the same size with the same frequencies (except for isolation which may be empty)
    vector<uint64_t> freqs;
    for(auto type : requiredMeasurements) {
        auto m = measurements[type];
        if(m.datapoints.size() == 0) {
            // empty required measurement
            return false;
        }
        if(freqs.size() == 0) {
            // this is the first measurement, create frequency vector
            for(auto p : m.datapoints) {
                freqs.push_back(p.frequency);
            }
        } else {
            // compare with already assembled frequency vector
            if(m.datapoints.size() != freqs.size()) {
                return false;
            }
            for(unsigned int i=0;i<freqs.size();i++) {
                if(m.datapoints[i].frequency != freqs[i]) {
                    return false;
                }
            }
        }
    }
    minFreq = freqs.front();
    maxFreq = freqs.back();
    return true;
}

Calibration::Point Calibration::getCalibrationPoint(Protocol::Datapoint &d)
{
    if(!points.size()) {
        throw runtime_error("No calibration points available");
    }
    if(d.frequency <= points.front().frequency) {
        // use first point even for lower frequencies
        return points.front();
    }
    if(d.frequency >= points.back().frequency) {
        // use last point even for higher frequencies
        return points.back();
    }
    auto p = lower_bound(points.begin(), points.end(), d.frequency, [](Point p, uint64_t freq) -> bool {
        return p.frequency < freq;
    });
    if(p->frequency == d.frequency) {
        // Exact match, return point
        return *p;
    }
    // need to interpolate
    auto high = p;
    p--;
    auto low = p;
    double alpha = (d.frequency - low->frequency) / (high->frequency - low->frequency);
    Point ret;
    ret.frequency = d.frequency;
    ret.fe00 = low->fe00 * (1 - alpha) + high->fe00 * alpha;
    ret.fe11 = low->fe11 * (1 - alpha) + high->fe11 * alpha;
    ret.fe22 = low->fe22 * (1 - alpha) + high->fe22 * alpha;
    ret.fe30 = low->fe30 * (1 - alpha) + high->fe30 * alpha;
    ret.re03 = low->re03 * (1 - alpha) + high->re03 * alpha;
    ret.re11 = low->re11 * (1 - alpha) + high->re11 * alpha;
    ret.re22 = low->re22 * (1 - alpha) + high->re22 * alpha;
    ret.re33 = low->re33 * (1 - alpha) + high->re33 * alpha;
    ret.fe10e01 = low->fe10e01 * (1 - alpha) + high->fe10e01 * alpha;
    ret.fe10e32 = low->fe10e32 * (1 - alpha) + high->fe10e32 * alpha;
    ret.re23e01 = low->re23e01 * (1 - alpha) + high->re23e01 * alpha;
    ret.re23e32 = low->re23e32 * (1 - alpha) + high->re23e32 * alpha;
    return ret;
}

void Calibration::computeSOL(std::complex<double> s_m, std::complex<double> o_m, std::complex<double> l_m,
                             std::complex<double> &directivity, std::complex<double> &match, std::complex<double> &tracking,
                             std::complex<double> o_c, std::complex<double> s_c, std::complex<double> l_c)
{
    // equations from page 13 of http://www2.electron.frba.utn.edu.ar/~jcecconi/Bibliografia/04%20-%20Param_S_y_VNA/Network_Analyzer_Error_Models_and_Calibration_Methods.pdf
    // solved while taking non ideal o/s/l standards into account
    auto denom = l_c * o_c * (o_m - l_m) + l_c * s_c * (l_m - s_m) + o_c * s_c * (s_m - o_m);
    directivity = (l_c * o_m * (s_m * (o_c - s_c) + l_m * s_c) - l_c * o_c * l_m * s_m + o_c * l_m * s_c * (s_m - o_m)) / denom;
    match = (l_c * (o_m - s_m) + o_c * (s_m - l_m) + s_c * (l_m - o_m)) / denom;
    auto delta = (l_c * l_m * (o_m - s_m) + o_c * o_m * (s_m - l_m) + s_c * s_m * (l_m - o_m)) / denom;
    tracking = directivity * match - delta;
}

std::complex<double> Calibration::correctSOL(std::complex<double> measured, std::complex<double> directivity, std::complex<double> match, std::complex<double> tracking)
{
    return (measured - directivity) / (measured * match - directivity * match + tracking);
}

Calkit &Calibration::getCalibrationKit()
{
    return kit;
}

void Calibration::setCalibrationKit(const Calkit &value)
{
    kit = value;
}

Calibration::Type Calibration::getType() const
{
    return type;
}


