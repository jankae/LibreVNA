#include "util.h"

#include <random>
#include <QVector2D>

void Util::unwrapPhase(std::vector<double> &phase, unsigned int start_index)
{
    for (unsigned int i = start_index + 1; i < phase.size(); i++) {
        int d = trunc(phase[i] - phase[i-1]) / M_PI;
        if(d > 0) {
            // there is larger than a 180° shift between this and the previous phase
            phase[i] -= 2*M_PI*(int)((d+1)/2);
        } else if(d < 0) {
            // there is larger than a -180° shift between this and the previous phase
            phase[i] -= 2*M_PI*(int)((d-1)/2);
        }
    }
}

void Util::linearRegression(const std::vector<double> &input, double &B_0, double &B_1)
{
    double x_mean = (input.size() - 1.0) / 2.0;
    double y_mean = std::accumulate(input.begin(), input.end(), 0.0) / input.size();
    double ss_xy = 0.0;
    for(unsigned int i=0;i<input.size();i++) {
        ss_xy += input[i] * i;
    }
    ss_xy -= input.size() * x_mean * y_mean;
    int n = input.size() - 1;
    double ss_xx = (1.0/6.0) * n * (n + 1) * (2*n + 1) - input.size() * x_mean * x_mean;

    B_1 = ss_xy / ss_xx;
    B_0 = y_mean - B_1 * x_mean;
}

double Util::distanceToLine(QPointF point, QPointF l1, QPointF l2, QPointF *closestLinePoint, double *pointRatio)
{
    auto M = l2 - l1;
    auto t0 = QPointF::dotProduct(M, point - l1) / QPointF::dotProduct(M, M);
    QPointF closestPoint;
    QVector2D orthVect;
    if (t0 <= 0) {
        orthVect = QVector2D(point - l1);
        closestPoint = l1;
        t0 = 0;
    } else if(t0 >= 1) {
        orthVect = QVector2D(point - l2);
        closestPoint = l2;
        t0 = 1;
    } else {
        auto intersect = l1 + t0 * M;
        orthVect = QVector2D(point - intersect);
        closestPoint = intersect;
    }
    if(closestLinePoint) {
        *closestLinePoint = closestPoint;
    }
    if(pointRatio) {
        *pointRatio = t0;
    }
    return orthVect.length();
}

std::complex<double> Util::SparamToImpedance(std::complex<double> d, std::complex<double> Z0) {
    if (d != 1.0) {
        return Z0 * (1.0 + d) / (1.0 - d);
    } else {
        return std::complex<double>(std::numeric_limits<double>::infinity(), 0.0);
    }
}

std::complex<double> Util::ImpedanceToSparam(std::complex<double> Z, std::complex<double> Z0) {
    if(std::isinf(Z.real())) {
        return 1.0;
    } else {
        return (Z-Z0)/(Z+Z0);
    }
}

double Util::dBmTodBuV(double dBm)
{
    double uVpower = 0.000001*0.000001/50.0;
    double dBdiff = 10*log10(uVpower*1000);
    return dBm - dBdiff;
}

double Util::dBuVTodBm(double dBuV)
{
    double uVpower = 0.000001*0.000001/50.0;
    double dBdiff = 10*log10(uVpower*1000);
    return dBuV + dBdiff;
}

unsigned long long Util::random(unsigned long long max)
{
    static std::random_device os_seed;
    static const unsigned long long seed = os_seed();
    static std::mt19937_64 generator(seed);

    std::uniform_int_distribution<unsigned long long> distribute(0, max);
    return distribute(generator);
}

std::complex<double> Util::findCenterOfCircle(const std::vector<std::complex<double> > &points)
{
    int i,iter,IterMAX=99;

    double Xi,Yi,Zi;
    double Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
    double A0,A1,A2,A22;
    double Dy,xnew,x,ynew,y;
    double DET,Xcenter,Ycenter;

    // find means
    double meanX = 0.0, meanY = 0.0;
    for(auto p : points) {
        meanX += p.real();
        meanY += p.imag();
    }
    meanX /= points.size();
    meanY /= points.size();

    //     computing moments

    Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;

    for (i=0; i<(int) points.size(); i++)
    {
        Xi = points[i].real() - meanX;   //  centered x-coordinates
        Yi = points[i].imag() - meanY;   //  centered y-coordinates
        Zi = Xi*Xi + Yi*Yi;

        Mxy += Xi*Yi;
        Mxx += Xi*Xi;
        Myy += Yi*Yi;
        Mxz += Xi*Zi;
        Myz += Yi*Zi;
        Mzz += Zi*Zi;
    }
    Mxx /= points.size();
    Myy /= points.size();
    Mxy /= points.size();
    Mxz /= points.size();
    Myz /= points.size();
    Mzz /= points.size();

    //    computing the coefficients of the characteristic polynomial

    Mz = Mxx + Myy;
    Cov_xy = Mxx*Myy - Mxy*Mxy;
    Var_z = Mzz - Mz*Mz;

    A2 = 4.0*Cov_xy - 3.0*Mz*Mz - Mzz;
    A1 = Var_z*Mz + 4.0*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
    A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
    A22 = A2 + A2;

    //    finding the root of the characteristic polynomial
    //    using Newton's method starting at x=0
    //     (it is guaranteed to converge to the right root)

    for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
        Dy = A1 + x*(A22 + 16.*x*x);
        xnew = x - y/Dy;
        if ((xnew == x)||(!isfinite(xnew))) break;
        ynew = A0 + xnew*(A1 + xnew*(A2 + 4.0*xnew*xnew));
        if (abs(ynew)>=abs(y))  break;
        x = xnew;  y = ynew;
    }

    //    computing paramters of the fitting circle

    DET = x*x - x*Mz + Cov_xy;
    Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/2.0;
    Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/2.0;

    //       assembling the output

    return std::complex<double>(Xcenter + meanX, Ycenter + meanY);
}

std::complex<double> Util::addTransmissionLine(std::complex<double> termination_reflection, double offset_impedance, double offset_delay, double offset_loss, double frequency)
{
    // nomenclature and formulas from https://loco.lab.asu.edu/loco-memos/edges_reports/report_20130807.pdf
    auto Gamma_T = termination_reflection;
    auto f = frequency;
    auto w = 2.0 * M_PI * frequency;
    auto f_sqrt = sqrt(f / 1e9);

    auto Z_c = std::complex<double>(offset_impedance + (offset_loss / (2*w)) * f_sqrt, -(offset_loss / (2*w)) * f_sqrt);
    auto gamma_l = std::complex<double>(offset_loss*offset_delay/(2*offset_impedance)*f_sqrt, w*offset_delay+offset_loss*offset_delay/(2*offset_impedance)*f_sqrt);

    auto Z_r = std::complex<double>(50.0);

    auto Gamma_1 = (Z_c - Z_r) / (Z_c + Z_r);

    auto Gamma_i = (Gamma_1*(1.0-exp(-2.0*gamma_l)-Gamma_1*Gamma_T)+exp(-2.0*gamma_l)*Gamma_T)
            / (1.0-Gamma_1*(exp(-2.0*gamma_l)*Gamma_1+Gamma_T*(1.0-exp(-2.0*gamma_l))));

    return Gamma_i;
}

QColor Util::getIntensityGradeColor(double intensity)
{
    if(intensity < 0.0) {
        return Qt::black;
    } else if(intensity > 1.0) {
        return Qt::white;
    } else if(intensity >= 0.0 && intensity <= 1.0) {
        return QColor::fromHsv(Util::Scale<double>(intensity, 0.0, 1.0, 240, 0), 255, 255);
    } else {
        return Qt::black;
    }
}

bool Util::firmwareEqualOrHigher(QString firmware, QString compare)
{
    QStringList f = firmware.split(".");
    QStringList c = compare.split(".");
    if(f.size() != 3 || c.size() != 3) {
        return false;
    }
    if(f[0].toInt() < c[0].toInt()) {
        return false;
    } else if(f[0].toInt() > c[0].toInt()) {
        return true;
    }
    if(f[1].toInt() < c[1].toInt()) {
        return false;
    } else if(f[1].toInt() > c[1].toInt()) {
        return true;
    }
    if(f[2].toInt() < c[2].toInt()) {
        return false;
    } else {
        return true;
    }
}

std::complex<double> Util::interpolateMagPhase(const std::complex<double> &from, const std::complex<double> &to, double alpha)
{
    auto magFrom = abs(from);
    auto magTo = abs(to);
    auto phaseFrom = arg(from);
    auto phaseTo = arg(to);
    // unwrap phase
    if(phaseTo - phaseFrom > M_PI) {
        phaseTo -= 2*M_PI;
    } else if(phaseTo - phaseFrom < -M_PI) {
        phaseTo += 2*M_PI;
    }
    auto magInterp = magFrom * (1.0 - alpha) + magTo * alpha;
    auto phaseInterp = phaseFrom * (1.0 - alpha) + phaseTo * alpha;

    return std::polar<double>(magInterp, phaseInterp);
}
