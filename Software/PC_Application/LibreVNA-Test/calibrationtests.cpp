#include "calibrationtests.h"

#include "calibration.h"

CalibrationTests::CalibrationTests() {}

void CalibrationTests::LinearDetection()
{
    // create some measurements
    std::vector<CalibrationMeasurement::Base*> m;
    double startFreq = 100000;
    double stopFreq = 6000000000;
    int points = 1001;
    Calibration cal;
    cal.getKit().setIdealDefault();
    auto open = new CalibrationMeasurement::Open(&cal);
    open->setPort(1);
    m.push_back(open);
    auto _short = new CalibrationMeasurement::Short(&cal);
    _short->setPort(1);
    m.push_back(_short);
    auto load = new CalibrationMeasurement::Load(&cal);
    load->setPort(1);
    m.push_back(load);

    for(int i=0;i<points;i++) {
        double f = startFreq + (stopFreq - startFreq) * i / (points-1);
        DeviceDriver::VNAMeasurement meas;
        meas.frequency = f;
        meas.measurements["S11"] = 0.0;
        m[0]->addPoint(meas);
        m[1]->addPoint(meas);
        m[2]->addPoint(meas);
    }

    // verify correct detection
    double detectedStart;
    double detectedStop;
    int detectedPoints;
    bool detectedLog;
    Calibration::hasFrequencyOverlap(m, &detectedStart, &detectedStop, &detectedPoints, &detectedLog);

    QVERIFY(qFuzzyCompare(detectedStart, startFreq));
    QVERIFY(qFuzzyCompare(detectedStop, stopFreq));
    QVERIFY(detectedPoints == points);
    QVERIFY(detectedLog == false);
}

void CalibrationTests::LogDetection()
{
    // create some measurements
    std::vector<CalibrationMeasurement::Base*> m;
    double startFreq = 100000;
    double stopFreq = 6000000000;
    int points = 1001;
    Calibration cal;
    cal.getKit().setIdealDefault();
    auto open = new CalibrationMeasurement::Open(&cal);
    open->setPort(1);
    m.push_back(open);
    auto _short = new CalibrationMeasurement::Short(&cal);
    _short->setPort(1);
    m.push_back(_short);
    auto load = new CalibrationMeasurement::Load(&cal);
    load->setPort(1);
    m.push_back(load);

    for(int i=0;i<points;i++) {
        double f = startFreq * pow(10.0, i * log10(stopFreq / startFreq) / (points - 1));
        DeviceDriver::VNAMeasurement meas;
        meas.frequency = f;
        meas.measurements["S11"] = 0.0;
        m[0]->addPoint(meas);
        m[1]->addPoint(meas);
        m[2]->addPoint(meas);
    }

    // verify correct detection
    double detectedStart;
    double detectedStop;
    int detectedPoints;
    bool detectedLog;
    Calibration::hasFrequencyOverlap(m, &detectedStart, &detectedStop, &detectedPoints, &detectedLog);

    QVERIFY(qFuzzyCompare(detectedStart, startFreq));
    QVERIFY(qFuzzyCompare(detectedStop, stopFreq));
    QVERIFY(detectedPoints == points);
    QVERIFY(detectedLog == true);
}

void CalibrationTests::MixedDetection()
{
    // create some measurements
    std::vector<CalibrationMeasurement::Base*> m;
    double startFreq = 100000;
    double stopFreq = 6000000000;
    int points = 1001;
    Calibration cal;
    cal.getKit().setIdealDefault();
    auto open = new CalibrationMeasurement::Open(&cal);
    open->setPort(1);
    m.push_back(open);
    auto _short = new CalibrationMeasurement::Short(&cal);
    _short->setPort(1);
    m.push_back(_short);
    auto load = new CalibrationMeasurement::Load(&cal);
    load->setPort(1);
    m.push_back(load);

    for(int i=0;i<points;i++) {
        // one linear measurement, two log measurement
        double flin = startFreq + (stopFreq - startFreq) * i / (points-1);
        double flog = startFreq * pow(10.0, i * log10(stopFreq / startFreq) / (points - 1));
        DeviceDriver::VNAMeasurement measlin;
        measlin.frequency = flin;
        measlin.measurements["S11"] = 0.0;
        DeviceDriver::VNAMeasurement measlog;
        measlog.frequency = flog;
        measlog.measurements["S11"] = 0.0;
        m[0]->addPoint(measlin);
        m[1]->addPoint(measlog);
        m[2]->addPoint(measlog);
    }

    // verify correct detection
    double detectedStart;
    double detectedStop;
    int detectedPoints;
    bool detectedLog;
    Calibration::hasFrequencyOverlap(m, &detectedStart, &detectedStop, &detectedPoints, &detectedLog);

    QVERIFY(qFuzzyCompare(detectedStart, startFreq));
    QVERIFY(qFuzzyCompare(detectedStop, stopFreq));
    QVERIFY(detectedPoints == points);
    QVERIFY(detectedLog == true);
}
