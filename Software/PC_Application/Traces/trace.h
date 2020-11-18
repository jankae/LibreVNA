#ifndef TRACE_H
#define TRACE_H

#include <QObject>
#include <complex>
#include <map>
#include <QColor>
#include <set>
#include "touchstone.h"
#include "Device/device.h"

class TraceMarker;

class Trace : public QObject
{
    Q_OBJECT
public:

    class Data {
    public:
        double frequency;
        std::complex<double> S;
    };

    class TimedomainData {
    public:
        double time;
        double distance;
        double impulseResponse;
        double stepResponse;
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
    unsigned int size() { return _data.size(); }
    double minFreq() { return size() > 0 ? _data.front().frequency : 0.0; };
    double maxFreq() { return size() > 0 ? _data.back().frequency : 0.0; };
    double findExtremumFreq(bool max);
    /* Searches for peaks in the trace data and returns the peak frequencies in ascending order.
     * Up to maxPeaks will be returned, with higher level peaks taking priority over lower level peaks.
     * Only peaks with at least minLevel will be considered.
     * To detect the next peak, the signal first has to drop at least minValley below the peak level.
     */
    std::vector<double> findPeakFrequencies(unsigned int maxPeaks = 100, double minLevel = -100.0, double minValley = 3.0);
    Data sample(unsigned int index) { return _data.at(index); }
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
    const std::vector<TimedomainData>& getTDR() { return timeDomain;}

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
    void dataAdded(Trace *t, Data d);
    void deleted(Trace *t);
    void visibilityChanged(Trace *t);
    void dataChanged();
    void nameChanged();
    void colorChanged(Trace *t);
    void markerAdded(TraceMarker *m);
    void markerRemoved(TraceMarker *m);

private:
    void updateTimeDomainData();
    void printTimeDomain();
    std::vector<Data> _data;
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
};

#endif // TRACE_H
