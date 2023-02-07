#ifndef COMPOUNDDEVICE_H
#define COMPOUNDDEVICE_H

#include "savable.h"
#include "../librevnadriver.h"

#include <vector>

#include <QString>

class CompoundDevice : public Savable
{
    friend class CompoundDeviceEditDialog;
public:
    CompoundDevice();

    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;

    class PortMapping {
    public:
        unsigned int device; // starts at zero
        unsigned int port;   // starts at zero
        static unsigned int findActiveStage(std::vector<PortMapping> map, unsigned int device, unsigned int port);
    };

    static QString SyncToString(LibreVNADriver::Synchronization sync);
    static LibreVNADriver::Synchronization SyncFromString(QString s);

    QString getDesription();

    QString name;
    LibreVNADriver::Synchronization sync;
    std::vector<QString> deviceSerials;
    std::vector<PortMapping> portMapping;
};

#endif // COMPOUNDDEVICE_H
