#ifndef AVERAGING_H
#define AVERAGING_H

#include "Device/devicedriver.h"

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
    DeviceDriver::VNAMeasurement process(DeviceDriver::VNAMeasurement d);
    DeviceDriver::SAMeasurement process(DeviceDriver::SAMeasurement d);
    // Returns the number of averaged sweeps. Value is incremented whenever the last point of the sweep is added.
    // Returned values are in range 0 to averages
    unsigned int getLevel();
    // Returns the number of the currently active sweep. Value is incremented whenever the the first point of the sweep is added
    // Returned values are in range 0 (when no data has been added yet) to averages
    unsigned int currentSweep();
    // Returns true if all required averages have been taken
    bool settled();
    Mode getMode() const;
    void setMode(const Mode &value);

private:
    void process(unsigned int pointNum, std::vector<std::complex<double> > &data);

    std::vector<std::deque<std::vector<std::complex<double>>>> avg;
    unsigned int numMeasurements;
    unsigned int averages;
    Mode mode;
};

#endif // AVERAGING_H
