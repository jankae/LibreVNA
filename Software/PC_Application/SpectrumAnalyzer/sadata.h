#ifndef SADATA_H
#define SADATA_H

#include "Tools/parameters.h"

#include <complex>

class SAData {
public:
    SAData() = default;
    SAData(const Protocol::SpectrumAnalyzerResult &d) {
        port1 = std::complex<double>(d.real_port1, d.imag_port1);
        port2 = std::complex<double>(d.real_port2, d.imag_port2);
        frequency = d.frequency;
        time = (double) d.us / 1000000.0;
        pointNum = d.pointNum;
    }
    double frequency;
    double time;
    std::complex<double> port1, port2;
    unsigned int pointNum;
};

#endif // SADATA_H
