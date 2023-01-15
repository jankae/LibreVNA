#include "portextensiontests.h"

#include "util.h"
#include "json.hpp"

#include <QtTest>

PortExtensionTests::PortExtensionTests() : QObject(nullptr)
{
    // create dummy data: port 1 is a simple open, port 2 is open with 1ns of (one-way) delay and some loss
    constexpr double startFreq = 1000000;
    constexpr double stopFreq = 6000000000;
    constexpr int steps = 501;
    for(int i=0;i<steps;i++) {
        double f = startFreq + (stopFreq - startFreq) * i / (steps - 1);
        DeviceDriver::VNAMeasurement m;
        m.frequency = f;
        m.dBm = -10;
        m.pointNum = i;
        m.Z0 = 50.0;
        m.measurements["S11"] = 1.0;
        m.measurements["S22"] = Util::addTransmissionLine(0.5, 50.0, 1e-9, 10, f);
        dummyData.push_back(m);
    }
}

void PortExtensionTests::autocalc()
{
    auto pe2 = new PortExtension();

    nlohmann::json j;
    j["port"] = 2;
    pe2->fromJSON(j);

    pe2->edit();
    pe2->measurementCompleted(dummyData);

    j = pe2->toJSON();
    QVERIFY(qFuzzyCompare((float)j.value("delay", 0.0), (float)1e-9));
    QVERIFY(qFuzzyCompare((float)j.value("DCloss", 0.0), (float)(-10*log10(0.5))));

    auto pe1 = new PortExtension();

    j.clear();
    j["port"] = 1;
    pe1->fromJSON(j);

    pe1->edit();
    pe1->measurementCompleted(dummyData);

    j = pe1->toJSON();
    QVERIFY(qFuzzyIsNull((float)j.value("delay", 0.0)));
    QVERIFY(qFuzzyIsNull((float)j.value("DCloss", 0.0)));
}

void PortExtensionTests::correct()
{
    auto pe = new PortExtension();
    nlohmann::json j;
    j["port"] = 2;
    pe->fromJSON(j);
    pe->edit();
    pe->measurementCompleted(dummyData);

    for(auto m : dummyData) {
        pe->transformDatapoint(m);
        QVERIFY(qFuzzyIsNull((float)m.measurements["S22"].imag()));
        QVERIFY(qFuzzyCompare((float)m.measurements["S22"].real(), 1.0f));
    }
}


