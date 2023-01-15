#ifndef PORTEXTENSIONTESTS_H
#define PORTEXTENSIONTESTS_H

#include <QObject>

#include "portextension.h"

class PortExtensionTests : public QObject
{
    Q_OBJECT
public:
    explicit PortExtensionTests();

private slots:
    void autocalc();
    void correct();
private:
    std::vector<DeviceDriver::VNAMeasurement> dummyData;
};

#endif // PORTEXTENSIONTESTS_H
