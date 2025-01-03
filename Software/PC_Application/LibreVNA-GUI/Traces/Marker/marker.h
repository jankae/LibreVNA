#ifndef TRACEMARKER_H
#define TRACEMARKER_H

#include "../trace.h"
#include "CustomWidgets/siunitedit.h"
#include "savable.h"

#include <QPixmap>
#include <QObject>
#include <QMenu>
#include <QComboBox>

class MarkerModel;
class MarkerGroup;

class Marker : public QObject, public Savable
{
    Q_OBJECT
public:
    Marker(MarkerModel *model, int number = 1, Marker *parent = nullptr, QString descr = QString());
    ~Marker();
    void assignTrace(Trace *t);
    Trace* trace();

    enum class Format {
        dB,
        dBm,
        dBuV,
        dBAngle,
        RealImag,
        Impedance,
        VSWR,
        SeriesR,
        Capacitance,
        Inductance,
        QualityFactor,
        GroupDelay,
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
        // compression parameters
        P1dB,                   // power level at 1dB compression
        Flatness,
        maxDeltaPos,
        maxDeltaNeg,
        // keep last at end
        Last,
    };

    static QString formatToString(Format f);
    static Format formatFromString(QString s);
    static std::vector<Format> formats();
    std::vector<Format> applicableFormats();
    std::vector<Format> defaultActiveFormats();

    QString readableData(Format format = Format::Last);
    QString readablePosition();
    QString readableSettings();
    QString tooltipSettings();
    QString readableType();
    QString domainToUnit();
    static QString domainToUnit(Trace::DataType domain);

    double getPosition() const;
    std::complex<double> getData() const;
    bool isMovable();
    bool isEditable();
    Trace::DataType getDomain();

    class Line {
    public:
        TraceMath::Data p1, p2;
    };
    std::vector<Line> getLines();

    QPixmap& getSymbol();

    unsigned long getCreationTimestamp() const;

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
        NegativePeakTable,
        Lowpass,
        Highpass,
        Bandpass,
        TOI,
        PhaseNoise,
        P1dB,
        Flatness,
        // keep last at end
        Last,
    };
    Type getType() const;
    QWidget *getTypeEditor(QAbstractItemDelegate *delegate = nullptr);
    void updateTypeFromEditor(QWidget *c);
    SIUnitEdit* getSettingsEditor();
    QWidget *getRestrictEditor();
    void adjustSettings(double value);
    bool isVisible();
    void setVisible(bool visible);

    QMenu *getContextMenu();

    // Updates marker position and data on automatic markers. Should be called whenever the tracedata is complete
    void update();
    Marker *getParent() const;
    const std::vector<Marker *>& getHelperMarkers() const;
    Marker *helperMarker(unsigned int i);
    bool canUseAsDelta(Marker *m);
    void assignDeltaMarker(Marker *m);
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

    MarkerGroup *getGroup() const;
    void setGroup(MarkerGroup *value);

public slots:
    void setPosition(double freq);
    void updateContextmenu();
signals:
    void positionChanged(double pos);
    void deleted(Marker *m);
    void dataChanged(Marker *m);
    void visibilityChanged(Marker *m);
    void symbolChanged(Marker *m);
    void typeChanged(Marker *m);
    void assignedDeltaChanged(Marker *m);
    void traceChanged(Marker *m);
    void beginRemoveHelperMarkers(Marker *m);
    void endRemoveHelperMarkers(Marker *m);
    void dataFormatChanged(Marker *m);

private slots:
    void parentTraceDeleted(Trace *t);
    void traceDataChanged(unsigned int begin, unsigned int end);
    void updateSymbol();
    void checkDeltaMarker();
    void deltaDeleted();
    void traceTypeChanged();
signals:
    void rawDataChanged();
    void domainChanged(Marker *m);
private:
    std::set<Type> getSupportedTypes();
    static QString typeToString(Type t) {
        switch(t) {
        case Type::Manual: return "Manual";
        case Type::Maximum: return "Maximum";
        case Type::Minimum: return "Minimum";
        case Type::Delta: return "Delta";
        case Type::PeakTable: return "Peak Table";
        case Type::NegativePeakTable: return "Negative Peak Table";
        case Type::Lowpass: return "Lowpass";
        case Type::Highpass: return "Highpass";
        case Type::Bandpass: return "Bandpass";
        case Type::TOI: return "TOI/IP3";
        case Type::PhaseNoise: return "Phase noise";
        case Type::P1dB: return "1dB compression";
        case Type::Flatness: return "Flatness";
        default: return QString();
        }
    }
    void constrainPosition();
    void constrainFormat();
    Marker *bestDeltaCandidate();
    void deleteHelperMarkers();
    void setType(Type t);
    double toDecibel();
    bool isDisplayedMarker();

    void setTableFormat(Format f);

    MarkerModel *model;
    Trace *parentTrace;
    unsigned long creationTimestamp;
    double position;
    double minPosition;
    double maxPosition;
    bool restrictPosition;
    int number;
    bool visible;
    // Frequency domain: S parameter
    // Time domain: impulse response
    std::complex<double> data;
    QPixmap symbol;
    Type type;
    QString suffix;
    QString description;

    QMenu contextmenu;

    Marker *delta;
    std::vector<Marker*> helperMarkers;
    Marker *parent;

    // additional lines the marker wants to show on the graphs (the graphs are responsible for drawing the lines)
    std::vector<Line> lines;

    // settings for the different marker types
    double cutoffAmplitude;
    double peakThreshold;
    double offset;

    // non-uniformity
    double maxDeltaNeg;
    double maxDeltaPos;

    Format formatTable;
    std::set<Format> formatGraph;

    MarkerGroup *group;
};

#endif // TRACEMARKER_H
