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

    class TimedomainData {
    public:
        double time;
        double distance;
        double impulseResponse;
        double stepResponse;
        double impedance;
    };

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
    unsigned int size();
    double minFreq();
    double maxFreq();
    double findExtremumFreq(bool max);
    /* Searches for peaks in the trace data and returns the peak frequencies in ascending order.
     * Up to maxPeaks will be returned, with higher level peaks taking priority over lower level peaks.
     * Only peaks with at least minLevel will be considered.
     * To detect the next peak, the signal first has to drop at least minValley below the peak level.
     */
    std::vector<double> findPeakFrequencies(unsigned int maxPeaks = 100, double minLevel = -100.0, double minValley = 3.0);
    Data sample(unsigned int index);
    QString getTouchstoneFilename() const;
    unsigned int getTouchstoneParameter() const;
    std::complex<double> getData(double frequency);
    /* Returns the noise in dbm/Hz for spectrum analyzer measurements. May return NaN if calculation not possible */
    double getNoise(double frequency);
    int index(double frequency);
    std::set<TraceMarker *> getMarkers() const;
    void setCalibration(bool value);
    void setReflection(bool value);

    // TDR calculation can be ressource intensive, only perform when some other module is interested.
    // Each interested module should call addTDRinterest(), read the data with getTDR() and finally
    // call removeTDRinterest() once TDR updates are no longer required.
    // The data is only updated at the end of a sweep and upon the first addTDRinterest() call.
    void addTDRinterest();
    void removeTDRinterest();
    bool TDRactive() { return tdr_users > 0;};
    const std::vector<TimedomainData>& getTDR() { return timeDomain;}
    // interpolates the TDR data
    // position is assumed to be the delay if it is smaller than the maximum sampled delay, otherwise it is assumed to be the distance.
    // Since the usual delay values are way smaller than the distance values this should work
    TimedomainData getTDR(double position);

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

public slots:
    void setTouchstoneParameter(int value);
    void setTouchstoneFilename(const QString &value);
    void setVisible(bool visible);
    void setColor(QColor color);
    void addMarker(TraceMarker *m);
    void removeMarker(TraceMarker *m);

private:
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
    void changedTDRstate(bool enabled);

private:
    void updateTimeDomainData();
    void printTimeDomain();
//    std::vector<Data> _data;
    std::vector<TimedomainData> timeDomain;
    unsigned int tdr_users;
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
