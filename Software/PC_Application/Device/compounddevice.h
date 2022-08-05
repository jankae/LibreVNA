#ifndef COMPOUNDDEVICE_H
#define COMPOUNDDEVICE_H

#include "savable.h"

#include <vector>

#include <QString>

class CompoundDevice : public Savable
{
    friend class CompoundDeviceEditDialog;
public:
    CompoundDevice();

    virtual nlohmann::json toJSON();
    virtual void fromJSON(nlohmann::json j);

    class PortMapping {
    public:
        unsigned int device;
        unsigned int port;
    };

    enum class Synchronization {
        USB,
        ExtRef,
        Trigger,
        Last
    };

    static QString SyncToString(Synchronization sync);
    static Synchronization SyncFromString(QString s);

    QString getDesription();

    QString name;
    Synchronization sync;
    std::vector<QString> deviceSerials;
    std::vector<PortMapping> portMapping;
};

#endif // COMPOUNDDEVICE_H
