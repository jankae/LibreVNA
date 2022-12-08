#include "parameters.h"

using namespace std;

Sparam::Sparam(const Tparam &t) {
    m11 = t.m12 / t.m22;
    m21 = Type(1) / t.m22;
    m12 = (t.m11*t.m22 - t.m12*t.m21) / t.m22;
    m22 = -t.m21 / t.m22;
}

Sparam::Sparam(const ABCDparam &a, Type Z01, Type Z02) {
    auto denom = a.m11*Z02+a.m12+a.m21*Z01*Z02+a.m22*Z01;
    m11 = (a.m11*Z02+a.m12-a.m21*conj(Z01)*Z02-a.m22*conj(Z01)) / denom;
    m12 = (2.0*(a.m11*a.m22-a.m12*a.m21)*sqrt(real(Z01)*real(Z02))) / denom;
    m21 = (2.0*sqrt(real(Z01)*real(Z02))) / denom;
    m22 = (-a.m11*conj(Z02)+a.m12-a.m21*Z01*conj(Z02)+a.m22*Z01) / denom;
}

Sparam::Sparam(const ABCDparam &a, Type Z0)
    : Sparam(a, Z0, Z0)
{
}

ABCDparam::ABCDparam(const Sparam &s, Type Z01, Type Z02)
{
    auto denom = 2.0*s.m21*sqrt(real(Z01)*real(Z02));
    m11 = ((conj(Z01)+s.m11*Z01)*(1.0-s.m22)+s.m12*s.m21*Z01) / denom;
    m12 = ((conj(Z01)+s.m11*Z01)*(conj(Z02)+s.m22*Z02)-s.m12*s.m21*Z01*Z02) / denom;
    m21 = ((1.0-s.m11)*(1.0-s.m22)-s.m12*s.m21) / denom;
    m22 = ((1.0-s.m11)*(conj(Z02)+s.m22*Z02)+s.m12*s.m21*Z02) / denom;
}

Tparam::Tparam(const Sparam &s)
{
    m11 = -(s.m11*s.m22 - s.m12*s.m21) / s.m21;
    m12 = s.m11 / s.m21;
    m21 = -s.m22 / s.m21;
    m22 = 1.0 / s.m21;
}

ABCDparam::ABCDparam(const Sparam &s, Type Z0)
    : ABCDparam(s, Z0, Z0)
{
}

nlohmann::json Parameters::toJSON()
{
    nlohmann::json j;
    j["m11_real"] = m11.real();
    j["m11_imag"] = m11.imag();
    j["m12_real"] = m12.real();
    j["m12_imag"] = m12.imag();
    j["m21_real"] = m21.real();
    j["m21_imag"] = m21.imag();
    j["m22_real"] = m22.real();
    j["m22_imag"] = m22.imag();
    return j;
}

void Parameters::fromJSON(nlohmann::json j)
{
    m11 = complex<double>(j.value("m11_real", 0.0), j.value("m11_imag", 0.0));
    m12 = complex<double>(j.value("m12_real", 0.0), j.value("m12_imag", 0.0));
    m21 = complex<double>(j.value("m21_real", 0.0), j.value("m21_imag", 0.0));
    m22 = complex<double>(j.value("m22_real", 0.0), j.value("m22_imag", 0.0));
}

Yparam::Yparam(const Sparam &s, Type Z01, Type Z02)
{
    // from https://www.rfcafe.com/references/electrical/s-h-y-z.htm
    auto denom = (conj(Z01)+s.m11*Z01)*(conj(Z02)+s.m22*Z02)-s.m12*s.m21*Z01*Z02;
    m11 = ((1.0-s.m11)*(conj(Z02)+s.m22*Z02)+s.m12*s.m21*Z02) / denom;
    m12 = -2.0*s.m12*sqrt(real(Z01)*real(Z02));
    m21 = -2.0*s.m21*sqrt(real(Z01)*real(Z02));
    m22 = ((conj(Z01)+s.m11*Z01)*(1.0-s.m22)+s.m12*s.m21*Z01) / denom;
}

Yparam::Yparam(const Sparam &s, Type Z0)
    : Yparam(s, Z0, Z0)
{
}
