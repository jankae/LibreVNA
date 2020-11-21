#ifndef UNIT_H
#define UNIT_H

#include <QString>

namespace Unit
{
    double FromString(QString string, QString unit = QString(), QString prefixes = " ");
    QString ToString(double value, QString unit = QString(), QString prefixes = " ", int precision = 6);
    double SIPrefixToFactor(char prefix);
};

#endif // UNIT_H
