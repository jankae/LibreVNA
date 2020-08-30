#ifndef UNIT_H
#define UNIT_H

#include <QString>

class Unit
{
public:
    static double FromString(QString string, QString unit = QString(), QString prefixes = " ");
    static QString ToString(double value, QString unit = QString(), QString prefixes = " ", int precision = 6);
    static double SIPrefixToFactor(char prefix);
};

#endif // UNIT_H
