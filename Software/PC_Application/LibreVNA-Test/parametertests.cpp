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

    QVERIFY(qFuzzyCompare(abcd.get(1,1).real(), A.real()));
    QVERIFY(qFuzzyCompare(abcd.get(1,1).imag(), A.imag()));
    QVERIFY(qFuzzyCompare(abcd.get(1,2).real(), B.real()));
    QVERIFY(qFuzzyCompare(abcd.get(1,2).imag(), B.imag()));
    QVERIFY(qFuzzyCompare(abcd.get(2,1).real(), C.real()));
    QVERIFY(qFuzzyCompare(abcd.get(2,1).imag(), C.imag()));
    QVERIFY(qFuzzyCompare(abcd.get(2,2).real(), D.real()));
    QVERIFY(qFuzzyCompare(abcd.get(2,2).imag(), D.imag()));
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

    QVERIFY(qFuzzyCompare(s.get(1,1).real(), S11.real()));
    QVERIFY(qFuzzyCompare(s.get(1,1).imag(), S11.imag()));
    QVERIFY(qFuzzyCompare(s.get(1,2).real(), S12.real()));
    QVERIFY(qFuzzyCompare(s.get(1,2).imag(), S12.imag()));
    QVERIFY(qFuzzyCompare(s.get(2,1).real(), S21.real()));
    QVERIFY(qFuzzyCompare(s.get(2,1).imag(), S21.imag()));
    QVERIFY(qFuzzyCompare(s.get(2,2).real(), S22.real()));
    QVERIFY(qFuzzyCompare(s.get(2,2).imag(), S22.imag()));
}

void ParameterTests::S2Z_1P()
{
    using namespace std::complex_literals;

    std::complex<double> S11 = 0.0038 + 0.0248i;
    auto S = Sparam(S11);

    // test for various characteristic impedances
    for(auto Z0 = 10.0; Z0 <= 100.0; Z0 += 10.0) {
        auto Z = Zparam(S, Z0);

        // calculate expected Z values based on two-port formulas
        auto Z11 = (1.0+S11) / (1.0-S11) * Z0;

        // error due to floating point calculations are too big for qFuzzyCompare(double, double), use qFuzzyCompare(float, float) instead
        QVERIFY(qFuzzyCompare((float)Z.get(1,1).real(), (float)Z11.real()));
        QVERIFY(qFuzzyCompare((float)Z.get(1,1).imag(), (float)Z11.imag()));
    }
}

void ParameterTests::S2Z_2P()
{
    using namespace std::complex_literals;

    std::complex<double> S11 = 0.0038 + 0.0248i;
    std::complex<double> S12 = 0.9961 - 0.0250i;
    std::complex<double> S21 = 0.9964 - 0.0254i;
    std::complex<double> S22 = 0.0037 + 0.0249i;
    auto S = Sparam(S11, S12, S21, S22);

    // test for various characteristic impedances
    for(auto Z0 = 10.0; Z0 <= 100.0; Z0 += 10.0) {
        auto Z = Zparam(S, Z0);

        // calculate expected Z values based on two-port formulas
        auto deltaS = (1.0-S.get(1,1))*(1.0-S.get(2,2))-S.get(1,2)*S.get(2,1);
        auto Z11 = ((1.0+S.get(1,1))*(1.0-S.get(2,2))+S.get(1,2)*S.get(2,1))/deltaS*Z0;
        auto Z12 = 2.0*S.get(1,2)/deltaS*Z0;
        auto Z21 = 2.0*S.get(2,1)/deltaS*Z0;
        auto Z22 = ((1.0-S.get(1,1))*(1.0+S.get(2,2))+S.get(1,2)*S.get(2,1))/deltaS*Z0;

        // error due to floating point calculations are too big for qFuzzyCompare(double, double), use qFuzzyCompare(float, float) instead
        QVERIFY(qFuzzyCompare((float)Z.get(1,1).real(), (float)Z11.real()));
        QVERIFY(qFuzzyCompare((float)Z.get(1,1).imag(), (float)Z11.imag()));
        QVERIFY(qFuzzyCompare((float)Z.get(1,2).real(), (float)Z12.real()));
        QVERIFY(qFuzzyCompare((float)Z.get(1,2).imag(), (float)Z12.imag()));
        QVERIFY(qFuzzyCompare((float)Z.get(2,1).real(), (float)Z21.real()));
        QVERIFY(qFuzzyCompare((float)Z.get(2,1).imag(), (float)Z21.imag()));
        QVERIFY(qFuzzyCompare((float)Z.get(2,2).real(), (float)Z22.real()));
        QVERIFY(qFuzzyCompare((float)Z.get(2,2).imag(), (float)Z22.imag()));
    }
}

void ParameterTests::Z2S_1P()
{
    using namespace std::complex_literals;

    std::complex<double> Z11 = 0.0038 + 0.0248i;
    auto Z = Zparam(Z11);

    // test for various characteristic impedances
    for(auto Z0 = 10.0; Z0 <= 100.0; Z0 += 10.0) {
        auto S = Sparam(Z, Z0);

        // calculate expected Z values based on two-port formulas
        auto S11 = (Z11/Z0-1.0) / (Z11/Z0+1.0);

        // error due to floating point calculations are too big for qFuzzyCompare(double, double), use qFuzzyCompare(float, float) instead
        QVERIFY(qFuzzyCompare((float)S.get(1,1).real(), (float)S11.real()));
        QVERIFY(qFuzzyCompare((float)S.get(1,1).imag(), (float)S11.imag()));
    }
}

void ParameterTests::Z2S_2P()
{
    using namespace std::complex_literals;

    std::complex<double> Z11 = 0.0038 + 0.0248i;
    std::complex<double> Z12 = 0.9961 - 0.0250i;
    std::complex<double> Z21 = 0.9964 - 0.0254i;
    std::complex<double> Z22 = 0.0037 + 0.0249i;
    auto Z = Zparam(Z11, Z12, Z21, Z22);

    // test for various characteristic impedances
    for(auto Z0 = 10.0; Z0 <= 100.0; Z0 += 10.0) {
        auto S = Sparam(Z, Z0);

        // calculate expected Z values based on two-port formulas
        auto delta = (Z.get(1,1)+Z0)*(Z.get(2,2)+Z0)-Z.get(1,2)*Z.get(2,1);
        auto S11 = ((Z.get(1,1)-Z0)*(Z.get(2,2)+Z0)-Z.get(1,2)*Z.get(2,1))/delta;
        auto S12 = 2.0*Z0*Z.get(1,2)/delta;
        auto S21 = 2.0*Z0*Z.get(2,1)/delta;
        auto S22 = ((Z.get(1,1)+Z0)*(Z.get(2,2)-Z0)-Z.get(1,2)*Z.get(2,1))/delta;

        // error due to floating point calculations are too big for qFuzzyCompare(double, double), use qFuzzyCompare(float, float) instead
        QVERIFY(qFuzzyCompare((float)S.get(1,1).real(), (float)S11.real()));
        QVERIFY(qFuzzyCompare((float)S.get(1,1).imag(), (float)S11.imag()));
        QVERIFY(qFuzzyCompare((float)S.get(1,2).real(), (float)S12.real()));
        QVERIFY(qFuzzyCompare((float)S.get(1,2).imag(), (float)S12.imag()));
        QVERIFY(qFuzzyCompare((float)S.get(2,1).real(), (float)S21.real()));
        QVERIFY(qFuzzyCompare((float)S.get(2,1).imag(), (float)S21.imag()));
        QVERIFY(qFuzzyCompare((float)S.get(2,2).real(), (float)S22.real()));
        QVERIFY(qFuzzyCompare((float)S.get(2,2).imag(), (float)S22.imag()));
    }
}
