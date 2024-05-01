#include "generator.h"

#include "CustomWidgets/informationbox.h"

#include <QSettings>

Generator::Generator(AppWindow *window, QString name)
    : Mode(window, name, "GENerator")
{
    central = new SignalgeneratorWidget(window, window);

    auto& pref = Preferences::getInstance();

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
    s.setValue("GeneratorFrequency", static_cast<unsigned long long>(settings.freq));
    s.setValue("GeneratorLevel", static_cast<unsigned long long>((double) settings.dBm));
    Mode::deactivate();
}

void Generator::initializeDevice()
{
    if(!window->getDevice()->supports(DeviceDriver::Feature::Generator)) {
        InformationBox::ShowError("Unsupported", "The connected device does not support generator mode");
        return;
    }
    updateDevice();
}

void Generator::resetSettings()
{
    central->setFrequency(1000000000);
    central->setLevel(0);
    central->setPort(0);
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

void Generator::preset()
{

}

void Generator::deviceInfoUpdated()
{
    central->deviceInfoUpdated();
}

void Generator::updateDevice()
{
    if(!window->getDevice() || isActive != true) {
        // can't update if not connected
        return;
    }
    window->getDevice()->setSG(central->getDeviceStatus());
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
        return QString::number(central->getDeviceStatus().freq);
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
        return QString::number(central->getDeviceStatus().dBm);
    }));
    add(new SCPICommand("PORT", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval) || newval > DeviceDriver::getInfo(window->getDevice()).Limits.Generator.ports) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            central->setPort(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(central->getDeviceStatus().port);
    }));
}
