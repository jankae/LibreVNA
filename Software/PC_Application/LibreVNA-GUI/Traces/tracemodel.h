#ifndef TRACEMODEL_H
#define TRACEMODEL_H

#include "Device/devicedriver.h"
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
        ColIndexDeembedding = 2,
        ColIndexMath = 3,
        ColIndexName = 4,
        ColIndexLast,
    };

    enum class DataSource {
        VNA,
        SA,
        Unknown,
    };

    void addTrace(Trace *t);
    void removeTrace(unsigned int index);
    void removeTrace(Trace *t);
    Trace *trace(unsigned int index);
    int findIndex(Trace *t);
    void toggleVisibility(unsigned int index);
    void togglePause(unsigned int index);
    void toggleMath(unsigned int index);
    void toggleDeembedding(unsigned int index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    std::vector<Trace*> getTraces() const;
    std::vector<Trace*> getLiveTraces() const;

    bool PortExcitationRequired(int port);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    MarkerModel *getMarkerModel() const;
    void setMarkerModel(MarkerModel *value);

    DataSource getSource() const;
    void setSource(const DataSource &value);

    double getSweepPosition() const;

signals:
    void SpanChanged(double fmin, double fmax);
    void traceAdded(Trace *t);
    void traceRemoved(Trace *t);
    void requiredExcitation();
    void traceNameChanged(Trace *t);

public slots:
    void clearLiveData();
    void addVNAData(const DeviceDriver::VNAMeasurement& d, TraceMath::DataType datatype, bool deembedded);
    void addSAData(const DeviceDriver::SAMeasurement &d, const DeviceDriver::SASettings &settings);

private:
    DataSource source;
    double lastSweepPosition;
    QDateTime lastReceivedData;
    std::vector<Trace*> traces;
    MarkerModel *markerModel;
};

#endif // TRACEMODEL_H
