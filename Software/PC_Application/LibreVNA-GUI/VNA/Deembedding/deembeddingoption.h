#ifndef DEEMBEDDINGOPTION_H
#define DEEMBEDDINGOPTION_H

#include "savable.h"
#include "Device/devicedriver.h"
#include "Traces/tracemodel.h"
#include "scpi.h"

#include <QWidget>

class DeembeddingOption : public QObject, public Savable, public SCPINode
{
    Q_OBJECT
public:   
    virtual ~DeembeddingOption(){}
    enum class Type {
        PortExtension,
        TwoThru,
        MatchingNetwork,
        ImpedanceRenormalization,
        // Add new deembedding options here, do not explicitly assign values and keep the Last entry at the last position
        Last,
    };

    static DeembeddingOption *create(Type type);
    static QString TypeToString(Type type);
    static Type TypeFromString(QString string);

    virtual std::set<unsigned int> getAffectedPorts() = 0;
    virtual void transformDatapoint(DeviceDriver::VNAMeasurement &p) = 0;
    virtual void edit(){}
    virtual Type getType() = 0;

public slots:
    virtual void measurementCompleted(std::vector<DeviceDriver::VNAMeasurement> m){Q_UNUSED(m)}
signals:
    // Deembedding option may selfdestruct if not applicable with current settings. It should emit this signal before deleting itself
    void deleted(DeembeddingOption *option);

   void triggerMeasurement();

protected:
   DeembeddingOption(QString SCPIname)
       : SCPINode(SCPIname){}
};

#endif // DEEMBEDDING_H
