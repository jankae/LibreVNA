#ifndef DEEMBEDDINGOPTION_H
#define DEEMBEDDINGOPTION_H

#include <QWidget>
#include "savable.h"
#include "Device/device.h"

class DeembeddingOption : public QObject, public Savable
{
    Q_OBJECT
public:   
    enum class Type {
        PortExtension,
        TwoThru,
        MatchingNetwork,
        // Add new deembedding options here, do not explicitly assign values and keep the Last entry at the last position
        Last,
    };

    static DeembeddingOption *create(Type type);
    static QString getName(Type type);

    virtual void transformDatapoint(Protocol::Datapoint &p) = 0;
    virtual void edit(){};
    virtual Type getType() = 0;

public slots:
    virtual void measurementCompleted(std::vector<Protocol::Datapoint> m){Q_UNUSED(m)};
signals:
    // Deembedding option may selfdestruct if not applicable with current settings. It should emit this signal before deleting itself
    void deleted(DeembeddingOption *option);

   void triggerMeasurement(bool S11 = true, bool S12 = true, bool S21 = true, bool S22 = true);
};

#endif // DEEMBEDDING_H
