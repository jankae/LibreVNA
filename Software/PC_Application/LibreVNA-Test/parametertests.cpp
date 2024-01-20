#include "parametertests.h"

#include "Tools/parameters.h"

#include <QDebug>

ParameterTests::ParameterTests()
{

}

void ParameterTests::S2ABCD()
{
    using namespace std::complex_literals;

    std::complex<double> S11 = 0.0038 + 0.0248i;
    std::complex<double> S12 = 0.9961 - 0.0250i;
    std::complex<double> S21 = 0.9964 - 0.0254i;
    std::complex<double> S22 = 0.0037 + 0.0249i;
    auto S = Sparam(S11, S12, S21, S22);
    auto abcd = ABCDparam(S, 50.0);

    std::complex<double> A = 0.99988705861501226 + 0.00014900771660107621i;
    std::complex<double> B = 0.31256891513454765 + 2.5194422425174801i;
    std::complex<double> C = -2.7673838933081734e-09 + 6.9983236736743502e-06i;
    std::complex<double> D = 0.99978420576400329 + 0.00024674711602337137i;

    QVERIFY(qFuzzyCompare(abcd.m11.real(), A.real()));
    QVERIFY(qFuzzyCompare(abcd.m11.imag(), A.imag()));
    QVERIFY(qFuzzyCompare(abcd.m12.real(), B.real()));
    QVERIFY(qFuzzyCompare(abcd.m12.imag(), B.imag()));
    QVERIFY(qFuzzyCompare(abcd.m21.real(), C.real()));
    QVERIFY(qFuzzyCompare(abcd.m21.imag(), C.imag()));
    QVERIFY(qFuzzyCompare(abcd.m22.real(), D.real()));
    QVERIFY(qFuzzyCompare(abcd.m22.imag(), D.imag()));
}

void ParameterTests::ABCD2S()
{
    using namespace std::complex_literals;

    std::complex<double> A = 0.99988705861501226 + 0.00014900771660107621i;
    std::complex<double> B = 0.31256891513454765 + 2.5194422425174801i;
    std::complex<double> C = -2.7673838933081734e-09 + 6.9983236736743502e-06i;
    std::complex<double> D = 0.99978420576400329 + 0.00024674711602337137i;
    auto abcd = ABCDparam(A, B, C, D);

    auto s = Sparam(abcd, 50.0);

    std::complex<double> S11 = 0.0038 + 0.0248i;
    std::complex<double> S12 = 0.9961 - 0.0250i;
    std::complex<double> S21 = 0.9964 - 0.0254i;
    std::complex<double> S22 = 0.0037 + 0.0249i;

    QVERIFY(qFuzzyCompare(s.m11.real(), S11.real()));
    QVERIFY(qFuzzyCompare(s.m11.imag(), S11.imag()));
    QVERIFY(qFuzzyCompare(s.m12.real(), S12.real()));
    QVERIFY(qFuzzyCompare(s.m12.imag(), S12.imag()));
    QVERIFY(qFuzzyCompare(s.m21.real(), S21.real()));
    QVERIFY(qFuzzyCompare(s.m21.imag(), S21.imag()));
    QVERIFY(qFuzzyCompare(s.m22.real(), S22.real()));
    QVERIFY(qFuzzyCompare(s.m22.imag(), S22.imag()));
}
