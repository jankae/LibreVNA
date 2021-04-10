#ifndef GENERATOR_H
#define GENERATOR_H

#include "mode.h"
#include "signalgenwidget.h"
#include "scpi.h"

class Generator : public Mode, public SCPINode
{
public:
    Generator(AppWindow *window);
    void deactivate() override;
    void initializeDevice() override;

    // Nothing to do for now
    virtual nlohmann::json toJSON() override {return nlohmann::json();};
    virtual void fromJSON(nlohmann::json j) override {Q_UNUSED(j)};

private slots:
    void updateDevice();

private:
    void setupSCPI();
    SignalgeneratorWidget *central;
};

#endif // GENERATOR_H
