#ifndef TRACEMARKER_H
#define TRACEMARKER_H

#include <QPixmap>
#include <QObject>
#include "trace.h"
#include <QMenu>
#include <QComboBox>
#include "CustomWidgets/siunitedit.h"
#include "savable.h"

class TraceMarkerModel;

class TraceMarker : public QObject, public Savable
{
    Q_OBJECT;
public:
    TraceMarker(TraceMarkerModel *model, int number = 1, TraceMarker *parent = nullptr, QString descr = QString());
    ~TraceMarker();
    void assignTrace(Trace *t);
    Trace* trace();

    enum class Format {
        dB,
        dBAngle,
        RealImag,
        Impedance,
        // Noise marker parameters
        Noise,
        PhaseNoise,
        // Filter parameters
        CenterBandwidth,
        Cutoff,
        InsertionLoss,
        // TOI parameters
        TOI,                    // third order intercept point
        AvgTone,                // average level of tone
        AvgModulationProduct,   // average level of modulation products
        // keep last at end
        Last,
    };

    static QString formatToString(Format f);
    static Format formatFromString(QString s);
    static std::vector<Format> formats();
    std::vector<Format> applicableFormats();

    QString readableData(Format format = Format::Last);
    QString readablePosition();
    QString readableSettings();
    QString tooltipSettings();
    QString readableType();

    double getPosition() const;
    std::complex<double> getData() const;
    bool isMovable();
    bool isTimeDomain();

    QPixmap& getSymbol();

    int getNumber() const;
    void setNumber(int value);

    bool editingFrequency;
    Trace *getTrace() const;

    enum class Type {
        Manual,
        Maximum,
        Minimum,
        Delta,
        PeakTable,
        Lowpass,
        Highpass,
        Bandpass,
        TOI,
        PhaseNoise,
        // keep last at end
        Last,
    };
    Type getType() const;
    QWidget *getTypeEditor(QAbstractItemDelegate *delegate = nullptr);
    void updateTypeFromEditor(QWidget *c);
    SIUnitEdit* getSettingsEditor();
    void adjustSettings(double value);

    QMenu *getContextMenu();

    // Updates marker position and data on automatic markers. Should be called whenever the tracedata is complete
    void update();
    TraceMarker *getParent() const;
    const std::vector<TraceMarker *>& getHelperMarkers() const;
    TraceMarker *helperMarker(unsigned int i);
    void assignDeltaMarker(TraceMarker *m);
    QString getSuffix() const;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    // Markers are referenced by pointers throughout this project (e.g. when added to a trace)
    // When saving the current marker configuration, the pointer is not useful (e.g. for determining
    // the associated delta marker. Instead a marker hash is saved to identify the correct marker.
    // The hash should be influenced by every setting the marker can have. It should not depend on
    // the marker data.
    unsigned int toHash();


    std::set<Format> getGraphDisplayFormats() const;

public slots:
    void setPosition(double freq);
signals:
    void deleted(TraceMarker *m);
    void dataChanged(TraceMarker *m);
    void symbolChanged(TraceMarker *m);
    void typeChanged(TraceMarker *m);
    void assignedDeltaChanged(TraceMarker *m);
    void traceChanged(TraceMarker *m);
    void beginRemoveHelperMarkers(TraceMarker *m);
    void endRemoveHelperMarkers(TraceMarker *m);
    void dataFormatChanged(TraceMarker *m);

private slots:
    void parentTraceDeleted(Trace *t);
    void traceDataChanged();
    void updateSymbol();
    void checkDeltaMarker();
    void deltaDeleted();
    void updateContextmenu();
signals:
    void rawDataChanged();
    void domainChanged();
private:
    std::set<Type> getSupportedTypes();
    static QString typeToString(Type t) {
        switch(t) {
        case Type::Manual: return "Manual";
        case Type::Maximum: return "Maximum";
        case Type::Minimum: return "Minimum";
        case Type::Delta: return "Delta";
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
    void constrainFormat();
    TraceMarker *bestDeltaCandidate();
    void deleteHelperMarkers();
    void setType(Type t);
    double toDecibel();
    bool isVisible();

    void setTableFormat(Format f);

    TraceMarkerModel *model;
    Trace *parentTrace;
    double position;
    int number;
    // Frequency domain: S parameter
    // Time domain: impulse response
    std::complex<double> data;
    QPixmap symbol;
    Type type;
    QString suffix;
    QString description;

    QMenu contextmenu;

    TraceMarker *delta;
    std::vector<TraceMarker*> helperMarkers;
    TraceMarker *parent;
    // settings for the different marker types
    double cutoffAmplitude;
    double peakThreshold;
    double offset;

    Format formatTable;
    std::set<Format> formatGraph;
};

#endif // TRACEMARKER_H
