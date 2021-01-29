#ifndef TPARAM_H
#define TPARAM_H

#include <complex>

using Type = std::complex<double>;

class Parameters {
public:
    Parameters(Type m11, Type m12, Type m21, Type m22)
        : m11(m11), m12(m12), m21(m21), m22(m22){};
    Parameters(){};

    Type m11, m12, m21, m22;
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
    Sparam operator+(const Sparam &r) {
        Sparam p;
        p.m11 = this->m11+r.m11;
        p.m12 = this->m12+r.m12;
        p.m21 = this->m21+r.m21;
        p.m22 = this->m22+r.m22;
        return p;
    }
    Sparam operator*(const Type &r) {
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

//template<typename T>
//class Tparam {
//public:
//    Tparam(){};
//    Tparam(T t11, T t12, T t21, T t22)
//        : t11(t11), t12(t12), t21(t21), t22(t22){};
//    void fromSparam(T S11, T S21, T S12, T S22) {
//        t11 = -(S11*S22 - S12*S21) / S21;
//        t12 = S11 / S21;
//        t21 = -S22 / S21;
//        t22 = 1.0 / S21;
//    }
//    void toSparam(T &S11, T &S21, T &S12, T &S22) {
//        S11 = t12 / t22;
//        S21 = T(1) / t22;
//        S12 = (t11*t22 - t12*t21) / t22;
//        S22 = -t21 / t22;
//    }
//    Tparam inverse() {
//        Tparam i;
//        T det = t11*t22 - t12*t21;
//        i.t11 = t22 / det;
//        i.t12 = -t12 / det;
//        i.t21 = -t21 / det;
//        i.t22 = t11 / det;
//        return i;
//    }
//    Tparam root() {
//        // calculate root of 2x2 matrix, according to https://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix (choose positive roots)
//        auto tau = t11 + t22;
//        auto sigma = t11*t22 - t12*t21;
//        auto s = sqrt(sigma);
//        auto t = sqrt(tau + 2.0*s);
//        Tparam r = *this;
//        r.t11 += s;
//        r.t22 += s;
//        r = r * (1.0/t);
//        return r;
//    }
//    Tparam operator*(const Tparam &r) {
//        Tparam p;
//        p.t11 = t11*r.t11 + t12*r.t21;
//        p.t12 = t11*r.t12 + t12*r.t22;
//        p.t21 = t21*r.t11 + t22*r.t21;
//        p.t22 = t21*r.t12 + t22*r.t22;
//        return p;
//    }
//    Tparam operator*(const T &r) {
//        Tparam p;
//        p.t11 = t11 * r;
//        p.t12 = t12 * r;
//        p.t21 = t21 * r;
//        p.t22 = t22 * r;
//        return p;
//    }
//    T t11, t12, t21, t22;
//};

#endif // TPARAM_H
