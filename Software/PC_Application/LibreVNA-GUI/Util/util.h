#ifndef UTILH_H
#define UTILH_H

#include <complex>
#include <math.h>
#include <limits>
#include <vector>

#include <QColor>
#include <QPoint>
#include <QEventLoop>
#include <QTimer>

namespace Util {
    template<typename T> T Scale(T value, T from_low, T from_high, T to_low, T to_high, bool log_from = false, bool log_to = false) {
        T normalized;
        if(log_from) {
            normalized = log10(value / from_low) / log10(from_high / from_low);
        } else {
            normalized = (value - from_low) / (from_high - from_low);
        }
        if(log_to) {
            value = to_low * pow(10.0, normalized * log10(to_high / to_low));
        } else {
            value = normalized * (to_high - to_low) + to_low;
        }
        return value;
    }

    static inline double SparamTodB(double d) {
        return 20*log10(d);
    }
    static inline double SparamTodB(std::complex<double> d) {
        return SparamTodB(abs(d));
    }
    static inline double dBToMagnitude(double dB) {
        return pow(10, dB / 20);
    }
    double dBmTodBuV(double dBm);
    double dBuVTodBm(double dBuV);
    static inline double SparamToDegree(std::complex<double> d) {
        return (arg(d) * 180.0 / M_PI);
    }
    static inline double SparamToVSWR(double d) {
        if(abs(d) < 1.0) {
            return (1+abs(d)) / (1-abs(d));
        } else {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
    static inline double SparamToVSWR(std::complex<double> d) {
        return SparamToVSWR(abs(d));
    }
    std::complex<double> SparamToImpedance(std::complex<double> d, std::complex<double> Z0 = 50.0);
    std::complex<double> ImpedanceToSparam(std::complex<double> Z, std::complex<double> Z0 = 50.0);
    // all these conversions assume series connection of real and imag part
    static inline double SparamToResistance(std::complex<double> d, std::complex<double> Z0 = 50.0) {
        return SparamToImpedance(d, Z0).real();
    }
    static inline double SparamToCapacitance(std::complex<double> d, double freq, std::complex<double> Z0 = 50.0) {
        return -1.0 / (SparamToImpedance(d, Z0).imag() * 2.0 * M_PI * freq);
    }
    static inline double SparamToInductance(std::complex<double> d, double freq, std::complex<double> Z0 = 50.0) {
        return SparamToImpedance(d, Z0).imag() / (2.0 * M_PI * freq);
    }
    static inline double SparamToQualityFactor(std::complex<double> d) {
        auto imp = SparamToImpedance(d);
        return abs(imp.imag()) / imp.real();
    }
    // attempts to return a font color with good contrast against the given background color
    static inline QColor getFontColorFromBackground(QColor q) {
        auto brightness = q.redF() * 0.299 + q.greenF() * 0.587 + q.blueF() * 0.114;
        return brightness > 0.6 ? Qt::black : Qt::white;
    }

    template<typename T> void constrain(T &value, const T &min, const T &max) {
        if(value > max) {
            value = max;
        } else if(value < min) {
            value = min;
        }
    }

    /*
     * Performs interpolation of a list of sorted values.
     * T: type of the elements in the list. Must contain a value by which these elements are sorted in the list.
     * C: Target value of the interpolation (e.g. frequency).
     * extract: A function returning the "sorted by"-value of the element
     *
     * The following operators must be defined:
     * T*double
     * T+T
     *
     * Will return an interpolated element T. If no interpolation is possible because target is outside
     * of the list, either the first or last element in the list is returned instead.
     */
    template<typename T, typename C>
    T interpolate(const std::vector<T> &list, C target, std::function<C(const T &lhs)> extract) {
        T ret = {};
        if(list.size() > 0) {
            auto it = std::lower_bound(list.begin(), list.end(), target, [=](const T &lhs, C rhs)->bool{
                return extract(lhs) < rhs;
            });
            if(it == list.begin()) {
                // just return the first element
                ret = list.front();
            } else if(it == list.end()) {
                // outside of the list of provided values, just use the last one
                ret = list.back();
            } else {
                // needs to interpolate
                auto highPoint = *it;
                auto lowPoint = *std::prev(it);
                double alpha = (target - extract(lowPoint)) / (extract(highPoint) - extract(lowPoint));
                ret = lowPoint * (1.0 - alpha) + highPoint * alpha;
            }
        }
        return ret;
    }

    std::complex<double> interpolateMagPhase(const std::complex<double> &from, const std::complex<double> &to, double alpha);

    void unwrapPhase(std::vector<double> &phase, unsigned int start_index = 0);

    // input values are Y coordinates, assumes evenly spaced linear X values from 0 to input.size() - 1
    void linearRegression(const std::vector<double> &input, double &B_0, double &B_1);

    double distanceToLine(QPointF point, QPointF l1, QPointF l2, QPointF *closestLinePoint = nullptr, double *pointRatio = nullptr);

    unsigned long long random(unsigned long long max);

    std::complex<double> findCenterOfCircle(const std::vector<std::complex<double>> &points);

    std::complex<double> addTransmissionLine(std::complex<double> termination_reflection, double offset_impedance, double offset_delay, double offset_loss, double frequency);

    template<typename T> void solveQuadratic(T a, T b, T c, T &result1, T &result2)
    {
        T root = sqrt(b * b - T(4) * a * c);
        result1 = (-b + root) / (T(2) * a);
        result2 = (-b - root) / (T(2) * a);
    }

    // intensity color scale, input value from 0.0 to 1.0
    QColor getIntensityGradeColor(double intensity);

    bool firmwareEqualOrHigher(QString firmware, QString compare);
}

#endif // UTILH_H
