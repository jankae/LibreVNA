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
signals:
    // Deembedding option may selfdestruct if not applicable with current settings. It should emit this signal before deleting itself
    void deleted(DeembeddingOption *option);
};

#endif // DEEMBEDDING_H
