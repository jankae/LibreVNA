#include "calibration.h"

#include "unit.h"
#include "Tools/parameters.h"
#include "CustomWidgets/informationbox.h"

#include <QDebug>
#include <algorithm>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>

using namespace std;

Calibration::Calibration()
{
    // Create vectors for measurements
    measurements[Measurement::Port1Open].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Port1Short].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Port1Load].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Port2Open].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Port2Short].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Port2Load].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Isolation].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Through].datapoints = vector<VirtualDevice::VNAMeasurement>();
    measurements[Measurement::Line].datapoints = vector<VirtualDevice::VNAMeasurement>();

    type = Type::None;
    port1Standard = port2Standard = PortStandard::Male;
    throughZeroLength = false;
}

Calibration::Standard Calibration::getPort1Standard(Calibration::Measurement m)
{
    switch(m) {
    case Measurement::Port1Open: return Standard::Open;
    case Measurement::Port1Short: return Standard::Short;
    case Measurement::Port1Load: return Standard::Load;
    case Measurement::Port2Open: return Standard::Any;
    case Measurement::Port2Short: return Standard::Any;
    case Measurement::Port2Load: return Standard::Any;
    case Measurement::Through: return Standard::Through;
    case Measurement::Isolation: return Standard::Load;
    case Measurement::Line: return Standard::Through;
    default: return Standard::Any;
    }
}

Calibration::Standard Calibration::getPort2Standard(Calibration::Measurement m)
{
    switch(m) {
    case Measurement::Port1Open: return Standard::Any;
    case Measurement::Port1Short: return Standard::Any;
    case Measurement::Port1Load: return Standard::Any;
    case Measurement::Port2Open: return Standard::Open;
    case Measurement::Port2Short: return Standard::Short;
    case Measurement::Port2Load: return Standard::Load;
    case Measurement::Through: return Standard::Through;
    case Measurement::Isolation: return Standard::Load;
    case Measurement::Line: return Standard::Through;
    default: return Standard::Any;
    }
}

void Calibration::clearMeasurements()
{
    qDebug() << "Clearing all calibration measurements...";
    for(auto m : measurements) {
        clearMeasurement(m.first);
    }
}

void Calibration::clearMeasurements(std::set<Calibration::Measurement> types)
{
    for(auto t : types) {
        clearMeasurement(t);
    }
}

void Calibration::clearMeasurement(Calibration::Measurement type)
{
    measurements[type].datapoints.clear();
    measurements[type].timestamp = QDateTime();
    qDebug() << "Deleted" << MeasurementToString(type) << "measurement";
}

void Calibration::addMeasurement(Calibration::Measurement type, VirtualDevice::VNAMeasurement &d)
{
    measurements[type].datapoints.push_back(d);
    measurements[type].timestamp = QDateTime::currentDateTime();
}

void Calibration::addMeasurements(std::set<Calibration::Measurement> types, VirtualDevice::VNAMeasurement &d)
{
    for(auto t : types) {
        addMeasurement(t, d);
    }
}

bool Calibration::calculationPossible(Calibration::Type type)
{
    if(type == Type::None) {
        // always possible to reset to None
        return true;
    }
    qDebug() << "Checking if" << TypeToString(type) << "calibration is possible...";
    auto ret = SanityCheckSamples(Measurements(type, false));
    if(ret) {
        qDebug() << "...calibration possible";
    } else {
        qDebug() << "...calibration not possible";
    }
    return SanityCheckSamples(Measurements(type, false));
}

bool Calibration::constructErrorTerms(Calibration::Type type)
{
    if(type == Type::None) {
        resetErrorTerms();
        return true;
    }
    if(!calculationPossible(type)) {
        return false;
    }
    qDebug() << "Constructing error terms for" << TypeToString(type) << "calibration";
    bool isTRL = type == Type::TRL;
    bool uses_male = true;
    bool uses_female = true;
    if(!kit.checkIfValid(minFreq, maxFreq, isTRL, uses_male, uses_female)) {
        // TODO adjust for male/female standards
        // Calkit does not support complete calibration range
        QString msg = QString("The calibration kit does not support the complete span.\n\n")
                + "The measured calibration data covers " + Unit::ToString(minFreq, "Hz", " kMG", 4) + " to " + Unit::ToString(maxFreq, "Hz", " kMG", 4)
                + ", however the calibration kit does not support the whole frequency range.\n\n"
                + "Please adjust the calibration kit or the span and take the calibration measurements again.";
        InformationBox::ShowError("Unable to perform calibration", msg);
        qWarning() << msg;
        return false;
    }
    // check calkit standards and adjust if necessary
    if(!kit.hasSeparateMaleFemaleStandards()) {
        port1Standard = PortStandard::Male;
        port2Standard = PortStandard::Male;
    }
    if(port1Standard == port2Standard) {
        // unable to use zero-length through
        throughZeroLength = false;
    }
    switch(type) {
    case Type::Port1SOL: constructPort1SOL(); break;
    case Type::Port2SOL: constructPort2SOL(); break;
    case Type::FullSOLT: construct12TermPoints(); break;
    case Type::TransmissionNormalization: constructTransmissionNormalization(); break;
    case Type::TRL: constructTRL(); break;
    default: break;
    }
    this->type = type;
    return true;
}

void Calibration::resetErrorTerms()
{
    type = Type::None;
    points.clear();
    qDebug() << "Error terms reset";
}

void Calibration::construct12TermPoints()
{
    std::vector<Measurement> requiredMeasurements = Measurements(Type::FullSOLT);
    requiredMeasurements.push_back(Measurement::Isolation);
    bool isolation_measured = SanityCheckSamples(requiredMeasurements);

    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port1Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port1Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S11_open = measurements[Measurement::Port1Open].datapoints[i].measurements["S11"];
        auto S11_short = measurements[Measurement::Port1Short].datapoints[i].measurements["S11"];
        auto S11_load = measurements[Measurement::Port1Load].datapoints[i].measurements["S11"];
        auto S22_open = measurements[Measurement::Port2Open].datapoints[i].measurements["S22"];
        auto S22_short = measurements[Measurement::Port2Short].datapoints[i].measurements["S22"];
        auto S22_load = measurements[Measurement::Port2Load].datapoints[i].measurements["S22"];
        auto S21_isolation = complex<double>(0,0);
        auto S12_isolation = complex<double>(0,0);
        if(isolation_measured) {
            S21_isolation = measurements[Measurement::Isolation].datapoints[i].measurements["S21"];
            S12_isolation = measurements[Measurement::Isolation].datapoints[i].measurements["S12"];
        }
        auto S11_through = measurements[Measurement::Through].datapoints[i].measurements["S11"];
        auto S21_through = measurements[Measurement::Through].datapoints[i].measurements["S21"];
        auto S22_through = measurements[Measurement::Through].datapoints[i].measurements["S22"];
        auto S12_through = measurements[Measurement::Through].datapoints[i].measurements["S12"];

        auto actual = kit.toSOLT(p.frequency, port1Standard == PortStandard::Male);
        // Forward calibration
        computeSOL(S11_short, S11_open, S11_load, p.fe00, p.fe11, p.fe10e01, actual.Open, actual.Short, actual.Load);
        p.fe30 = S21_isolation;
        // See page 18 of https://www.rfmentor.com/sites/default/files/NA_Error_Models_and_Cal_Methods.pdf
        // Formulas for S11M and S21M solved for e22 and e10e32
        if (throughZeroLength) {
            // use ideal through
            actual.ThroughS11 = 0.0;
            actual.ThroughS12 = 1.0;
            actual.ThroughS21 = 1.0;
            actual.ThroughS22 = 0.0;
        }

        auto deltaS = actual.ThroughS11*actual.ThroughS22 - actual.ThroughS21 * actual.ThroughS12;
        p.fe22 = ((S11_through - p.fe00)*(1.0 - p.fe11 * actual.ThroughS11)-actual.ThroughS11*p.fe10e01)
                / ((S11_through - p.fe00)*(actual.ThroughS22-p.fe11*deltaS)-deltaS*p.fe10e01);
        p.fe10e32 = (S21_through - p.fe30)*(1.0 - p.fe11*actual.ThroughS11 - p.fe22*actual.ThroughS22 + p.fe11*p.fe22*deltaS) / actual.ThroughS21;
        // Reverse calibration
        actual = kit.toSOLT(p.frequency, port2Standard == PortStandard::Male);
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
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port1Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port1Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S11_open = measurements[Measurement::Port1Open].datapoints[i].measurements["S11"];
        auto S11_short = measurements[Measurement::Port1Short].datapoints[i].measurements["S11"];
        auto S11_load = measurements[Measurement::Port1Load].datapoints[i].measurements["S11"];
        // OSL port1
        auto actual = kit.toSOLT(p.frequency, port1Standard == PortStandard::Male);
        // See page 13 of https://www.rfmentor.com/sites/default/files/NA_Error_Models_and_Cal_Methods.pdf
        computeSOL(S11_short, S11_open, S11_load, p.fe00, p.fe11, p.fe10e01, actual.Open, actual.Short, actual.Load);
        // All other calibration coefficients to ideal values
        p.fex = 0.0;
        p.fe30 = 0.0;
        p.fe22 = 0.0;
        p.fe10e32 = 1.0;
        p.re33 = 0.0;
        p.re22 = 0.0;
        p.re23e32 = 1.0;
        p.re03 = 0.0;
        p.rex = 0.0;
        p.re11 = 0.0;
        p.re23e01 = 1.0;
        points.push_back(p);
    }
}

void Calibration::constructPort2SOL()
{
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port2Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port2Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S22_open = measurements[Measurement::Port2Open].datapoints[i].measurements["S22"];
        auto S22_short = measurements[Measurement::Port2Short].datapoints[i].measurements["S22"];
        auto S22_load = measurements[Measurement::Port2Load].datapoints[i].measurements["S22"];
        // OSL port2
        auto actual = kit.toSOLT(p.frequency, port1Standard == PortStandard::Male);
        // See page 19 of https://www.rfmentor.com/sites/default/files/NA_Error_Models_and_Cal_Methods.pdf
        computeSOL(S22_short, S22_open, S22_load, p.re33, p.re22, p.re23e32, actual.Open, actual.Short, actual.Load);
        // All other calibration coefficients to ideal values
        p.fex = 0.0;
        p.fe30 = 0.0;
        p.fe22 = 0.0;
        p.fe10e32 = 1.0;
        p.fe00 = 0.0;
        p.fe11 = 0.0;
        p.fe10e01 = 1.0;
        p.re03 = 0.0;
        p.rex = 0.0;
        p.re11 = 0.0;
        p.re23e01 = 1.0;
        points.push_back(p);
    }
}

void Calibration::constructTransmissionNormalization()
{
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Through].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Through].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S21_through = measurements[Measurement::Through].datapoints[i].measurements["S21"];
        auto S12_through = measurements[Measurement::Through].datapoints[i].measurements["S12"];
        auto actual = kit.toSOLT(p.frequency);
        p.fe10e32 = S21_through / actual.ThroughS21;
        p.re23e01 = S12_through / actual.ThroughS12;
        // All other calibration coefficients to ideal values
        p.fe30 = 0.0;
        p.fex = 0.0;
        p.fe22 = 0.0;
        p.fe00 = 0.0;
        p.fe11 = 0.0;
        p.fe10e01 = 1.0;
        p.re03 = 0.0;
        p.rex = 0.0;
        p.re11 = 0.0;
        p.re33 = 0.0;
        p.re22 = 0.0;
        p.re23e32 = 1.0;
        points.push_back(p);
    }
}

template<typename T> void solveQuadratic(T a, T b, T c, T &result1, T &result2)
{
    T root = sqrt(b * b - T(4) * a * c);
    result1 = (-b + root) / (T(2) * a);
    result2 = (-b - root) / (T(2) * a);
}

void Calibration::constructTRL()
{
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Through].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Through].datapoints[i].frequency;

        // grab raw measurements
        auto S11_through = measurements[Measurement::Through].datapoints[i].measurements["S11"];
        auto S21_through = measurements[Measurement::Through].datapoints[i].measurements["S21"];
        auto S22_through = measurements[Measurement::Through].datapoints[i].measurements["S22"];
        auto S12_through = measurements[Measurement::Through].datapoints[i].measurements["S12"];
        auto S11_line = measurements[Measurement::Line].datapoints[i].measurements["S11"];
        auto S21_line = measurements[Measurement::Line].datapoints[i].measurements["S21"];
        auto S22_line = measurements[Measurement::Line].datapoints[i].measurements["S22"];
        auto S12_line = measurements[Measurement::Line].datapoints[i].measurements["S12"];
        auto trl = kit.toTRL(p.frequency);
        complex<double> S11_reflection, S22_reflection;
        if(trl.reflectionIsNegative) {
            // used short
            S11_reflection = measurements[Measurement::Port1Short].datapoints[i].measurements["S11"];
            S22_reflection = measurements[Measurement::Port2Short].datapoints[i].measurements["S22"];
        } else {
            // used open
            S11_reflection = measurements[Measurement::Port1Open].datapoints[i].measurements["S11"];
            S22_reflection = measurements[Measurement::Port2Open].datapoints[i].measurements["S22"];
        }
        // calculate TRL calibration
        // variable names and formulas according to http://emlab.uiuc.edu/ece451/notes/new_TRL.pdf
        // page 19
        Sparam Sthrough(S11_through, S12_through, S21_through, S22_through);
        Sparam Sline(S11_line, S12_line, S21_line, S22_line);
        auto R_T = Tparam(Sthrough);
        auto R_D = Tparam(Sline);
        auto T = R_D*R_T.inverse();
        complex<double> a_over_c, b;
        // page 21-22
        solveQuadratic(T.m21, T.m22 - T.m11, -T.m12, b, a_over_c);
        // ensure correct root selection
        // page 23
        if(abs(b) >= abs(a_over_c)) {
            swap(b, a_over_c);
        }
        // page 24
        auto g = R_T.m22;
        auto d = R_T.m11 / g;
        auto e = R_T.m12 / g;
        auto f = R_T.m21 / g;

        // page 25
        auto r22_rho22 = g * (1.0 - e / a_over_c) / (1.0 - b / a_over_c);
        auto gamma = (f - d / a_over_c) / (1.0 - e / a_over_c);
        auto beta_over_alpha = (e - b) / (d - b * f);
        // page 26
        auto alpha_a = (d - b * f) / (1.0 - e / a_over_c);
        auto w1 = S11_reflection;
        auto w2 = S22_reflection;
        // page 28
        auto a = sqrt((w1 - b) / (w2 + gamma) * (1.0 + w2 * beta_over_alpha) / (1.0 - w1 / a_over_c) * alpha_a);
        // page 29, check sign of a
        auto reflection = (w1 - b) / (a * (1.0 - w1 / a_over_c));
        if((reflection.real() > 0 && trl.reflectionIsNegative) || (reflection.real() < 0 && !trl.reflectionIsNegative)) {
            // wrong sign for a
            a = -a;
        }
        // Revert back from error boxes with T parameters to S paramaters,
        // page 17 + formulas for calculating S parameters from T parameters.
        // Forward coefficients, normalize for S21 = 1.0 -> r22 = 1.0
        auto r22 = complex<double>(1.0);
        auto rho22 = r22_rho22 / r22;
        auto alpha = alpha_a / a;
        auto beta = beta_over_alpha * alpha;
        auto c = a / a_over_c;
        auto Box_A = Tparam(r22 * a, r22 * b, r22 * c, r22);
        auto Box_B = Tparam(rho22 * alpha, rho22 * beta, rho22 * gamma, rho22);
        auto S_A = Sparam(Box_A);
        p.fe00 = S_A.m11;
        p.fe10e01 = S_A.m12;
        p.fe11 = S_A.m22;
        auto S_B = Sparam(Box_B);
        p.fe22 = S_B.m11;
        p.fe10e32 = S_B.m21;
        // no isolation measurement available
        p.fe30 = 0.0;
        p.fex = 0.0;

        // Reverse coefficients, normalize for S12 = 1.0
        // => det(T)/T22 = 1.0
        // => (rho22*alpa*rho22 - rho22*beta*rho*gamma)/rho22 = 1.0
        // => rho22*alpha - rho22*beta*gamma = 1.0
        // => rho22 = 1.0/(alpha - beta * gamma)
        rho22 = 1.0/(alpha - beta * gamma);
        r22 = r22_rho22 / rho22;

        Box_A = Tparam(r22 * a, r22 * b, r22 * c, r22);
        Box_B = Tparam(rho22 * alpha, rho22 * beta, rho22 * gamma, rho22);
        S_A = Sparam(Box_A);
        p.re23e01 = S_A.m12;
        p.re11 = S_A.m22;
        S_B = Sparam(Box_B);
        p.re22 = S_B.m11;
        p.re23e32 = S_B.m21;
        p.re33 = S_B.m22;
        // no isolation measurement available
        p.re03 = 0.0;
        p.rex = 0.0;

        points.push_back(p);
    }
}

void Calibration::correctMeasurement(VirtualDevice::VNAMeasurement &d)
{
    if(type == Type::None) {
        // No calibration data, do nothing
        return;
    }
    // Extract measurement S parameters
    auto S11m = d.measurements["S11"];
    auto S21m = d.measurements["S21"];
    auto S22m = d.measurements["S22"];
    auto S12m = d.measurements["S12"];

    // find correct entry
    auto p = getCalibrationPoint(d);

    complex<double> S11, S12, S21, S22;

    // equations from page 19 of https://www.rfmentor.com/sites/default/files/NA_Error_Models_and_Cal_Methods.pdf
    auto denom = (1.0 + (S11m - p.fe00) / p.fe10e01 * p.fe11) * (1.0 + (S22m - p.re33) / p.re23e32 * p.re22)
            - (S21m - p.fe30) / p.fe10e32 * (S12m - p.re03) / p.re23e01 * p.fe22 * p.re11;
    S11 = ((S11m - p.fe00) / p.fe10e01 * (1.0 + (S22m - p.re33) / p.re23e32 * p.re22)
            - p.fe22 * (S21m - p.fe30) / p.fe10e32 * (S12m - p.re03) / p.re23e01) / denom;
    S21 = ((S21m - p.fe30) / p.fe10e32 * (1.0 + (S22m - p.re33) / p.re23e32 * (p.re22 - p.fe22))) / denom;
    S22 = ((S22m - p.re33) / p.re23e32 * (1.0 + (S11m - p.fe00) / p.fe10e01 * p.fe11)
            - p.re11 * (S21m - p.fe30) / p.fe10e32 * (S12m - p.re03) / p.re23e01) / denom;
    S12 = ((S12m - p.re03) / p.re23e01 * (1.0 + (S11m - p.fe00) / p.fe10e01 * (p.fe11 - p.re11))) / denom;

    auto S = Sparam(S11, S12, S21, S22);
    d.measurements["S11"] = S.m11;
    d.measurements["S12"] = S.m12;
    d.measurements["S21"] = S.m21;
    d.measurements["S22"] = S.m22;
}

void Calibration::correctTraces(Trace &S11, Trace &S12, Trace &S21, Trace &S22)
{
    auto points = Trace::assembleDatapoints(S11, S12, S21, S22);
    if(points.size()) {
        // succeeded in assembling datapoints
        for(auto &p : points) {
            correctMeasurement(p);
        }
        Trace::fillFromDatapoints(S11, S12, S21, S22, points);
    }
}

Calibration::InterpolationType Calibration::getInterpolation(double f_start, double f_stop, int npoints)
{
    if(!points.size()) {
        return InterpolationType::NoCalibration;
    }
    if(f_start < points.front().frequency || f_stop > points.back().frequency) {
        return InterpolationType::Extrapolate;
    }
    // Either exact or interpolation, check individual frequencies
    uint32_t f_step;
    if(npoints > 1) {
        f_step = (f_stop - f_start) / (npoints - 1);
    } else {
        f_step = f_stop - f_start;
    }
    uint64_t f = f_start;
    do {
        if(find_if(points.begin(), points.end(), [&f](const Point& p){
            return abs(f - p.frequency) < 100;
        }) == points.end()) {
            return InterpolationType::Interpolate;
        }
        f += f_step;
    } while(f <= f_stop && f_step > std::numeric_limits<double>::epsilon());

    // if we get here all frequency points were matched
    if(points.front().frequency == f_start && points.back().frequency == f_stop) {
        return InterpolationType::Unchanged;
    } else {
        return InterpolationType::Exact;
    }
}

Calibration::Measurement Calibration::MeasurementFromString(QString s)
{
    for(unsigned int i=0;i<(int)Measurement::Last;i++) {
        auto m = (Measurement) i;
        if(s.compare(MeasurementToString(m), Qt::CaseInsensitive)==0) {
            return m;
        }
    }
    return Measurement::Last;
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
    case Measurement::Line:
        return "Line";
    default:
        return "Unknown";
    }
}

Calibration::Type Calibration::TypeFromString(QString s)
{
    for(unsigned int i=0;i<(int)Type::Last;i++) {
        auto t = (Type) i;
        if(s.compare(TypeToString(t), Qt::CaseInsensitive)==0) {
            return t;
        }
    }
    return Type::Last;
}

QString Calibration::TypeToString(Calibration::Type t)
{
    switch(t) {
    case Type::Port1SOL: return "Port 1"; break;
    case Type::Port2SOL: return "Port 2"; break;
    case Type::FullSOLT: return "SOLT"; break;
    case Type::TransmissionNormalization: return "Normalize"; break;
    case Type::TRL: return "TRL"; break;
    default: return "None"; break;
    }
}

const std::vector<Calibration::Type> Calibration::Types()
{
    const std::vector<Calibration::Type> ret = {Type::Port1SOL, Type::Port2SOL, Type::FullSOLT, Type::TransmissionNormalization, Type::TRL};
    return ret;
}

const std::vector<Calibration::Measurement> Calibration::Measurements(Calibration::Type type, bool optional_included)
{
    switch(type) {
    case Type::None:
        // all possible measurements
        return {Measurement::Port1Short, Measurement::Port1Open, Measurement::Port1Load,
                    Measurement::Port2Short, Measurement::Port2Open, Measurement::Port2Load,
                    Measurement::Through, Measurement::Isolation, Measurement::Line};
    case Type::FullSOLT:
        if(optional_included) {
            return {Measurement::Port1Short, Measurement::Port1Open, Measurement::Port1Load, Measurement::Port2Short, Measurement::Port2Open, Measurement::Port2Load, Measurement::Through, Measurement::Isolation};
        } else {
            return {Measurement::Port1Short, Measurement::Port1Open, Measurement::Port1Load, Measurement::Port2Short, Measurement::Port2Open, Measurement::Port2Load, Measurement::Through};
        }
        break;
    case Type::Port1SOL:
        return {Measurement::Port1Short, Measurement::Port1Open, Measurement::Port1Load};
        break;
    case Type::Port2SOL:
        return {Measurement::Port2Short, Measurement::Port2Open, Measurement::Port2Load};
        break;
    case Type::TransmissionNormalization:
        return {Measurement::Through};
        break;
    case Type::TRL:
        if(kit.isTRLReflectionShort()) {
            return {Measurement::Through, Measurement::Port1Short, Measurement::Port2Short, Measurement::Line};
        } else {
            return {Measurement::Through, Measurement::Port1Open, Measurement::Port2Open, Measurement::Line};
        }
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
        break;
    case Measurement::Line:
        info.name = "Line";
        info.prerequisites = "Port 1 connected to port 2 via line standard";
        break;
    default:
        info.name = "Invalid";
        info.prerequisites = "Invalid";
        break;
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
        t->setCalibration();
        t->setReflection(reflection[i]);
        traces.push_back(t);
    }
    for(auto p : points) {
        Trace::Data d;
        d.x = p.frequency;
        for(int i=0;i<12;i++) {
            switch(i) {
            case 0: d.y = p.fe00; break;
            case 1: d.y = p.fe11; break;
            case 2: d.y = p.fe10e01; break;
            case 3: d.y = p.fe10e32; break;
            case 4: d.y = p.fe22; break;
            case 5: d.y = p.fe30; break;
            case 6: d.y = p.re33; break;
            case 7: d.y = p.re11; break;
            case 8: d.y = p.re23e32; break;
            case 9: d.y = p.re23e01; break;
            case 10: d.y = p.re22; break;
            case 11: d.y = p.re03; break;
            }
            traces[i]->addData(d, TraceMath::DataType::Frequency);
        }
    }
    return traces;
}

std::vector<Trace *> Calibration::getMeasurementTraces()
{
    std::vector<Trace*> traces;
    for(auto m : measurements) {
        auto info = getMeasurementInfo(m.first);
        if(info.points > 0) {
            vector<QString> usedPrefixes;
            switch(m.first) {
            case Measurement::Port1Load:
            case Measurement::Port1Open:
            case Measurement::Port1Short:
                usedPrefixes = {"S11"};
                break;
            case Measurement::Port2Load:
            case Measurement::Port2Open:
            case Measurement::Port2Short:
                usedPrefixes = {"S22"};
                break;
            case Measurement::Through:
            case Measurement::Line:
            case Measurement::Isolation:
                usedPrefixes = {"S11", "S12", "S21", "S22"};
                break;
            default:
                break;
            }
            for(auto prefix : usedPrefixes) {
                auto t = new Trace(prefix + " " + info.name);
                t->setCalibration();
                t->setReflection(prefix == "S11" || prefix == "S22");
                for(auto p : m.second.datapoints) {
                    Trace::Data d;
                    d.x = p.frequency;
                    d.y = p.measurements[prefix];
                    t->addData(d, TraceMath::DataType::Frequency);
                }
                traces.push_back(t);
            }
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

    // force correct file ending
    if(filename.toLower().endsWith(".cal")) {
        filename.chop(4);
        filename += ".cal";
    }

    qDebug() << "Attempting to open calibration from file" << filename;

    // reset all data before loading new calibration
    clearMeasurements();
    resetErrorTerms();

    // attempt to load associated calibration kit first (needs to be available when performing calibration)
    auto calkit_file = filename;
    auto dotPos = calkit_file.lastIndexOf('.');
    if(dotPos >= 0) {
        calkit_file.truncate(dotPos);
    }
    calkit_file.append(".calkit");
    qDebug() << "Associated calibration kit expected in" << calkit_file;
    try {
        kit = Calkit::fromFile(calkit_file);
    } catch (runtime_error &e) {
        InformationBox::ShowError("Missing calibration kit", "The calibration kit file associated with the selected calibration could not be parsed. The calibration might not be accurate. (" + QString(e.what()) + ")");
        qWarning() << "Parsing of calibration kit failed while opening calibration file: " << e.what();
    }

    ifstream file;

    file.open(filename.toStdString());
    if(!file.good()) {
        QString msg = "Unable to open file: "+filename;
        InformationBox::ShowError("Error", msg);
        qWarning() << msg;
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        fromJSON(j);
    } catch(exception &e) {
        InformationBox::ShowError("File parsing error", e.what());
        qWarning() << "Calibration file parsing failed: " << e.what();
        return false;
    }
    this->currentCalFile = filename;    // if all ok, remember this

    return true;
}

bool Calibration::saveToFile(QString filename)
{
    if(filename.isEmpty()) {
        QString fn = descriptiveCalName();
        filename = QFileDialog::getSaveFileName(nullptr, "Save calibration data", fn, "Calibration files (*.cal)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return false;
        }
    }

    if(filename.toLower().endsWith(".cal")) {
        filename.chop(4);
    }
    auto calibration_file = filename + ".cal";
    ofstream file;
    file.open(calibration_file.toStdString());
    file << setw(1) << toJSON();

    auto calkit_file = filename + ".calkit";
    qDebug() << "Saving associated calibration kit to file" << calkit_file;
    kit.toFile(calkit_file);
    this->currentCalFile = calibration_file;    // if all ok, remember this

    return true;
}

/**
 * @brief Calibration::hzToString
 * @param freqHz - input frequency in Hz
 * @return descriptive name ie. "SOLT 40M-700M 1000pt"
 */
QString Calibration::descriptiveCalName(){
    int precision = 3;
    QString lo = Unit::ToString(this->minFreq, "", " kMG", precision);
    QString hi = Unit::ToString(this->maxFreq, "", " kMG", precision);
    // due to rounding up 123.66M and 123.99M -> we get lo="124M" and hi="124M"
    // so let's add some precision
    if (lo == hi) {
        // Only in case of 123.66M and 123.69M we would need 5 digits, but that kind of narrow cal. is very unlikely.
        precision = 4;
        lo = Unit::ToString(this->minFreq, "", " kMG", precision);
        hi = Unit::ToString(this->maxFreq, "", " kMG", precision);
    }

    QString tmp =
            Calibration::TypeToString(this->getType())
            + " "
            + lo + "-" + hi
            + " "
            + QString::number(this->points.size()) + "pt";
    return tmp;
}

bool Calibration::getThroughZeroLength() const
{
    return throughZeroLength;
}

void Calibration::setThroughZeroLength(bool value)
{
    throughZeroLength = value;
}

double Calibration::getMinFreq(){
    return this->minFreq;
}
double Calibration::getMaxFreq(){
    return this->maxFreq;
}
int Calibration::getNumPoints(){
    return this->points.size();
}

nlohmann::json Calibration::toJSON()
{
    nlohmann::json j;
    nlohmann::json j_measurements;
    for(auto m : measurements) {
        if(m.second.datapoints.size() > 0) {
            nlohmann::json j_measurement;
            j_measurement["name"] = MeasurementToString(m.first).toStdString();
            j_measurement["timestamp"] = m.second.timestamp.toSecsSinceEpoch();
            nlohmann::json j_points;
            for(auto p : m.second.datapoints) {
                nlohmann::json j_point;
                j_point["frequency"] = p.frequency;
                j_point["S11_real"] = p.measurements["S11"].real();
                j_point["S11_imag"] = p.measurements["S11"].imag();
                j_point["S12_real"] = p.measurements["S12"].real();
                j_point["S12_imag"] = p.measurements["S12"].imag();
                j_point["S21_real"] = p.measurements["S21"].real();
                j_point["S21_imag"] = p.measurements["S21"].imag();
                j_point["S22_real"] = p.measurements["S22"].real();
                j_point["S22_imag"] = p.measurements["S22"].imag();
                j_points.push_back(j_point);
            }
            j_measurement["points"] = j_points;
            j_measurements.push_back(j_measurement);
        }
    }
    j["measurements"] = j_measurements;
    j["type"] = TypeToString(getType()).toStdString();
    j["port1StandardMale"] = port1Standard == PortStandard::Male;
    j["port2StandardMale"] = port2Standard == PortStandard::Male;
    j["throughZeroLength"] = throughZeroLength;

    return j;
}

void Calibration::fromJSON(nlohmann::json j)
{
    clearMeasurements();
    resetErrorTerms();
    port1Standard = j.value("port1StandardMale", true) ? PortStandard::Male : PortStandard::Female;
    port2Standard = j.value("port2StandardMale", true) ? PortStandard::Male : PortStandard::Female;
    throughZeroLength = j.value("throughZeroLength", false);
    if(j.contains("measurements")) {
        // grab measurements
        for(auto j_m : j["measurements"]) {
            if(!j_m.contains("name")) {
                throw runtime_error("Measurement without name given");
            }
            auto m = MeasurementFromString(QString::fromStdString(j_m["name"]));
            if(m == Measurement::Last) {
                throw runtime_error("Measurement name unknown: "+std::string(j_m["name"]));
            }
            // get timestamp
            measurements[m].timestamp = QDateTime::fromSecsSinceEpoch(j_m.value("timestamp", 0));
            // extract points
            if(!j_m.contains("points")) {
                throw runtime_error("Measurement "+MeasurementToString(m).toStdString()+" does not contain any points");
            }
            int pointNum = 0;
            for(auto j_p : j_m["points"]) {
                VirtualDevice::VNAMeasurement p;
                p.pointNum = pointNum++;
                p.frequency = j_p.value("frequency", 0.0);
                p.Z0 = 50.0;
                p.measurements["S11"] = complex<double>(j_p.value("S11_real", 0.0), j_p.value("S11_imag", 0.0));
                p.measurements["S12"] = complex<double>(j_p.value("S12_real", 0.0), j_p.value("S12_imag", 0.0));
                p.measurements["S21"] = complex<double>(j_p.value("S21_real", 0.0), j_p.value("S21_imag", 0.0));
                p.measurements["S22"] = complex<double>(j_p.value("S22_real", 0.0), j_p.value("S22_imag", 0.0));
                measurements[m].datapoints.push_back(p);
            }
        }
    }
    // got all measurements, construct calibration according to type
    if(j.contains("type")) {
        auto t = TypeFromString(QString::fromStdString(j["type"]));
        if(t == Type::Last) {
            throw runtime_error("Calibration type unknown: "+std::string(j["type"]));
        }
        if(calculationPossible(t)) {
            constructErrorTerms(t);
        } else {
            throw runtime_error("Incomplete calibration data, the requested calibration could not be performed.");
        }
    }
}

QString Calibration::getCurrentCalibrationFile(){
    return this->currentCalFile;
}

bool Calibration::SanityCheckSamples(const std::vector<Calibration::Measurement> &requiredMeasurements)
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

Calibration::Point Calibration::getCalibrationPoint(VirtualDevice::VNAMeasurement &d)
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
    ret.fex = low->fex * (1 - alpha) + high->fex * alpha;
    ret.re03 = low->re03 * (1 - alpha) + high->re03 * alpha;
    ret.rex = low->rex * (1 - alpha) + high->rex * alpha;
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

void Calibration::computeIsolation(std::complex<double> x0_m, std::complex<double> x1_m, std::complex<double> reverse_match, std::complex<double> reverse_tracking, std::complex<double> reverse_directivity, std::complex<double> x0, std::complex<double> x1, std::complex<double> &internal_isolation, std::complex<double> &external_isolation)
{
    external_isolation = (x1_m - x0_m)*(1.0 - reverse_match * (x1 - x0) + x1*x0*reverse_match*reverse_match) / (reverse_tracking * (x1 - x0));
    internal_isolation = x0_m - external_isolation*(reverse_directivity + reverse_tracking*x0 / (1.0 - x0*reverse_match));
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

void Calibration::setPortStandard(int port, Calibration::PortStandard standard)
{
    if(port == 1) {
        port1Standard = standard;
    } else if(port == 2) {
        port2Standard = standard;
    }
}

Calibration::PortStandard Calibration::getPortStandard(int port)
{
    if(port == 1) {
        return port1Standard;
    } else if(port == 2) {
        return port2Standard;
    } else {
        return PortStandard::Male;
    }
}

Calibration::Type Calibration::getType() const
{
    return type;
}


