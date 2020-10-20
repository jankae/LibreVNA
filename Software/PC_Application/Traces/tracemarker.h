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
    };
    static std::vector<Type> getTypes() {
        return {Type::Manual, Type::Maximum, Type::Minimum, Type::Delta};
    }
    static QString typeToString(Type t) {
        switch(t) {
        case Type::Manual: return "Manual";
        case Type::Maximum: return "Maximum";
        case Type::Minimum: return "Minimum";
        case Type::Delta: return "Delta";
        default: return QString();
        }
    }
    void constrainFrequency();
    void assignDeltaMarker(TraceMarker *m);

    TraceMarkerModel *model;
    Trace *parentTrace;
    double frequency;
    int number;
    std::complex<double> data;
    QPixmap symbol;
    Type type;

    TraceMarker *delta;
};

#endif // TRACEMARKER_H
