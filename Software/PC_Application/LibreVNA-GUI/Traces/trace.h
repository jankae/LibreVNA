#ifndef TRACE_H
#define TRACE_H

#include "touchstone.h"
#include "csv.h"
#include "Device/devicedriver.h"
#include "Math/tracemath.h"
#include "Tools/parameters.h"

#include <QObject>
#include <complex>
#include <map>
#include <QColor>
#include <set>
#include <QTime>
#include <QTimer>

class Marker;
class TraceModel;

class Trace : public TraceMath
{
    Q_OBJECT
    friend class TraceEditDialog;
public:

    using Data = TraceMath::Data;

    enum class Source {
        Live,
        File,
        Math,
        Calibration,
        Last,
    };

    Trace(QString name = QString(), QColor color = Qt::darkYellow, QString live = "S11");
    ~Trace();

    enum class LivedataType {
        Overwrite,
        MaxHold,
        MinHold,
        Invalid,
    };

    void clear(bool force = false);
    void addData(const Data& d, DataType domain, double reference_impedance = 50.0, int index = -1);
    void addData(const Data& d, const DeviceDriver::SASettings &s, int index = -1);
    void addDeembeddingData(const Data& d, double reference_impedance = 50.0, int index = -1);
    void setName(QString name);
    void setVelocityFactor(double v);
    void fillFromTouchstone(Touchstone &t, unsigned int parameter);
    QString fillFromCSV(CSV &csv, unsigned int parameter); // returns the suggested trace name (not yet set in member data)
    static void fillFromDatapoints(std::map<QString, Trace*> traceSet, const std::vector<DeviceDriver::VNAMeasurement> &data, bool deembedded = false);
    void fromLivedata(LivedataType type, QString param);
    void fromMath();
    QString name() { return _name; }
    QColor color() { return _color; }
    bool isVisible();
    bool canBePaused();
    void pause();
    void resume();
    bool isPaused();
    Source getSource() {return source;}
    bool isReflection();
    QString liveParameter() { return liveParam; }
    LivedataType liveType() { return _liveType; }
    TraceMath::DataType outputType() const { return lastMath->getDataType(); }
    TraceMath *getLastMath() { return lastMath;}
    unsigned int size() const;

    bool isDeembeddingActive();
    bool deembeddingAvailable();
    void setDeembeddingActive(bool active);
    void clearDeembedding();

    double minX() override;
    double maxX() override;
    double findExtremum(bool max, double xmin = std::numeric_limits<double>::lowest(), double xmax = std::numeric_limits<double>::max());
    /* Searches for peaks in the trace data and returns the peak frequencies in ascending order.
     * Up to maxPeaks will be returned, with higher level peaks taking priority over lower level peaks.
     * Only peaks with at least minLevel will be considered.
     * To detect the next peak, the signal first has to drop at least minValley below the peak level.
     */
    std::vector<double> findPeakFrequencies(unsigned int maxPeaks = 100, double minLevel = -100.0, double minValley = 3.0,
                                            double xmin = std::numeric_limits<double>::lowest(), double xmax = std::numeric_limits<double>::max(), bool negativePeaks = false);
    enum class SampleType {
        Frequency,
        TimeImpulse,
        TimeStep,
    };

    Data sample(unsigned int index, bool getStepResponse = false) const;

    virtual Data getSample(unsigned int index) override;
    virtual Data getInterpolatedSample(double x) override;
    virtual unsigned int numSamples() override;
    virtual std::vector<Data> getData() override;

    double getUnwrappedPhase(unsigned int index);
    // returns a (possibly interpolated sample) at a specified frequency/time/power
    Data interpolatedSample(double x);
    QString getFilename() const;
    unsigned int getFileParameter() const;
    /* Returns the noise in dbm/Hz for spectrum analyzer measurements. May return NaN if calculation not possible */
    double getNoise(double frequency);
    double getGroupDelay(double frequency);
    int index(double x);
    std::set<Marker *> getMarkers() const;
    void setCalibration();
    void setReflection(bool value);

    DataType outputType(DataType inputType) override;
    QString description() override;

    bool mathEnabled(); // check if math operations are enabled
    bool hasMathOperations(); // check if math operations are set up (not necessarily enabled)
    void enableMath(bool enable);
    // Adds a new math operation at the end of the list and enables it
    void addMathOperation(TraceMath *math);
    void addMathOperations(std::vector<TraceMath*> maths);
    // removes the math operation at the given index. Index 0 is invalid as this would be the trace itself
    void removeMathOperation(unsigned int index);
    // swaps the order of math operations at index and index+1. Does nothing if either index is invalid
    void swapMathOrder(unsigned int index);
    void enableMathOperation(unsigned int index, bool enable);
    class MathInfo {
    public:
        TraceMath *math;
        bool enabled;
    };
    const std::vector<MathInfo>& getMathOperations() const;

    static bool isSAParameter(QString param);
    static bool isVNAParameter(QString param);

    double velocityFactor();
    double timeToDistance(double time);
    double distanceToTime(double distance);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    Type getType() override {return Type::Last;} // can return invalid type, this will never be called

    // Traces are referenced by pointers throughout this project (e.g. when added to a graph)
    // When saving the current graph configuration, the pointer is not useful. Instead a trace
    // hash is saved to identify the correct trace. The hash should be influenced by every setting
    // the trace can have (and its math function). It should not depend on the acquired trace samples
    unsigned int toHash(bool forceUpdate = false);

    static std::vector<Trace*> createFromTouchstone(Touchstone &t);
    static std::vector<Trace*> createFromCSV(CSV &csv);

    // Assembles datapoints as received from the VNA from four S parameter traces. Requires that all traces are in the frequency domain,
    // have the same number of samples and their samples must be at the same frequencies across all traces
    static std::vector<DeviceDriver::VNAMeasurement> assembleDatapoints(std::map<QString, Trace *> traceSet);

    static LivedataType TypeFromString(QString s);
    static QString TypeToString(LivedataType t);

    double getReferenceImpedance() const;

    void setModel(TraceModel *newModel);
    TraceModel *getModel() const;

    const QString &getMathFormula() const;
    void setMathFormula(const QString &newMathFormula);
    QString getMathFormulaError() const;

    // When loading setups, some traces may be used as a math source before they are loaded.
    // If that happens, their hashes are added to a list. Call this function for every new trace
    // after all traces from the setup file have been created. It will look for the missing traces
    bool resolveMathSourceHashes();

    void setReferenceImpedance(double value);

public slots:
    void setVisible(bool visible);
    void setColor(QColor color);
    void addMarker(Marker *m);
    void removeMarker(Marker *m);

    // Functions for handling source == Source::Math

    // Checks whether the trace data depends on trace t.
    // If onlyDirectDependency is true, only the direct math sources are checked (i.e. the math sources of this trace).
    // If onlyDirectDependency is false, math sources and all their sources are checked recursively
    bool mathDependsOn(Trace *t, bool onlyDirectDependency = false);
    // Checks whether a trace can potentially be used as a math source. It can not be used if:
    // - it is the trace itself
    // - it is in a different domain than an already used math source
    // - it depends on this trace
    bool canAddAsMathSource(Trace *t);
    // Adds another trace as a math source, with a custom variable name. The same trace may be added multiple times with
    // different names, older variable names will be replaced with the new one
    bool addMathSource(Trace *t, QString variableName);
    // Removes a trace as a math source (if its variable name is still used in the mathFormula, this will break the calculation)
    void removeMathSource(Trace *t);
    // Retrieves the variable name used for the specified trace. If the trace is not used as a math source, an emptry string is returned
    QString getSourceVariableName(Trace *t);

signals:
    void cleared(Trace *t);
    void typeChanged(Trace *t);
    void deleted(Trace *t);
    void visibilityChanged(Trace *t);
    void dataChanged(unsigned int begin, unsigned int end);
    void nameChanged();
    void pauseChanged();
    void deembeddingChanged(Trace *t);
    void colorChanged(Trace *t);
    void markerAdded(Marker *m);
    void markerRemoved(Marker *m);
    void markerFormatChanged(Marker *m);
    void lastMathChanged();

private slots:
    void markerVisibilityChanged(Marker *m);

    // Functions for handling source == Source::Math

    // Updates the datapoints, based on the available span of all math sources.
    // The least common span is used for this trace. This function only creates the datapoints
    // and sets the X coordinate. The Y coordinates are set in calculateMath()
    void updateMathTracePoints();
    // Keeps track of deleted traces and removes them from the math sources
    void mathSourceTraceDeleted(Trace *t);
    // Schedules an update of this trace, should be called whenever any source data changes.
    // As it is likely that multiple source traces will change directly after each other, no
    // calculation is performed directly. Instead the calculation is only scheduled and executed
    // shortly after. This prevents calculating data that will be overwritten immediately afterwards
    void scheduleMathCalculation(unsigned int begin, unsigned int end);
    // Actually calculates the Y coordinate values of this trace. It expects that the data vector is
    // already set up with the required amount of points and X coordinates
    void calculateMath();
    // Removes all math sources, use this when switching to a different source mode
    void clearMathSources();
    // Attempts to add a math source using the trace hash instead of a pointer (when loading setups)
    bool addMathSource(unsigned int hash, QString variableName);

private:
    TraceModel *model; // model which this trace will be part of
    QString _name;
    QColor _color;
    Source source;

    // Hash for identifying the trace when loading/saving setup files.
    // When writing setup files, the hash is created from the JSON representation of the trace.
    // When loading setup files, the hash specified in the setup file is used (prevents different
    // hashes when the JSON format changes in newer versions)
    unsigned int hash;
    bool hashSet;
    bool JSONskipHash;

    // Members for when source == Source::Live
    LivedataType _liveType;
    QString liveParam;

    // Members for when source == Source::File
    QString filename;
    unsigned int fileParameter;

    // Members for when source == Source::Math
    std::map<Trace*,QString> mathSourceTraces;
    std::map<unsigned int,QString> mathSourceUnresolvedHashes;
    QString mathFormula;
    static constexpr int MinMathUpdateInterval = 100;
    QTime lastMathUpdate;
    QTimer mathCalcTimer;
    unsigned int mathUpdateBegin;
    unsigned int mathUpdateEnd;

    double vFactor;
    bool reflection;
    bool visible;
    bool paused;
    double reference_impedance;
    DataType domain;

    std::set<Marker*> markers;
    struct {
        union {
            DeviceDriver::SASettings SA;
        };
        bool valid;
    } settings;

    // de-embedding variables
    std::vector<Data> deembeddingData;
    bool deembeddingActive;
    double deembedded_reference_impedance;

    std::vector<MathInfo> mathOps;
    TraceMath *lastMath;
    std::vector<double> unwrappedPhase;
    void updateLastMath(std::vector<MathInfo>::reverse_iterator start);
};

#endif // TRACE_H
