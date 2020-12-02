#ifndef TRACE_H
#define TRACE_H

#include <QObject>
#include <complex>
#include <map>
#include <QColor>
#include <set>
#include "touchstone.h"
#include "Device/device.h"
#include "Math/tracemath.h"

class TraceMarker;

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
    };

    Trace(QString name = QString(), QColor color = Qt::darkYellow, LiveParameter live = LiveParameter::S11);
    ~Trace();

    enum class LivedataType {
        Overwrite,
        MaxHold,
        MinHold,
    };


    void clear();
    void addData(const Data& d);
    void addData(const Data& d, const Protocol::SweepSettings& s);
    void addData(const Data& d, const Protocol::SpectrumAnalyzerSettings& s);
    void setName(QString name);
    void fillFromTouchstone(Touchstone &t, unsigned int parameter, QString filename = QString());
    void fromLivedata(LivedataType type, LiveParameter param);
    QString name() { return _name; };
    QColor color() { return _color; };
    bool isVisible();
    void pause();
    void resume();
    bool isPaused();
    bool isTouchstone();
    bool isCalibration();
    bool isLive();
    bool isReflection();
    LiveParameter liveParameter() { return _liveParam; }
    LivedataType liveType() { return _liveType; }
    TraceMath::DataType outputType() { return lastMath->getDataType(); };
    unsigned int size();
    double minX();
    double maxX();
    double findExtremumFreq(bool max);
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

    Data sample(unsigned int index, SampleType type = SampleType::Frequency);
    QString getTouchstoneFilename() const;
    unsigned int getTouchstoneParameter() const;
    /* Returns the noise in dbm/Hz for spectrum analyzer measurements. May return NaN if calculation not possible */
    double getNoise(double frequency);
    int index(double x);
    std::set<TraceMarker *> getMarkers() const;
    void setCalibration(bool value);
    void setReflection(bool value);

    DataType outputType(DataType inputType) override { Q_UNUSED(inputType) return DataType::Frequency;};
    QString description() override;

    bool mathEnabled(); // check if math operations are enabled
    bool hasMathOperations(); // check if math operations are set up (not necessarily enabled)
    void enableMath(bool enable);
    // Adds a new math operation at the end of the list and enables it
    void addMathOperation(TraceMath *mathOps);
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

public slots:
    void setTouchstoneParameter(int value);
    void setTouchstoneFilename(const QString &value);
    void setVisible(bool visible);
    void setColor(QColor color);
    void addMarker(TraceMarker *m);
    void removeMarker(TraceMarker *m);

signals:
    void cleared(Trace *t);
    void typeChanged(Trace *t);
    void deleted(Trace *t);
    void visibilityChanged(Trace *t);
    void dataChanged();
    void nameChanged();
    void colorChanged(Trace *t);
    void markerAdded(TraceMarker *m);
    void markerRemoved(TraceMarker *m);

private:
    QString _name;
    QColor _color;
    LivedataType _liveType;
    LiveParameter _liveParam;
    bool reflection;
    bool visible;
    bool paused;
    bool touchstone;
    bool calibration;
    QString touchstoneFilename;
    unsigned int touchstoneParameter;
    std::set<TraceMarker*> markers;
    struct {
        union {
            Protocol::SweepSettings VNA;
            Protocol::SpectrumAnalyzerSettings SA;
        };
        bool valid;
    } settings;

    std::vector<MathInfo> mathOps;
    TraceMath *lastMath;
    void updateLastMath(std::vector<MathInfo>::reverse_iterator start);
};

#endif // TRACE_H
