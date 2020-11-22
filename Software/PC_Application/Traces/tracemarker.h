#ifndef TRACEMARKER_H
#define TRACEMARKER_H

#include <QPixmap>
#include <QObject>
#include "trace.h"
#include <QComboBox>
#include "CustomWidgets/siunitedit.h"

class TraceMarkerModel;

class TraceMarker : public QObject
{
    Q_OBJECT;
public:
    TraceMarker(TraceMarkerModel *model, int number = 1, TraceMarker *parent = nullptr, QString descr = QString());
    ~TraceMarker();
    void setTimeDomain(bool timeDomain);
    void assignTrace(Trace *t);
    Trace* trace();
    QString readableData();
    QString readableSettings();
    QString readableType();

    double getPosition() const;
    std::complex<double> getData() const;
    Trace::TimedomainData getTimeData() const;
    bool isMovable();

    QPixmap& getSymbol();

    int getNumber() const;
    void setNumber(int value);

    bool editingFrequeny;
    Trace *getTrace() const;


    QWidget *getTypeEditor(QAbstractItemDelegate *delegate = nullptr);
    void updateTypeFromEditor(QWidget *c);
    SIUnitEdit* getSettingsEditor();
    void adjustSettings(double value);

    // Updates marker position and data on automatic markers. Should be called whenever the tracedata is complete
    void update();
    TraceMarker *getParent() const;
    const std::vector<TraceMarker *>& getHelperMarkers() const;
    TraceMarker *helperMarker(unsigned int i);
    QString getSuffix() const;

    bool isTimeDomain() const;

public slots:
    void setPosition(double freq);
signals:
    void deleted(TraceMarker *m);
    void dataChanged(TraceMarker *m);
    void symbolChanged(TraceMarker *m);
    void typeChanged(TraceMarker *m);
    void traceChanged(TraceMarker *m);
    void beginRemoveHelperMarkers(TraceMarker *m);
    void endRemoveHelperMarkers(TraceMarker *m);
    void timeDomainChanged();

private slots:
    void parentTraceDeleted(Trace *t);
    void traceDataChanged();
    void updateSymbol();
signals:
    void rawDataChanged();
private:

    enum class Type {
        Manual,
        Maximum,
        Minimum,
        Delta,
        Noise,
        PeakTable,
        Lowpass,
        Highpass,
        Bandpass,
        TOI,
        PhaseNoise,
    };
    std::set<Type> getSupportedTypes();
    static QString typeToString(Type t) {
        switch(t) {
        case Type::Manual: return "Manual";
        case Type::Maximum: return "Maximum";
        case Type::Minimum: return "Minimum";
        case Type::Delta: return "Delta";
        case Type::Noise: return "Noise";
        case Type::PeakTable: return "Peak Table";
        case Type::Lowpass: return "Lowpass";
        case Type::Highpass: return "Highpass";
        case Type::Bandpass: return "Bandpass";
        case Type::TOI: return "TOI/IP3";
        case Type::PhaseNoise: return "Phase noise";
        default: return QString();
        }
    }
    void constrainPosition();
    void assignDeltaMarker(TraceMarker *m);
    void deleteHelperMarkers();
    void setType(Type t);
    double toDecibel();
    bool isVisible();

    TraceMarkerModel *model;
    Trace *parentTrace;
    double position;
    int number;
    // Frequency domain: S parameter
    // Time domain: imag part is impulse response, real part is step response
    std::complex<double> data;
    Trace::TimedomainData timeData;
    QPixmap symbol;
    Type type;
    QString suffix;
    QString description;

    TraceMarker *delta;
    std::vector<TraceMarker*> helperMarkers;
    TraceMarker *parent;
    union {
        double cutoffAmplitude;
        double peakThreshold;
        double offset;
    };

    bool timeDomain;
};

#endif // TRACEMARKER_H
