#ifndef CALIBRATIONTESTS_H
#define CALIBRATIONTESTS_H

#include <QtTest>

class CalibrationTests : public QObject
{
    Q_OBJECT
public:
    CalibrationTests();

private slots:
    void LinearDetection();
    void LogDetection();
    void MixedDetection();
};

#endif // CALIBRATIONTESTS_H
