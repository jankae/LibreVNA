#include "generator.h"

Generator::Generator(AppWindow *window)
    : Mode(window, "Signal Generator")
{
    central = new SignalgeneratorWidget();
    finalize(central);
    connect(central, &SignalgeneratorWidget::SettingsChanged, this, &Generator::updateDevice);
}

void Generator::initializeDevice()
{
    updateDevice();
}

void Generator::updateDevice()
{
    if(!window->getDevice()) {
        // can't updat if not connected
        return;
    }
    auto status = central->getDeviceStatus();
    // TODO comment in once status is filled with valid values
//    window->getDevice()->SetManual(status);
}
