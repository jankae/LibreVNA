#ifndef TRACEMODEL_H
#define TRACEMODEL_H

#include <QAbstractTableModel>
#include "trace.h"
#include <vector>
#include "Device/device.h"

class TraceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TraceModel(QObject *parent = 0);

    void addTrace(Trace *t);
    void removeTrace(unsigned int index);
    Trace *trace(unsigned int index);
    void toggleVisibility(unsigned int index);
    void togglePause(unsigned int index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    std::vector<Trace*> getTraces();
signals:
    void traceAdded(Trace *t);
    void traceRemoved(Trace *t);

public slots:
    void clearVNAData();
    void addVNAData(Protocol::Datapoint d);

private:
    std::vector<Trace*> traces;
};

#endif // TRACEMODEL_H
