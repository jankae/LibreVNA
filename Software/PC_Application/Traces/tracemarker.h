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
    TraceMarker(TraceMarkerModel *model, int number = 1);
    ~TraceMarker();
    void assignTrace(Trace *t);
    Trace* trace();
    QString readableData();
    QString readableSettings();

    double getFrequency() const;
    std::complex<double> getData() const;

    QPixmap& getSymbol();

    int getNumber() const;
    bool editingFrequeny;
    Trace *getTrace() const;

    void setNumber(int value);

    QWidget *getTypeEditor(QAbstractItemDelegate *delegate = nullptr);
    void updateTypeFromEditor(QWidget *c);

    SIUnitEdit* getSettingsEditor();
    void adjustSettings(double value);

    // Updates marker position and data on automatic markers. Should be called whenever the tracedata is complete
    void update();

public slots:
    void setFrequency(double freq);
signals:
    void deleted(TraceMarker *m);
    void dataChanged(TraceMarker *m);
    void symbolChanged(TraceMarker *m);
    void typeChanged(TraceMarker *m);

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
        Lowpass,
        Highpass,
        Bandpass,
        TOI,
    };
    std::set<Type> getSupportedTypes();
    static QString typeToString(Type t) {
        switch(t) {
        case Type::Manual: return "Manual";
        case Type::Maximum: return "Maximum";
        case Type::Minimum: return "Minimum";
        case Type::Delta: return "Delta";
        case Type::Lowpass: return "Lowpass";
        case Type::Highpass: return "Highpass";
        case Type::Bandpass: return "Bandpass";
        case Type::TOI: return "TOI/IP3";
        default: return QString();
        }
    }
    void constrainFrequency();
    void assignDeltaMarker(TraceMarker *m);
    void deleteHelperMarkers();
    void setType(Type t);
    double toDecibel();

    TraceMarkerModel *model;
    Trace *parentTrace;
    double frequency;
    int number;
    std::complex<double> data;
    QPixmap symbol;
    Type type;
    QString suffix;

    TraceMarker *delta;
    std::vector<TraceMarker*> helperMarkers;
    double cutoffAmplitude;
};

#endif // TRACEMARKER_H
