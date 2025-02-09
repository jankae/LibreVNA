#include "impedancerenormalizationtests.h"

ImpedanceRenormalizationTests::ImpedanceRenormalizationTests()
{
    renorm = new ImpedanceRenormalization();

    // set it up to normalize to 75 Ohm
    nlohmann::json j;
    j["impedance"] = 75;
    renorm->fromJSON(j);
}

ImpedanceRenormalizationTests::~ImpedanceRenormalizationTests()
{
    delete renorm;
}

void ImpedanceRenormalizationTests::OnePortTests()
{
    // Create dummy measurements
    DeviceDriver::VNAMeasurement measShort;
    measShort.pointNum = 0;
    measShort.Z0 = 50.0;
    measShort.measurements["S11"] = -1.0;

    auto measLoad = measShort;
    measLoad.measurements["S11"] = 0.0;

    auto measOpen = measShort;
    measOpen.measurements["S11"] = 1.0;

    // perform renormalization
    renorm->transformDatapoint(measShort);
    renorm->transformDatapoint(measLoad);
    renorm->transformDatapoint(measOpen);

    QVERIFY(measShort.measurements["S11"] == -1.0);
    // a Ohm load renormalized to 75 Ohm impedance has a reflection coefficient of -0.2
    QVERIFY(measLoad.measurements["S11"] == -0.2);
    QVERIFY(measOpen.measurements["S11"] == 1.0);
}

