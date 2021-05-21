#ifndef TRACEMODEL_H
#define TRACEMODEL_H

#include <QAbstractTableModel>
#include "trace.h"
#include <vector>
#include "Device/device.h"
#include "savable.h"

class TraceMarkerModel;

class TraceModel : public QAbstractTableModel, public Savable
{
    Q_OBJECT
public:
    TraceModel(QObject *parent = 0);
    ~TraceModel();

    enum {
        ColIndexVisible = 0,
        ColIndexPlayPause = 1,
        ColIndexMath = 2,
        ColIndexName = 3,
        ColIndexLast,
    };

    void addTrace(Trace *t);
    void removeTrace(unsigned int index);
    Trace *trace(unsigned int index);
    void toggleVisibility(unsigned int index);
    void togglePause(unsigned int index);
    void toggleMath(unsigned int index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    std::vector<Trace*> getTraces() const;

    bool PortExcitationRequired(int port);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    TraceMarkerModel *getMarkerModel() const;
    void setMarkerModel(TraceMarkerModel *value);

signals:
    void SpanChanged(double fmin, double fmax);
    void traceAdded(Trace *t);
    void traceRemoved(Trace *t);
    void requiredExcitation(bool excitePort1, bool excitePort2);
    void traceNameChanged(Trace *t);

public slots:
    void clearLiveData();
    void addVNAData(const Protocol::Datapoint& d, const Protocol::SweepSettings& settings);
    void addSAData(const Protocol::SpectrumAnalyzerResult& d, const Protocol::SpectrumAnalyzerSettings& settings);

private:
    std::vector<Trace*> traces;
    TraceMarkerModel *markerModel;
};

#endif // TRACEMODEL_H
