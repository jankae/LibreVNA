#ifndef HAROGICB60_H
#define HAROGICB60_H

#include "../LibreVNA/librevnausbdriver.h"

class HarogicB60 : public LibreVNAUSBDriver
{
    Q_OBJECT
public:
    HarogicB60();

    /**
     * @brief Returns the driver name. It must be unique across all implemented drivers and is used to identify the driver
     * @return driver name
     */
    virtual QString getDriverName() override;
};

#endif // HAROGICB60_H
