#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "Device/device.h"
#include "calkit.h"
#include "Traces/tracemodel.h"

#include <complex>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <QDateTime>
#include <savable.h>

class Calibration : public Savable
{
public:
    Calibration();

    enum class Measurement {
        Port1Open,
        Port1Short,
        Port1Load,
        Port2Open,
        Port2Short,
        Port2Load,
        Isolation,
        Through,
        Line,
        Last,
    };

    enum class Standard {
        Open,
        Short,
        Load,
        Through,
        Any,
    };

    static Standard getPort1Standard(Measurement m);
    static Standard getPort2Standard(Measurement m);

    void clearMeasurements();
    void clearMeasurements(std::set<Measurement> types);
    void clearMeasurement(Measurement type);
    void addMeasurement(Measurement type, VNAData &d);
    void addMeasurements(std::set<Measurement> types, VNAData &d);

    enum class Type {
        Port1SOL,
        Port2SOL,
        FullSOLT,
        TransmissionNormalization,
        TRL,
        None,
        Last,
    };


    bool calculationPossible(Type type);
    bool constructErrorTerms(Type type);
    void resetErrorTerms();

    void correctMeasurement(VNAData &d);
    void correctTraces(Trace &S11, Trace &S12, Trace &S21, Trace &S22);

    enum class InterpolationType {
        Unchanged, // Nothing has changed, settings and calibration points match
        Exact, // Every frequency point in settings has an exact calibration point (but there are more calibration points outside of the sweep)
        Interpolate, // Every point in the sweep can be interpolated between two calibration points
        Extrapolate, // At least one point in sweep is outside of the calibration and has to be extrapolated
        NoCalibration, // No calibration available
    };

    InterpolationType getInterpolation(double f_start, double f_stop, int points);

    static Measurement MeasurementFromString(QString s);
    static QString MeasurementToString(Measurement m);
    static Type TypeFromString(QString s);
    static QString TypeToString(Type t);

    class MeasurementInfo {
    public:
        QString name, prerequisites;
        double fmin, fmax;
        unsigned int points;
        QDateTime timestamp;
    };

    static const std::vector<Type> Types();
    const std::vector<Measurement> Measurements(Type type = Type::None, bool optional_included = true);
    MeasurementInfo getMeasurementInfo(Measurement m);

    friend std::istream& operator >> (std::istream &in, Calibration& c);
    int nPoints() {
        return points.size();
    }

    std::vector<Trace*> getErrorTermTraces();
    std::vector<Trace*> getMeasurementTraces();

    bool openFromFile(QString filename = QString());
    bool saveToFile(QString filename = QString());
    Type getType() const;

    Calkit& getCalibrationKit();
    void setCalibrationKit(const Calkit &value);

    enum class PortStandard {
        Male,
        Female,
    };
    void setPortStandard(int port, PortStandard standard);
    PortStandard getPortStandard(int port);
    bool getThroughZeroLength() const;
    void setThroughZeroLength(bool value);

    QString getCurrentCalibrationFile();
    double getMinFreq();
    double getMaxFreq();
    int getNumPoints();

    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;

private:
    void construct12TermPoints();
    void constructPort1SOL();
    void constructPort2SOL();
    void constructTransmissionNormalization();
    void constructTRL();
    bool SanityCheckSamples(const std::vector<Measurement> &requiredMeasurements);
    class Point
    {
    public:
        double frequency;
        // Forward error terms
        std::complex<double> fe00, fe11, fe10e01, fe10e32, fe22, fe30, fex;
        // Reverse error terms
        std::complex<double> re33, re11, re23e32, re23e01, re22, re03, rex;
    };
    Point getCalibrationPoint(VNAData &d);
    /*
     * Constructs directivity, match and tracking correction factors from measurements of three distinct impedances
     * Normally, an open, short and load are used (with ideal reflection coefficients of 1, -1 and 0 respectively).
     * The actual reflection coefficients can be passed on as optional arguments to take into account the non-ideal
     * calibration kit.
     */
    void computeSOL(std::complex<double> s_m,
                    std::complex<double> o_m,
                    std::complex<double> l_m,
                    std::complex<double> &directivity,
                    std::complex<double> &match,
                    std::complex<double> &tracking,
                    std::complex<double> o_c = std::complex<double>(1.0, 0),
                    std::complex<double> s_c = std::complex<double>(-1.0, 0),
                    std::complex<double> l_c = std::complex<double>(0, 0));
    void computeIsolation(std::complex<double> x0_m,
                          std::complex<double> x1_m,
                          std::complex<double> reverse_match,
                          std::complex<double> reverse_tracking,
                          std::complex<double> reverse_directivity,
                          std::complex<double> x0,
                          std::complex<double> x1,
                          std::complex<double> &internal_isolation,
                          std::complex<double> &external_isolation);
    std::complex<double> correctSOL(std::complex<double> measured,
                                    std::complex<double> directivity,
                                    std::complex<double> match,
                                    std::complex<double> tracking);
    class MeasurementData {
    public:
        QDateTime timestamp;
        std::vector<VNAData> datapoints;
    };
    Type type;

    std::map<Measurement, MeasurementData> measurements;
    double minFreq, maxFreq;
    std::vector<Point> points;

    Calkit kit;
    QString descriptiveCalName();
    QString currentCalFile;

    PortStandard port1Standard, port2Standard;
    bool throughZeroLength;
};

#endif // CALIBRATION_H
