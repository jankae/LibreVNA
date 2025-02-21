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
    void S2Z_1P();
    void S2Z_2P();
    void Z2S_1P();
    void Z2S_2P();
};

#endif // PARAMETERTESTS_H
