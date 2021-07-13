#ifndef TRACE_H
#define TRACE_H

#include <QObject>
#include <complex>
#include <map>
#include <QColor>
#include <set>
#include "touchstone.h"
#include "csv.h"
#include "Device/device.h"
#include "Math/tracemath.h"

class Marker;

class Trace : public TraceMath
{
    Q_OBJECT
public:

    using Data = TraceMath::Data;

    enum class LiveParameter {
        S11,
        S12,
        S21,
        S22,
        Port1,
        Port2,
        Invalid,
    };

    Trace(QString name = QString(), QColor color = Qt::darkYellow, LiveParameter live = LiveParameter::S11);
    ~Trace();

    enum class LivedataType {
        Overwrite,
        MaxHold,
        MinHold,
        Invalid,
    };


    void clear();
    void addData(const Data& d, DataType domain);
    void addData(const Data& d, const Protocol::SpectrumAnalyzerSettings& s);
    void setName(QString name);
    void setVelocityFactor(double v);
    void fillFromTouchstone(Touchstone &t, unsigned int parameter);
    QString fillFromCSV(CSV &csv, unsigned int parameter); // returns the suggested trace name (not yet set in member data)
    static void fillFromDatapoints(Trace &S11, Trace &S12, Trace &S21, Trace &S22, const std::vector<Protocol::Datapoint> &data);
    void fromLivedata(LivedataType type, LiveParameter param);
    QString name() { return _name; };
    QColor color() { return _color; };
    bool isVisible();
    void pause();
    void resume();
    bool isPaused();
    bool isFromFile();
    bool isCalibration();
    bool isLive();
    bool isReflection();
    LiveParameter liveParameter() { return _liveParam; }
    LivedataType liveType() { return _liveType; }
    TraceMath::DataType outputType() const { return lastMath->getDataType(); };
    unsigned int size() const;
    double minX();
    double maxX();
    double findExtremum(bool max);
    /* Searches for peaks in the trace data and returns the peak frequencies in ascending order.
     * Up to maxPeaks will be returned, with higher level peaks taking priority over lower level peaks.
     * Only peaks with at least minLevel will be considered.
     * To detect the next peak, the signal first has to drop at least minValley below the peak level.
     */
    std::vector<double> findPeakFrequencies(unsigned int maxPeaks = 100, double minLevel = -100.0, double minValley = 3.0);
    enum class SampleType {
        Frequency,
        TimeImpulse,
        TimeStep,
    };

    Data sample(unsigned int index, bool getStepResponse = false) const;
    // returns a (possibly interpolated sample) at a specified frequency/time/power
    Data interpolatedSample(double x);
    QString getFilename() const;
    unsigned int getFileParameter() const;
    /* Returns the noise in dbm/Hz for spectrum analyzer measurements. May return NaN if calculation not possible */
    double getNoise(double frequency);
    int index(double x);
    std::set<Marker *> getMarkers() const;
    void setCalibration(bool value);
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

    double velocityFactor();
    double timeToDistance(double time);
    double distanceToTime(double distance);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    Type getType() override {return Type::Last;}; // can return invalid type, this will never be called

    // Traces are referenced by pointers throughout this project (e.g. when added to a graph)
    // When saving the current graph configuration, the pointer is not useful. Instead a trace
    // hash is saved to identify the correct trace. The hash should be influenced by every setting
    // the trace can have (and its math function). It should not depend on the acquired trace samples
    unsigned int toHash();

    static std::vector<Trace*> createFromTouchstone(Touchstone &t);
    static std::vector<Trace*> createFromCSV(CSV &csv);

    // Assembles datapoints as received from the VNA from four S parameter traces. Requires that all traces are in the frequency domain,
    // have the same number of samples and their samples must be at the same frequencies across all traces
    static std::vector<Protocol::Datapoint> assembleDatapoints(const Trace &S11, const Trace &S12, const Trace &S21, const Trace &S22);

    static LiveParameter ParameterFromString(QString s);
    static QString ParameterToString(LiveParameter p);
    static bool isVNAParameter(LiveParameter p);
    static bool isSAParamater(LiveParameter p);

    static LivedataType TypeFromString(QString s);
    static QString TypeToString(LivedataType t);

public slots:
    void setVisible(bool visible);
    void setColor(QColor color);
    void addMarker(Marker *m);
    void removeMarker(Marker *m);

signals:
    void cleared(Trace *t);
    void typeChanged(Trace *t);
    void deleted(Trace *t);
    void visibilityChanged(Trace *t);
    void dataChanged();
    void nameChanged();
    void pauseChanged();
    void colorChanged(Trace *t);
    void markerAdded(Marker *m);
    void markerRemoved(Marker *m);
    void markerFormatChanged(Marker *m);

private:
    QString _name;
    QColor _color;
    LivedataType _liveType;
    LiveParameter _liveParam;
    double vFactor;
    bool reflection;
    bool visible;
    bool paused;
    bool createdFromFile;
    bool calibration;
    DataType domain;
    QString filename;
    unsigned int fileParameter;
    std::set<Marker*> markers;
    struct {
        union {
            Protocol::SpectrumAnalyzerSettings SA;
        };
        bool valid;
    } settings;

    std::vector<MathInfo> mathOps;
    TraceMath *lastMath;
    void updateLastMath(std::vector<MathInfo>::reverse_iterator start);
};

#endif // TRACE_H
