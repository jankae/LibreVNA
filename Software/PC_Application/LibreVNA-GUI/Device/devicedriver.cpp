#include "devicedriver.h"

DeviceDriver *DeviceDriver::activeDriver = nullptr;

bool DeviceDriver::connectDevice(QString serial)
{
    if(activeDriver && activeDriver != this) {
        activeDriver->disconnect();
    }
    if(connectTo(serial)) {
        activeDriver = this;
        return true;
    } else {
        return false;
    }
}

void DeviceDriver::disconnectDevice()
{
    disconnect();
    activeDriver = nullptr;
}
