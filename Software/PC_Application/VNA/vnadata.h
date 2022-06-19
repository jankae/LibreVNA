#ifndef VNADATA_H
#define VNADATA_H

#include "Tools/parameters.h"

#include <complex>

class VNAData {
public:
    VNAData() = default;
    VNAData(const Protocol::Datapoint &d) {
        S = Sparam(std::complex<double>(d.real_S11, d.imag_S11),
                    std::complex<double>(d.real_S12, d.imag_S12),
                    std::complex<double>(d.real_S21, d.imag_S21),
                    std::complex<double>(d.real_S22, d.imag_S22));
        frequency = d.frequency;
        time = (double) d.us / 1000000.0;
        cdbm = d.cdbm;
        pointNum = d.pointNum;
        reference_impedance = 50.0;
    }
    double frequency;
    double time;
    int cdbm;
    Sparam S;
    unsigned int pointNum;
    double reference_impedance;
};


#endif // VNADATA_H
