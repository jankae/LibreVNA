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

int ModeHandler::createMode(QString name, Mode::Type t)
{
    auto mode = Mode::createNew(aw, name, t);
    return createMode(mode);
}

int ModeHandler::createMode(Mode *mode)
{
    modes.push_back(mode);
    currentModeIndex = int(modes.size()) - 1;
    connect(mode, &Mode::statusbarMessage, this, &ModeHandler::setStatusBarMessageChanged);

    auto * m = getMode(currentModeIndex);
    m->activate();

    emit ModeCreated(currentModeIndex);
    return (currentModeIndex);
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
    if (index >= 0) {
        currentModeIndex = index;
        auto * m = getMode(getCurrentIndex());
        m->activate();
        emit CurrentModeChanged(getCurrentIndex());
    }
}

void ModeHandler::currentModeMoved(int from, int to)
{
    auto modeFrom = modes.at(from);
    auto modeTo = modes.at(to);
    modes[from] = modeTo;
    modes[to] = modeFrom;
    setCurrentIndex(to);
}

int ModeHandler::getCurrentIndex()
{
    return currentModeIndex;
}

void ModeHandler::closeMode(int index)
{
    disconnect(modes.at(index), &Mode::statusbarMessage, this, &ModeHandler::setStatusBarMessageChanged);

    std::vector<int> idx;
    for(int i=0; i < int(modes.size()); i++)
    {
       idx.push_back(i);
    }

    auto left = std::find(idx.begin(), idx.end(), getCurrentIndex()-1);
    auto right = std::find(idx.begin(), idx.end(), getCurrentIndex()+1);
    auto foundLeft = false;
    auto foundRight = false;

    if ( left != idx.end() )
    {
       foundLeft = true;
    }

    if ( right != idx.end() )
    {
       foundRight = true;
    }

    auto lastIndex = getCurrentIndex();

    if (int(modes.size()) > 0) {
        if (getCurrentIndex() == index)
        {
            if (foundLeft)
            {
                setCurrentIndex(getCurrentIndex()-1);
            }
            else if (foundRight)
            {
                 setCurrentIndex(getCurrentIndex()+1);
            }
        }
    }

    delete modes.at(index);
    modes.erase(modes.begin() + index);

    if (getCurrentIndex() == 1 && lastIndex == 0) {
        setCurrentIndex(0);
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

int ModeHandler::findIndex(Mode *targetMode)
{
    auto it = std::find(modes.begin(), modes.end(), targetMode);
    return it - modes.begin();
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

