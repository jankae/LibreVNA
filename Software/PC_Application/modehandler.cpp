#include "modehandler.h"

#include "VNA/vna.h"
#include "SpectrumAnalyzer/spectrumanalyzer.h"
#include "Generator/generator.h"
#include "mode.h"
#include "averaging.h"

ModeHandler::ModeHandler(AppWindow *aw):
    QObject(),
    aw(aw)
{}

void ModeHandler::shutdown()
{
    for(auto m : modes) {
        m->shutdown();
    }
}

void ModeHandler::createMode(QString name, Mode::Type t)
{
    auto mode = Mode::createNew(aw, name, t);
    createMode(mode);
}

void ModeHandler::createMode(Mode *mode)
{
    modes.push_back(mode);
    currentModeIndex = int(modes.size());
    connect(mode, &Mode::statusbarMessage, this, &ModeHandler::setStatusBarMessageChanged);
    emit ModeCreated(currentModeIndex - 1);

}

Mode* ModeHandler::getMode(int index)
{
    return modes.at(index);
}

std::vector<Mode*> ModeHandler::getModes()
{
    return modes;
}

void ModeHandler::setCurrentIndex(int index)
{
    if ( (currentModeIndex != index) && (index >= 0)) {
        currentModeIndex = index;
        auto * mode = modes.at(currentModeIndex);
        mode->activate();
    }
}

int ModeHandler::getCurrentIndex()
{
    return currentModeIndex;
}

void ModeHandler::closeMode(int index)
{
    disconnect(modes.at(index), &Mode::statusbarMessage, this, &ModeHandler::setStatusBarMessageChanged);
    delete modes.at(index);
    modes.erase(modes.begin() + index);
    if (currentModeIndex > int(modes.size()) ) {
        setCurrentIndex(currentModeIndex - 1); // Select bar before one deleted
        auto vna = modes.at(currentModeIndex);
        vna->activate();
    }
    emit ModeClosed(index);
}

void ModeHandler::closeModes()
{
    while(modes.size() > 0) {
        closeMode(0);
    }
}

void ModeHandler::setStatusBarMessageChanged(const QString &msg)
{
    emit StatusBarMessageChanged(msg);
}

bool ModeHandler::nameAllowed(const QString &name)
{
    for(auto m : modes) {
        if(m->getName() == name) {
            /* name already taken, no duplicates allowed
             * when importing, name is used as value
             */
            return false;
        }
    }
    return true;
}

Mode* ModeHandler::findFirstOfType(Mode::Type t)
{
    for(auto m : modes) {
        if(m->getType() == t) {
            return m;
        }
    }
    return nullptr;
}

void ModeHandler::setAveragingMode(Averaging::Mode value)
{


    // averaging mode may have changed, update for all relevant modes
    for (auto m : getModes())
    {
        switch (m->getType())
        {
            case Mode::Type::VNA:
                static_cast<VNA*>(m)->setAveragingMode(value);
                break;
            case Mode::Type::SA:
                static_cast<SpectrumAnalyzer*>(m)->setAveragingMode(value);
                break;
            case Mode::Type::SG:
            case Mode::Type::Last:
            default:
                break;
        }
    }

    for(auto m : modes) {
        if (m->getType() == Mode::Type::SA) {
            static_cast<SpectrumAnalyzer*>(m)->setAveragingMode(value);
        }
        else if (m->getType() == Mode::Type::VNA) {
            static_cast<VNA*>(m)->setAveragingMode(value);
        }
    }
}
