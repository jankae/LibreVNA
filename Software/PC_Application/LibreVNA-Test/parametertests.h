#ifndef PARAMETERTESTS_H
#define PARAMETERTESTS_H

#include <QtTest>

class ParameterTests : public QObject
{
    Q_OBJECT
public:
    ParameterTests();

private slots:
    void S2ABCD();
    void ABCD2S();
};

#endif // PARAMETERTESTS_H
