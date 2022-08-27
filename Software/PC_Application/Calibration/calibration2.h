#ifndef CALIBRATION2_H
#define CALIBRATION2_H

#include "savable.h"
#include "calibrationmeasurement.h"
#include "calkit.h"

class Calibration2 : public Savable
{
public:
    Calibration2();

    void edit();

    Calkit& getKit();

private:
    CalibrationMeasurement::Base *newMeasurement(CalibrationMeasurement::Base::Type type);

    std::vector<CalibrationMeasurement::Base*> measurements;

    Calkit kit;
};

#endif // CALIBRATION2_H
