#ifndef FFTTESTS_H
#define FFTTESTS_H

#include <QtTest>

class fftTests : public QObject
{
    Q_OBJECT
public:
    fftTests();

private slots:
    void fft();
    void fftAndIfft();
    void ifftAndFft();
    void fftAndIfftWithShift();
};

#endif // FFTTESTS_H
