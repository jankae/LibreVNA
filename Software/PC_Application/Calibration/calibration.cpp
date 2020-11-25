#include "calibration.h"
#include <algorithm>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>
#include "unit.h"
#include <QDebug>

using namespace std;

Calibration::Calibration()
{
    // Create vectors for measurements
    measurements[Measurement::Port1Open].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port1Short].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port1Load].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port2Open].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port2Short].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Port2Load].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Isolation].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Through].datapoints = vector<Protocol::Datapoint>();
    measurements[Measurement::Line].datapoints = vector<Protocol::Datapoint>();

    type = Type::None;
}

void Calibration::clearMeasurements()
{
    qDebug() << "Clearing all calibration measurements...";
    for(auto m : measurements) {
        clearMeasurement(m.first);
    }
}

void Calibration::clearMeasurement(Calibration::Measurement type)
{
    measurements[type].datapoints.clear();
    measurements[type].timestamp = QDateTime();
    qDebug() << "Deleted" << MeasurementToString(type) << "measurement";
}

void Calibration::addMeasurement(Calibration::Measurement type, Protocol::Datapoint &d)
{
    measurements[type].datapoints.push_back(d);
    measurements[type].timestamp = QDateTime::currentDateTime();
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
    double kit_minFreq = kit.minFreq(isTRL);
    double kit_maxFreq = kit.maxFreq(isTRL);
    if(minFreq < kit_minFreq || maxFreq > kit_maxFreq) {
        // Calkit does not support complete calibration range
        QString msg = QString("The calibration kit does not support the complete span.\n\n")
                + "The measured calibration data covers " + Unit::ToString(minFreq, "Hz", " kMG", 4) + " to " + Unit::ToString(maxFreq, "Hz", " kMG", 4)
                + ", however the calibration kit is only valid from " + Unit::ToString(kit_minFreq, "Hz", " kMG", 4) + " to " + Unit::ToString(kit_maxFreq, "Hz", " kMG", 4) + ".\n\n"
                + "Please adjust the calibration kit or the span and take the calibration measurements again.";
        QMessageBox::critical(nullptr, "Unable to perform calibration", msg);
        qWarning() << msg;
        return false;
    }
    switch(type) {
    case Type::Port1SOL: constructPort1SOL(); break;
    case Type::Port2SOL: constructPort2SOL(); break;
    case Type::FullSOLT: construct12TermPoints(); break;
    case Type::TransmissionNormalization: constructTransmissionNormalization(); break;
    case Type::TRL: constructTRL(); break;
    case Type::None: break;
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

        auto actual = kit.toSOLT(p.frequency);
        // Forward calibration
        computeSOL(S11_short, S11_open, S11_load, p.fe00, p.fe11, p.fe10e01, actual.Open, actual.Short, actual.Load);
        p.fe30 = S21_isolation;
        // See page 18 of https://www.rfmentor.com/sites/default/files/NA_Error_Models_and_Cal_Methods.pdf
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
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port1Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port1Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S11_open = complex<double>(measurements[Measurement::Port1Open].datapoints[i].real_S11, measurements[Measurement::Port1Open].datapoints[i].imag_S11);
        auto S11_short = complex<double>(measurements[Measurement::Port1Short].datapoints[i].real_S11, measurements[Measurement::Port1Short].datapoints[i].imag_S11);
        auto S11_load = complex<double>(measurements[Measurement::Port1Load].datapoints[i].real_S11, measurements[Measurement::Port1Load].datapoints[i].imag_S11);
        // OSL port1
        auto actual = kit.toSOLT(p.frequency);
        // See page 13 of https://www.rfmentor.com/sites/default/files/NA_Error_Models_and_Cal_Methods.pdf
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
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Port2Open].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Port2Open].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S22_open = complex<double>(measurements[Measurement::Port2Open].datapoints[i].real_S22, measurements[Measurement::Port2Open].datapoints[i].imag_S22);
        auto S22_short = complex<double>(measurements[Measurement::Port2Short].datapoints[i].real_S22, measurements[Measurement::Port2Short].datapoints[i].imag_S22);
        auto S22_load = complex<double>(measurements[Measurement::Port2Load].datapoints[i].real_S22, measurements[Measurement::Port2Load].datapoints[i].imag_S22);
        // OSL port2
        auto actual = kit.toSOLT(p.frequency);
        // See page 19 of https://www.rfmentor.com/sites/default/files/NA_Error_Models_and_Cal_Methods.pdf
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

void Calibration::constructTransmissionNormalization()
{
    points.clear();
    for(unsigned int i = 0;i<measurements[Measurement::Through].datapoints.size();i++) {
        Point p;
        p.frequency = measurements[Measurement::Through].datapoints[i].frequency;
        // extract required complex reflection/transmission factors from datapoints
        auto S21_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S21, measurements[Measurement::Through].datapoints[i].imag_S21);
        auto S12_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S12, measurements[Measurement::Through].datapoints[i].imag_S12);
        auto actual = kit.toSOLT(p.frequency);
        p.fe10e32 = S21_through / actual.ThroughS21;
        p.re23e01 = S12_through / actual.ThroughS12;
        // All other calibration coefficients to ideal values
        p.fe30 = 0.0;
        p.fe22 = 0.0;
        p.fe00 = 0.0;
        p.fe11 = 0.0;
        p.fe10e01 = 1.0;
        p.re03 = 0.0;
        p.re11 = 0.0;
        p.re33 = 0.0;
        p.re22 = 0.0;
        p.re23e32 = 1.0;
        points.push_back(p);
    }
}

template<typename T>
class Tparam {
public:
    Tparam(){};
    Tparam(T t11, T t12, T t21, T t22)
        : t11(t11), t12(t12), t21(t21), t22(t22){};
    void fromSparam(T S11, T S21, T S12, T S22) {
        t11 = -(S11*S22 - S12*S21) / S21;
        t12 = S11 / S21;
        t21 = -S22 / S21;
        t22 = 1.0 / S21;
    }
    void toSparam(T &S11, T &S21, T &S12, T &S22) {
        S11 = t12 / t22;
        S21 = T(1) / t22;
        S12 = (t11*t22 - t12*t21) / t22;
        S22 = -t21 / t22;
    }
    Tparam inverse() {
        Tparam i;
        T det = t11*t22 - t12*t21;
        i.t11 = t22 / det;
        i.t12 = -t12 / det;
        i.t21 = -t21 / det;
        i.t22 = t11 / det;
        return i;
    }
    Tparam operator*(const Tparam &r) {
        Tparam p;
        p.t11 = t11*r.t11 + t12*r.t21;
        p.t12 = t11*r.t12 + t12*r.t22;
        p.t21 = t21*r.t11 + t22*r.t21;
        p.t22 = t21*r.t12 + t22*r.t22;
        return p;
    }
    Tparam operator*(const T &r) {
        Tparam p;
        p.t11 = t11 * r;
        p.t12 = t12 * r;
        p.t21 = t21 * r;
        p.t22 = t22 * r;
        return p;
    }
    T t11, t12, t21, t22;
};

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
        auto S11_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S11, measurements[Measurement::Through].datapoints[i].imag_S11);
        auto S21_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S21, measurements[Measurement::Through].datapoints[i].imag_S21);
        auto S22_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S22, measurements[Measurement::Through].datapoints[i].imag_S22);
        auto S12_through = complex<double>(measurements[Measurement::Through].datapoints[i].real_S12, measurements[Measurement::Through].datapoints[i].imag_S12);
        auto S11_line = complex<double>(measurements[Measurement::Line].datapoints[i].real_S11, measurements[Measurement::Line].datapoints[i].imag_S11);
        auto S21_line = complex<double>(measurements[Measurement::Line].datapoints[i].real_S21, measurements[Measurement::Line].datapoints[i].imag_S21);
        auto S22_line = complex<double>(measurements[Measurement::Line].datapoints[i].real_S22, measurements[Measurement::Line].datapoints[i].imag_S22);
        auto S12_line = complex<double>(measurements[Measurement::Line].datapoints[i].real_S12, measurements[Measurement::Line].datapoints[i].imag_S12);
        auto trl = kit.toTRL(p.frequency);
        complex<double> S11_reflection, S22_reflection;
        if(trl.reflectionIsNegative) {
            // used short
            S11_reflection = complex<double>(measurements[Measurement::Port1Short].datapoints[i].real_S11, measurements[Measurement::Port1Short].datapoints[i].imag_S11);
            S22_reflection = complex<double>(measurements[Measurement::Port2Short].datapoints[i].real_S22, measurements[Measurement::Port2Short].datapoints[i].imag_S22);
        } else {
            // used open
            S11_reflection = complex<double>(measurements[Measurement::Port1Open].datapoints[i].real_S11, measurements[Measurement::Port1Open].datapoints[i].imag_S11);
            S22_reflection = complex<double>(measurements[Measurement::Port2Open].datapoints[i].real_S22, measurements[Measurement::Port2Open].datapoints[i].imag_S22);
        }
        // calculate TRL calibration
        // variable names and formulas according to http://emlab.uiuc.edu/ece451/notes/new_TRL.pdf
        // page 19
        auto R_T = Tparam<complex<double>>();
        auto R_D = Tparam<complex<double>>();
        R_T.fromSparam(S11_through, S21_through, S12_through, S22_through);
        R_D.fromSparam(S11_line, S21_line, S12_line, S22_line);
        auto T = R_D*R_T.inverse();
        complex<double> a_over_c, b;
        // page 21-22
        solveQuadratic(T.t21, T.t22 - T.t11, -T.t12, b, a_over_c);
        // ensure correct root selection
        // page 23
        if(abs(b) >= abs(a_over_c)) {
            swap(b, a_over_c);
        }
        // page 24
        auto g = R_T.t22;
        auto d = R_T.t11 / g;
        auto e = R_T.t12 / g;
        auto f = R_T.t21 / g;

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
        auto Box_A = Tparam<complex<double>>(r22 * a, r22 * b, r22 * c, r22);
        auto Box_B = Tparam<complex<double>>(rho22 * alpha, rho22 * beta, rho22 * gamma, rho22);
        complex<double> dummy1, dummy2;
        Box_A.toSparam(p.fe00, dummy1, p.fe10e01, p.fe11);
        Box_B.toSparam(p.fe22, p.fe10e32, dummy1, dummy2);
        // no isolation measurement available
        p.fe30 = 0.0;

        // Reverse coefficients, normalize for S12 = 1.0
        // => det(T)/T22 = 1.0
        // => (rho22*alpa*rho22 - rho22*beta*rho*gamma)/rho22 = 1.0
        // => rho22*alpha - rho22*beta*gamma = 1.0
        // => rho22 = 1.0/(alpha - beta * gamma)
        rho22 = 1.0/(alpha - beta * gamma);
        r22 = r22_rho22 / rho22;

        Box_A = Tparam<complex<double>>(r22 * a, r22 * b, r22 * c, r22);
        Box_B = Tparam<complex<double>>(rho22 * alpha, rho22 * beta, rho22 * gamma, rho22);
        Box_A.toSparam(dummy1, dummy2, p.re23e01, p.re11);
        Box_B.toSparam(p.re22, p.re23e32, dummy1, p.re33);
        // no isolation measurement available
        p.re03 = 0.0;

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
    uint32_t f_step;
    if(settings.points > 1) {
        f_step = (settings.f_stop - settings.f_start) / (settings.points - 1);
    } else {
        f_step = settings.f_stop - settings.f_start;
    }
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
    case Measurement::Line:
        return "Line";
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
    } catch (runtime_error e) {
        QMessageBox::warning(nullptr, "Missing calibration kit", "The calibration kit file associated with the selected calibration could not be parsed. The calibration might not be accurate. (" + QString(e.what()) + ")");
        qWarning() << "Parsing of calibration kit failed while opening calibration file: " << e.what();
    }

    ifstream file;
    file.open(filename.toStdString());
    try {
        file >> *this;
    } catch(runtime_error e) {
        QMessageBox::warning(nullptr, "File parsing error", e.what());
        qWarning() << "Calibration file parsing failed: " << e.what();
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

    if(filename.endsWith(".cal")) {
        filename.chop(4);
    }
    auto calibration_file = filename + ".cal";
    ofstream file;
    file.open(calibration_file.toStdString());
    file << *this;

    auto calkit_file = filename + ".calkit";
    qDebug() << "Saving associated calibration kit to file" << calkit_file;
    kit.toFile(calkit_file);

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
        QString qLine = QString::fromStdString(line).simplified();
        for(auto m : c.Measurements()) {
            if(Calibration::MeasurementToString(m) == qLine) {
                // this is the correct measurement
                c.measurements[m].datapoints.clear();
                uint timestamp;
                in >> timestamp;
                c.measurements[m].timestamp = QDateTime::fromSecsSinceEpoch(timestamp);
                unsigned int points;
                in >> points;
                qDebug() << "Found measurement" << Calibration::MeasurementToString(m) << ", containing" << points << "points";
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
            if(Calibration::TypeToString(t) == qLine) {
                // try to apply this calibration type
                qDebug() << "Specified calibration in file is" << Calibration::TypeToString(t);
                if(c.calculationPossible(t)) {
                    c.constructErrorTerms(t);
                } else {
                    throw runtime_error("Incomplete calibration data, the requested \"" + line + "\"-Calibration could not be performed.");
                }
                break;
            }
        }
    }
    qDebug() << "Calibration file parsing complete";
    return in;
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


