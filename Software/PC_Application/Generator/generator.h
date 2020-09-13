#ifndef GENERATOR_H
#define GENERATOR_H

#include "mode.h"
#include "signalgenwidget.h"

class Generator : public Mode
{
public:
    Generator(AppWindow *window);
    void initializeDevice() override;
private slots:
    void updateDevice();
private:
    SignalgeneratorWidget *central;
};

#endif // GENERATOR_H
