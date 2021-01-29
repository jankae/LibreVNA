#include "parameters.h"

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
