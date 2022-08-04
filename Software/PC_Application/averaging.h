#ifndef AVERAGING_H
#define AVERAGING_H

#include "Device/virtualdevice.h"

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
    VirtualDevice::VNAMeasurement process(VirtualDevice::VNAMeasurement d);
    VirtualDevice::SAMeasurement process(VirtualDevice::SAMeasurement d);
    // Returns the number of averaged sweeps. Value is incremented whenever the last point of the sweep is added.
    // Returned values are in range 0 to averages
    unsigned int getLevel();
    // Returns the number of the currently active sweep. Value is incremented whenever the the first point of the sweep is added
    // Returned values are in range 0 (when no data has been added yet) to averages
    unsigned int currentSweep();
    Mode getMode() const;
    void setMode(const Mode &value);

private:
    void process(int pointNum, std::vector<std::complex<double> > &data);

    std::vector<std::deque<std::vector<std::complex<double>>>> avg;
    int maxPoints;
    int numMeasurements;
    unsigned int averages;
    Mode mode;
};

#endif // AVERAGING_H
