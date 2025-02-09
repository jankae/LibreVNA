#ifndef IMPEDANCERENORMALIZATIONTESTS_H
#define IMPEDANCERENORMALIZATIONTESTS_H

#include <QtTest>

#include "impedancerenormalization.h"

class ImpedanceRenormalizationTests : public QObject
{
    Q_OBJECT
public:
    ImpedanceRenormalizationTests();
    ~ImpedanceRenormalizationTests();
private slots:
    void OnePortTests();

private:
    ImpedanceRenormalization *renorm;
};

#endif // IMPEDANCERENORMALIZATIONTESTS_H
