#ifndef TRACE_H
#define TRACE_H

#include <QObject>
#include <complex>
#include <map>
#include <QColor>
#include <set>
#include "touchstone.h"

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

    Trace(QString name = QString(), QColor color = Qt::darkYellow);
    ~Trace();

    enum class LivedataType {
        Overwrite,
        MaxHold,
        MinHold,
    };
    enum class LiveParameter {
        S11,
        S12,
        S21,
        S22,
    };

    void clear();
    void addData(Data d);
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
    double minFreq() { return _data.front().frequency; };
    double maxFreq() { return _data.back().frequency; };
    Data sample(unsigned int index) { return _data.at(index); }
    QString getTouchstoneFilename() const;
    unsigned int getTouchstoneParameter() const;
    std::complex<double> getData(double frequency);
    int index(double frequency);
    std::set<TraceMarker *> getMarkers() const;
    void setCalibration(bool value);
    void setReflection(bool value);

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
    std::vector<Data> _data;
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
};

#endif // TRACE_H
