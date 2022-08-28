#ifndef CALIBRATION2_H
#define CALIBRATION2_H

#include "savable.h"
#include "calibrationmeasurement.h"
#include "calkit.h"

class Calibration2 : public QObject, public Savable
{
    Q_OBJECT
public:
    Calibration2();

    enum class Type {
        None,
        SOLT,
        Last,
    };
    class CalType {
    public:
        Type type;
        std::vector<int> usedPorts;
        QString getDescription();
    };

    void correctMeasurement(VirtualDevice::VNAMeasurement &d);

    void edit();

    Calkit& getKit();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    static std::vector<Type> getTypes();
    bool canCompute(CalType type, double *startFreq = nullptr, double *stopFreq = nullptr, int *points = nullptr);
    bool compute(CalType type);
    static int minimumPorts(Type type);

    void deleteMeasurements();
    void addMeasurements(std::set<CalibrationMeasurement::Base*> m, const VirtualDevice::VNAMeasurement &data);
    void clearMeasurements(std::set<CalibrationMeasurement::Base*> m);
public slots:
    void measurementsComplete();
signals:
    void startMeasurements(std::set<CalibrationMeasurement::Base*> m);
    void measurementsUpdated();
private:
    enum class DefaultMeasurements {
        SOL1Port,
        SOLT2Port,
        SOLT3Port,
        SOLT4Port,
        Last
    };
    static QString DefaultMeasurementsToString(DefaultMeasurements dm);
    void createDefaultMeasurements(DefaultMeasurements dm);

    bool hasFrequencyOverlap(std::vector<CalibrationMeasurement::Base*> m, double *startFreq = nullptr, double *stopFreq = nullptr, int *points = nullptr);
    CalibrationMeasurement::Base* findMeasurement(CalibrationMeasurement::Base::Type type, int port1 = 0, int port2 = 0);

    CalibrationMeasurement::Base *newMeasurement(CalibrationMeasurement::Base::Type type);

    class Point {
    public:
        double frequency;
        std::vector<std::complex<double>> D, R, S;
        std::vector<std::vector<std::complex<double>>> L, T;
        Point interpolate(const Point &to, double alpha);
    };
    std::vector<Point> points;

    Point computeSOLT(double f);

    std::vector<CalibrationMeasurement::Base*> measurements;

    Calkit kit;
    CalType caltype;
};

#endif // CALIBRATION2_H
