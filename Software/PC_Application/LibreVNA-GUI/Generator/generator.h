#ifndef GENERATOR_H
#define GENERATOR_H

#include "mode.h"
#include "signalgenwidget.h"
#include "scpi.h"

class Generator : public Mode
{
    Q_OBJECT
public:
    Generator(AppWindow *window, QString name = "Signal Generator");
    void deactivate() override;
    void initializeDevice() override;

    virtual Type getType() override { return Type::SG;}

    virtual void resetSettings() override;

    // Nothing to do for now
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void setAveragingMode(Averaging::Mode mode) override {Q_UNUSED(mode)}

    void preset() override;

    virtual void deviceInfoUpdated() override;

private slots:
    void updateDevice();

private:
    void setupSCPI();
    SignalgeneratorWidget *central;
};

#endif // GENERATOR_H
