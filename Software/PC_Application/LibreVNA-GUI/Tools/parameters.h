#ifndef TPARAM_H
#define TPARAM_H

#include "savable.h"

#include <complex>

using Type = std::complex<double>;

class Parameters : public Savable {
public:
    Parameters(Type m11, Type m12, Type m21, Type m22)
        : m11(m11), m12(m12), m21(m21), m22(m22){}
    Parameters() : m11(0.0),m12(0.0),m21(0.0),m22(0.0){}

    Type m11, m12, m21, m22;

    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;
};

// forward declaration of parameter classes
class Sparam;
class Tparam;
class ABCDparam;

class Sparam : public Parameters {
public:
    using Parameters::Parameters;
    Sparam(const Tparam &t);
    Sparam(const ABCDparam &a, Type Z01, Type Z02);
    Sparam(const ABCDparam &a, Type Z0);
    Sparam operator+(const Sparam &r) const {
        Sparam p;
        p.m11 = this->m11+r.m11;
        p.m12 = this->m12+r.m12;
        p.m21 = this->m21+r.m21;
        p.m22 = this->m22+r.m22;
        return p;
    }
    Sparam operator*(const Type &r) const {
        Sparam p(m11*r, m12*r, m21*r, m22*r);
        return p;
    }
};

class ABCDparam : public Parameters {
public:
    using Parameters::Parameters;
    ABCDparam(const Sparam &s, Type Z01, Type Z02);
    ABCDparam(const Sparam &s, Type Z0);
    ABCDparam operator*(const ABCDparam &r) {
        ABCDparam p;
        p.m11 = this->m11*r.m11 + this->m12*r.m21;
        p.m12 = this->m11*r.m12 + this->m12*r.m22;
        p.m21 = this->m21*r.m11 + this->m22*r.m21;
        p.m22 = this->m21*r.m12 + this->m22*r.m22;
        return p;
    }
    ABCDparam inverse() {
        ABCDparam i;
        Type det = m11*m22 - m12*m21;
        i.m11 = m22 / det;
        i.m12 = -m12 / det;
        i.m21 = -m21 / det;
        i.m22 = m11 / det;
        return i;
    }
    ABCDparam operator*(const Type &r) {
        ABCDparam p(m11*r, m12*r, m21*r, m22*r);
        return p;
    }
    ABCDparam root() {
        // calculate root of 2x2 matrix, according to https://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix (choose positive roots)
        auto tau = m11 + m22;
        auto sigma = m11*m22 - m12*m21;
        auto s = sqrt(sigma);
        auto t = sqrt(tau + 2.0*s);
        ABCDparam r = *this;
        r.m11 += s;
        r.m22 += s;
        r = r * (1.0/t);
        return r;
    }
};

class Tparam : public Parameters {
public:
    using Parameters::Parameters;
    Tparam(const Sparam &s);
    Tparam operator*(const Tparam &r) {
        Tparam p;
        p.m11 = this->m11*r.m11 + this->m12*r.m21;
        p.m12 = this->m11*r.m12 + this->m12*r.m22;
        p.m21 = this->m21*r.m11 + this->m22*r.m21;
        p.m22 = this->m21*r.m12 + this->m22*r.m22;
        return p;
    }
    Tparam operator+(const Tparam &r) {
        Tparam p;
        p.m11 = this->m11+r.m11;
        p.m12 = this->m12+r.m12;
        p.m21 = this->m21+r.m21;
        p.m22 = this->m22+r.m22;
        return p;
    }
    Tparam inverse() {
        Tparam i;
        Type det = m11*m22 - m12*m21;
        i.m11 = m22 / det;
        i.m12 = -m12 / det;
        i.m21 = -m21 / det;
        i.m22 = m11 / det;
        return i;
    }
    Tparam operator*(const Type &r) {
        Tparam p(m11*r, m12*r, m21*r, m22*r);
        return p;
    }
    Tparam root() {
        // calculate root of 2x2 matrix, according to https://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix (choose positive roots)
        auto tau = m11 + m22;
        auto sigma = m11*m22 - m12*m21;
        auto s = sqrt(sigma);
        auto t = sqrt(tau + 2.0*s);
        Tparam r = *this;
        r.m11 += s;
        r.m22 += s;
        r = r * (1.0/t);
        return r;
    }
};

#endif // TPARAM_H
