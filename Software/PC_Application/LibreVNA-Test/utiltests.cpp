#include "utiltests.h"

#include <vector>
#include "util.h"

using namespace std;

UtilTests::UtilTests()
{

}

void UtilTests::IdealCircleApproximation()
{
    vector<complex<double>> points;
    // create ideal circle points
    auto center = complex<double>(2.34, 4.12);
    auto radius = 5.0;
    static constexpr int numPoints = 10;
    for(int i=0;i<numPoints;i++) {
        auto angle = 2*M_PI * i / numPoints;
        auto offset = polar(radius, angle);
        points.push_back(center + offset);
    }
    auto circCenter = Util::findCenterOfCircle(points);
    QVERIFY(qFuzzyCompare(center.real(), circCenter.real()));
    QVERIFY(qFuzzyCompare(center.imag(), circCenter.imag()));
}

void UtilTests::IdealArcApproximation()
{
    vector<complex<double>> points;
    // create ideal circle points
    auto center = complex<double>(2.34, 4.12);
    auto radius = 5.0;
    static constexpr int numPoints = 10;
    for(int i=0;i<numPoints;i++) {
        auto angle = 0.1*2*M_PI * i / numPoints;
        auto offset = polar(radius, angle);
        points.push_back(center + offset);
    }
    auto circCenter = Util::findCenterOfCircle(points);
    QVERIFY(qFuzzyCompare(center.real(), circCenter.real()));
    QVERIFY(qFuzzyCompare(center.imag(), circCenter.imag()));
}

void UtilTests::NoisyCircleApproximation()
{
    srand(0);
    vector<complex<double>> points;
    // create ideal circle points
    auto center = complex<double>(2.34, 4.12);
    auto radius = 5.0;
    static constexpr int numPoints = 10;
    for(int i=0;i<numPoints;i++) {
        auto angle = 2*M_PI * i / numPoints;
        auto offset = polar(radius, angle);
        auto noise = polar(0.1*(double)rand() / RAND_MAX, 2*M_PI*(double)rand() / RAND_MAX);
        points.push_back(center + offset + noise);
    }
    auto circCenter = Util::findCenterOfCircle(points);
    constexpr double maxDelta = 0.1;
    QVERIFY(abs(center.real() - circCenter.real()) <= maxDelta);
    QVERIFY(abs(center.imag() - circCenter.imag()) <= maxDelta);
}

void UtilTests::FirmwareComparison()
{
    QVERIFY(Util::firmwareEqualOrHigher("3.1.2", "2.1.2") == true);
    QVERIFY(Util::firmwareEqualOrHigher("2.1.2", "2.0.2") == true);
    QVERIFY(Util::firmwareEqualOrHigher("2.1.2", "2.1.1") == true);
    QVERIFY(Util::firmwareEqualOrHigher("2.1.2", "2.1.2") == true);
    QVERIFY(Util::firmwareEqualOrHigher("2.1.1", "2.1.2") == false);

    QVERIFY(Util::firmwareEqualOrHigher("2.2.2", "2.3.1") == false);
    QVERIFY(Util::firmwareEqualOrHigher("2.2.2", "2.1.3") == true);

    QVERIFY(Util::firmwareEqualOrHigher("2.2.2", "2.3") == false);
    QVERIFY(Util::firmwareEqualOrHigher("2.2", "2.3.1") == false);
}

void UtilTests::ImpedanceSparameterCalculation()
{
    // Check impedance to S parameter conversion and back for different values
    auto Z = std::complex<double>(50.0);
    auto S = Util::ImpedanceToSparam(Z);
    QVERIFY(S == 0.0);
    QVERIFY(Util::SparamToImpedance(S) == Z);

    Z = std::complex<double>(0.0);
    S = Util::ImpedanceToSparam(Z);
    QVERIFY(S == -1.0);
    QVERIFY(Util::SparamToImpedance(S) == Z);

    Z = std::complex<double>(100.0);
    S = Util::ImpedanceToSparam(Z);
    QVERIFY(qFuzzyCompare(S.real(), 1.0/3));
    QVERIFY(S.imag() == 0.0);
    QVERIFY(qFuzzyCompare(Util::SparamToImpedance(S).real(), Z.real()));
    QVERIFY(qFuzzyCompare(Util::SparamToImpedance(S).imag(), Z.imag()));

    // Edge case: convert S parameter to impedance at 1.0 (which will give a +inf impedance)
    S = 1.0;
    Z = Util::SparamToImpedance(S);
    // convert back, we must land back at 1.0
    auto S_from_Z = Util::ImpedanceToSparam(Z);
    QVERIFY(S_from_Z == S);
}
