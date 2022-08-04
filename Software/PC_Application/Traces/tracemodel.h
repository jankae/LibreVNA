#ifndef TRACEMODEL_H
#define TRACEMODEL_H

#include "Device/device.h"
#include "savable.h"
#include "trace.h"

#include <QAbstractTableModel>
#include <vector>

class MarkerModel;

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

    enum class DataSource {
        VNA,
        SA,
        Unknown,
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

    MarkerModel *getMarkerModel() const;
    void setMarkerModel(MarkerModel *value);

    DataSource getSource() const;
    void setSource(const DataSource &value);

signals:
    void SpanChanged(double fmin, double fmax);
    void traceAdded(Trace *t);
    void traceRemoved(Trace *t);
    void requiredExcitation(bool excitePort1, bool excitePort2);
    void traceNameChanged(Trace *t);

public slots:
    void clearLiveData();
    void addVNAData(const VirtualDevice::VNAMeasurement& d, TraceMath::DataType datatype);
    void addSAData(const VirtualDevice::SAMeasurement &d, const VirtualDevice::SASettings &settings);

private:
    DataSource source;
    std::vector<Trace*> traces;
    MarkerModel *markerModel;
};

#endif // TRACEMODEL_H
