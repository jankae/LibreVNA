#ifndef DEEMBEDDINGOPTION_H
#define DEEMBEDDINGOPTION_H

#include "savable.h"
#include "Device/device.h"
#include "Traces/tracemodel.h"

#include <QWidget>

class DeembeddingOption : public QObject, public Savable
{
    Q_OBJECT
public:   
    enum class Type {
        PortExtension,
        TwoThru,
        MatchingNetwork,
        ImpedanceRenormalization,
        // Add new deembedding options here, do not explicitly assign values and keep the Last entry at the last position
        Last,
    };

    static DeembeddingOption *create(Type type);
    static QString getName(Type type);

    virtual std::set<unsigned int> getAffectedPorts() = 0;
    virtual void transformDatapoint(VirtualDevice::VNAMeasurement &p) = 0;
    virtual void edit(){}
    virtual Type getType() = 0;

public slots:
    virtual void measurementCompleted(std::vector<VirtualDevice::VNAMeasurement> m){Q_UNUSED(m)}
signals:
    // Deembedding option may selfdestruct if not applicable with current settings. It should emit this signal before deleting itself
    void deleted(DeembeddingOption *option);

   void triggerMeasurement();
};

#endif // DEEMBEDDING_H
