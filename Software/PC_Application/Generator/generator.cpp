#include "generator.h"
#include <QSettings>

Generator::Generator(AppWindow *window)
    : Mode(window, "Signal Generator")
{
    central = new SignalgeneratorWidget();

    auto pref = Preferences::getInstance();

    // set initial values
    if(pref.Startup.RememberSweepSettings) {
        QSettings s;
        central->setFrequency(s.value("GeneratorFrequency", pref.Startup.Generator.frequency).toDouble());
        central->setLevel(s.value("GeneratorLevel", pref.Startup.Generator.level).toDouble());
    } else {
        central->setFrequency(pref.Startup.Generator.frequency);
        central->setLevel(pref.Startup.Generator.level);
    }

    finalize(central);
    connect(central, &SignalgeneratorWidget::SettingsChanged, this, &Generator::updateDevice);
}

void Generator::deactivate()
{
    // store current settings
    QSettings s;
    auto settings = central->getDeviceStatus();
    s.setValue("GeneratorFrequency", static_cast<unsigned long long>(settings.frequency));
    s.setValue("GeneratorLevel", static_cast<unsigned long long>((double) settings.cdbm_level / 100.0));
    Mode::deactivate();
}

void Generator::initializeDevice()
{
    updateDevice();
}

void Generator::updateDevice()
{
    if(!window->getDevice()) {
        // can't update if not connected
        return;
    }
    // TODO comment in once status is filled with valid values
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::Generator;
    p.generator = central->getDeviceStatus();
    window->getDevice()->SendPacket(p);
}
