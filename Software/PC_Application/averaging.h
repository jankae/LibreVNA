#ifndef AVERAGING_H
#define AVERAGING_H


#include "Device/device.h"
#include <deque>
#include <complex>

class Averaging
{
public:
    Averaging();
    void reset();
    void setAverages(unsigned int a);
    Protocol::Datapoint process(Protocol::Datapoint d);
    Protocol::SpectrumAnalyzerResult process(Protocol::SpectrumAnalyzerResult d);
    unsigned int getLevel();
private:
    std::vector<std::deque<std::array<std::complex<double>, 4>>> avg;
    int maxPoints;
    unsigned int averages;
};

#endif // AVERAGING_H
