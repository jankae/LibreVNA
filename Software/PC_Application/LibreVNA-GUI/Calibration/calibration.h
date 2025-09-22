#ifndef CALIBRATION2_H
#define CALIBRATION2_H

#include "savable.h"
#include "calibrationmeasurement.h"
#include "calkit.h"
#include "Traces/trace.h"
#include "scpi.h"

#include <mutex>

class Calibration : public QObject, public Savable, public SCPINode
{
    Q_OBJECT

    friend class LibreCALDialog;
    friend class CalibrationTests;
public:
    Calibration();

    enum class Type {
        None,
        OSL,
        SOLT,
        SOLTwithoutRxMatch,
        ThroughNormalization,
        TRL,
        Last,
    };
    class CalType {
    public:
        Type type;
        std::vector<unsigned int> usedPorts; // port count starts at 1
        QString getReadableDescription();
        QString getShortString();

        static CalType fromShortString(QString s);

        friend bool operator==(const CalType &lhs, const CalType &rhs);
    };

    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);

    // Applies calculated calibration coefficients to measurement data
    void correctMeasurement(DeviceDriver::VNAMeasurement &d);
    void correctTraces(std::map<QString, Trace*> traceSet);

    // Starts the calibration edit dialog, allowing the user to make/delete measurements
    void edit(TraceModel *traceModel = nullptr);

    Calkit& getKit();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    bool toFile(QString filename = QString());
    bool fromFile(QString filename = QString());

    // Returns all possible calibration types/port permutations for the currently connected device.
    // If no device is connected, a two-port device is assumed
    static std::vector<CalType> getAvailableCalibrations();

    // Returns vector of all calibration types (without 'Last')
    static std::vector<Type> getTypes();
    // Checks whether all measurements for a specific calibration are available.
    // If pointer to the frequency/points variables are given, the start/stop frequency and number of points the calibration will have after the calculation is stored there
    bool canCompute(CalType type, double *startFreq = nullptr, double *stopFreq = nullptr, int *points = nullptr, bool *isLog = nullptr);
    // Resets the calibration (deletes all measurements and calculated coefficients)
    void reset();
    // Returns the minimum number of ports for a given calibration type.
    // E.g. the SOL(T) calibration can work with only one port, a through normalization requires at least two
    static int minimumPorts(Type type);

    // Adds a new measurement point (data) to all calibration measurements (m)
    void addMeasurements(std::set<CalibrationMeasurement::Base*> m, const DeviceDriver::VNAMeasurement &data);
    // Deletes all datapoints in the calibration measurements (m)
    void clearMeasurements(std::set<CalibrationMeasurement::Base*> m);
    CalType getCaltype() const;

    enum class InterpolationType {
        Unchanged, // Nothing has changed, settings and calibration points match
        Exact, // Every frequency point in settings has an exact calibration point (but there are more calibration points outside of the sweep)
        Interpolate, // Every point in the sweep can be interpolated between two calibration points
        Extrapolate, // At least one point in sweep is outside of the calibration and has to be extrapolated
        NoCalibration, // No calibration available
    };

    InterpolationType getInterpolation(double f_start, double f_stop, int npoints);

    std::vector<Trace*> getErrorTermTraces();
    std::vector<Trace*> getMeasurementTraces();

    QString getCurrentCalibrationFile();
    double getMinFreq();
    double getMaxFreq();

    int getNumPoints();
    bool hasUnsavedChanges() const;

    QString getValidDevice() const;
    bool validForDevice(QString serial) const;

    // query whether error terms coefficients are available. Port count starts at 1
    bool hasDirectivity(unsigned int port);
    bool hasReflectionTracking(unsigned int port);
    bool hasSourceMatch(unsigned int port);
    bool hasReceiverMatch(unsigned int sourcePort, unsigned int receivePort);
    bool hasTransmissionTracking(unsigned int sourcePort, unsigned int receivePort);
    bool hasIsolation(unsigned int sourcePort, unsigned int receivePort);

public slots:
    // Call once all datapoints of the current span have been added
    void measurementsComplete();
    // Call once when a measurement is aborted before all points have been captured
    void measurementsAbort();
    // Attempts to calculate the calibration coefficients. If not enough measurements are available, false is returned and the currently used coefficients are not changed
    bool compute(CalType type);
    // Deactivates the calibration, resets the calibration coefficients. Calibration measurements are NOT deleted.
    void deactivate();
signals:
    // emitted when the measurement of a set of calibration measurements should be started
    void startMeasurements(std::set<CalibrationMeasurement::Base*> m);
    // emitted whenever a measurement is complete (triggered by calling measurementsComplete())
    void measurementsUpdated();
    // emitted when taking a calibration measurement is aborted
    void measurementsAborted();
    // emitted when calibration coefficients were calculated/updated successfully
    void activated(CalType type);
    // emitted when the calibrationo coefficients were reset
    void deactivated();
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
    void deleteMeasurements();

    static bool hasFrequencyOverlap(std::vector<CalibrationMeasurement::Base*> m, double *startFreq = nullptr, double *stopFreq = nullptr, int *points = nullptr, bool *isLog = nullptr);
    // returns all measurements that match the paramaters
    std::vector<CalibrationMeasurement::Base*> findMeasurements(CalibrationMeasurement::Base::Type type, int port1 = 0, int port2 = 0);
    // returns the first measurement in the list that matches the parameters
    CalibrationMeasurement::Base* findMeasurement(CalibrationMeasurement::Base::Type type, int port1 = 0, int port2 = 0);
    CalibrationMeasurement::Base *newMeasurement(CalibrationMeasurement::Base::Type type);

    class Point {
    public:
        double frequency;
        std::vector<std::complex<double>> D; // Directivity
        std::vector<std::complex<double>> R; // Reflection tracking
        std::vector<std::complex<double>> S; // Source Match
        std::vector<std::vector<std::complex<double>>> L; // Receiver Match
        std::vector<std::vector<std::complex<double>>> T; // Transmission tracking
        std::vector<std::vector<std::complex<double>>> I; // Transmission isolation
        Point interpolate(const Point &to, double alpha);
    };
    std::vector<Point> points;

    Point createInitializedPoint(double f);
    Point computeOSL(double f);
    Point computeSOLT(double f);
    Point computeSOLTwithoutRxMatch(double f);
    Point computeThroughNormalization(double f);
    Point computeTRL(double f);

    std::vector<CalibrationMeasurement::Base*> measurements;

    Calkit kit;
    CalType caltype;

    QString descriptiveCalName();
    QString currentCalFile;

    QString validDevice;

    bool unsavedChanges;

    std::recursive_mutex access;
};

#endif // CALIBRATION2_H
