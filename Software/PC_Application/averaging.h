#ifndef AVERAGING_H
#define AVERAGING_H

#include "Device/device.h"

#include <array>
#include <deque>
#include <complex>

class Averaging
{
public:
    enum class Mode {
        Mean,
        Median
    };

    Averaging();
    void reset(unsigned int points);
    void setAverages(unsigned int a);
    Protocol::Datapoint process(Protocol::Datapoint d);
    Protocol::SpectrumAnalyzerResult process(Protocol::SpectrumAnalyzerResult d);
    // Returns the number of averaged sweeps. Value is incremented whenever the last point of the sweep is added.
    // Returned values are in range 0 to averages
    unsigned int getLevel();
    // Returns the number of the currently active sweep. Value is incremented whenever the the first point of the sweep is added
    // Returned values are in range 0 (when no data has been added yet) to averages
    unsigned int currentSweep();
    Mode getMode() const;
    void setMode(const Mode &value);

private:
    std::vector<std::deque<std::array<std::complex<double>, 4>>> avg;
    int maxPoints;
    unsigned int averages;
    Mode mode;
};

#endif // AVERAGING_H
