#ifndef UNIT_H
#define UNIT_H

#include <QString>
#include <complex>

namespace Unit
{
    double FromString(QString string, QString unit = QString(), QString prefixes = " ");
    // prefixed need to be in ascending order (e.g. "m kMG" is okay, whjle "MkG" does not work)
    QString ToString(double value, QString unit = QString(), QString prefixes = " ", int precision = 6);
    double SIPrefixToFactor(char prefix);
    double dB(std::complex<double> d);
    double dB(double d);
};

#endif // UNIT_H
