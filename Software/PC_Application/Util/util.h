#ifndef UTILH_H
#define UTILH_H

#include <complex>
#include <math.h>
#include <limits>

namespace Util {
    template<typename T> T Scale(T value, T from_low, T from_high, T to_low, T to_high) {
        value -= from_low;
        value *= (to_high - to_low) / (from_high - from_low);
        value += to_low;
        return value;
    }

    static inline double SparamTodB(double d) {
        return 20*log10(d);
    }
    static inline double SparamTodB(std::complex<double> d) {
        return SparamTodB(abs(d));
    }
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
    static inline std::complex<double> SparamToImpedance(std::complex<double> d) {
        return 50.0 * (1.0 + d) / (1.0 - d);
    }
    // all these conversions assume series connection of real and imag part
    static inline double SparamToResistance(std::complex<double> d) {
        return SparamToImpedance(d).real();
    }
    static inline double SparamToCapacitance(std::complex<double> d, double freq) {
        return -1.0 / (SparamToImpedance(d).imag() * 2.0 * M_PI * freq);
    }
    static inline double SparamToInductance(std::complex<double> d, double freq) {
        return SparamToImpedance(d).imag() / (2.0 * M_PI * freq);
    }
    static inline double SparamToQualityFactor(std::complex<double> d) {
        return abs(d.imag()) / d.real();
    }
}

#endif // UTILH_H
