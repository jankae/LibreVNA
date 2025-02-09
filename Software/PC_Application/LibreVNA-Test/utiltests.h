#ifndef UTILTESTS_H
#define UTILTESTS_H

#include <QtTest>

class UtilTests : public QObject
{
    Q_OBJECT
public:
    UtilTests();

private slots:
    void IdealCircleApproximation();
    void IdealArcApproximation();
    void NoisyCircleApproximation();
    void FirmwareComparison();
    void ImpedanceSparameterCalculation();
};

#endif // UTILTESTS_H
