#include "generator.h"

#include <QSettings>

Generator::Generator(AppWindow *window, QString name)
    : Mode(window, name, "GENerator")
{
    central = new SignalgeneratorWidget(window->getDevice(), window);

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

    setupSCPI();

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

nlohmann::json Generator::toJSON()
{
    return central->toJSON();
}

void Generator::fromJSON(nlohmann::json j)
{
    if(j.is_null()) {
        return;
    }
    central->fromJSON(j);
}

void Generator::updateDevice()
{
    if(!window->getDevice() || Mode::getActiveMode() != this) {
        // can't update if not connected
        return;
    }
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::Generator;
    p.generator = central->getDeviceStatus();
    window->getDevice()->SendPacket(p);
}

void Generator::setupSCPI()
{
    add(new SCPICommand("FREQuency", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            central->setFrequency(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(central->getDeviceStatus().frequency);
    }));
    add(new SCPICommand("LVL", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {

            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            central->setLevel(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(central->getDeviceStatus().cdbm_level / 100.0);
    }));
    add(new SCPICommand("PORT", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval) || newval > 2) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            central->setPort(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(central->getDeviceStatus().activePort);
    }));
}
