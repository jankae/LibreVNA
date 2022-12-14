#ifndef TRACEAXIS_H
#define TRACEAXIS_H

#include "tracemodel.h"

#include <vector>
#include <QString>

class Axis {
public:
    Axis();
    virtual ~Axis(){}
    virtual double sampleToCoordinate(Trace::Data data, Trace *t = nullptr, unsigned int sample = 0) = 0;
    double transform(double value, double to_low, double to_high);
    double inverseTransform(double value, double to_low, double to_high);
    bool getLog() const;
    bool getAutorange() const;
    double getRangeMin() const;
    double getRangeMax() const;
    double getRangeDiv() const;
    const std::vector<double> &getTicks() const;

protected:
    void updateTicks();
    bool log;
    bool autorange;
    double rangeMin;
    double rangeMax;
    double rangeDiv;
    std::vector<double> ticks;
};

class XAxis : public Axis {
public:
    enum class Type {
        Frequency,
        Time,
        Distance,
        Power,
        TimeZeroSpan,
        Last,
    };
    XAxis();
    double sampleToCoordinate(Trace::Data data, Trace *t = nullptr, unsigned int sample = 0) override;
    void set(Type type, bool log, bool autorange, double min, double max, double div);
    static QString TypeToName(Type type);
    static Type TypeFromName(QString name);
    static QString Unit(Type type);
    QString TypeToName();
    QString Unit();

    Type getType() const;

    static bool isSupported(XAxis::Type type, TraceModel::DataSource source);

private:
    Type type;
};

class YAxis : public Axis {
public:
    enum class Type {
        Disabled,
        // S parameter options
        Magnitude,
        MagnitudedBuV,
        MagnitudeLinear,
        Phase,
        UnwrappedPhase,
        VSWR,
        Real,
        Imaginary,
        // derived parameter options
        SeriesR,
        Reactance,
        Capacitance,
        Inductance,
        QualityFactor,
        GroupDelay,
        // TDR options
        ImpulseReal,
        ImpulseMag,
        Step,
        Impedance,
        Last,
    };
    YAxis();
    double sampleToCoordinate(Trace::Data data, Trace *t = nullptr, unsigned int sample = 0) override;

    void set(Type type, bool log, bool autorange, double min, double max, double div);
    static QString TypeToName(Type type);
    static Type TypeFromName(QString name);
    static QString Unit(Type type, TraceModel::DataSource source = TraceModel::DataSource::VNA);
    static QString Prefixes(Type type, TraceModel::DataSource source = TraceModel::DataSource::VNA);
    QString TypeToName();
    QString Unit(TraceModel::DataSource source = TraceModel::DataSource::VNA);
    QString Prefixes(TraceModel::DataSource source = TraceModel::DataSource::VNA);

    Type getType() const;

    static std::set<YAxis::Type> getSupported(XAxis::Type type, TraceModel::DataSource source);
    static std::complex<double> reconstructValueFromYAxisType(std::map<Type, double> yaxistypes);

private:
    Type type;
};

#endif // TRACEAXIS_H
