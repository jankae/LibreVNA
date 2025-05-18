#include "modehandler.h"

#include "VNA/vna.h"
#include "SpectrumAnalyzer/spectrumanalyzer.h"
#include "Generator/generator.h"
#include "mode.h"
#include "averaging.h"

ModeHandler::ModeHandler(AppWindow *aw):
    QObject(),
    currentModeIndex(0),
    aw(aw),
    activeMode(nullptr)
{}

void ModeHandler::shutdown()
{
    for(auto m : modes) {
        m->shutdown();
    }
}

int ModeHandler::createMode(QString name, Mode::Type t)
{
    auto mode = createNew(aw, name, t);
    return addMode(mode);
}

int ModeHandler::addMode(Mode *mode)
{
    modes.push_back(mode);
    currentModeIndex = int(modes.size()) - 1;
    connect(mode, &Mode::statusbarMessage, this, &ModeHandler::setStatusBarMessageChanged);
    emit ModeCreated(currentModeIndex);
    return (currentModeIndex);
}

Mode *ModeHandler::createNew(AppWindow *aw, QString name, Mode::Type t)
{
    switch(t) {
    case Mode::Type::VNA: return new VNA(aw, name);
    case Mode::Type::SG: return new Generator(aw, name);
    case Mode::Type::SA: return new SpectrumAnalyzer(aw, name);
    default: return nullptr;
    }
}

Mode* ModeHandler::getActiveMode()
{
    return activeMode;
}

Mode* ModeHandler::getMode(int index)
{
    return modes.at(index);
}

void ModeHandler::activate(Mode * mode)
{
    if (getActiveMode() == mode) {
        // Already active
        return;
    }
    else if (getActiveMode()) {
        deactivate(getActiveMode());
    }
    activeMode = mode;
    mode->activate();
}

void ModeHandler::deactivate(Mode* mode)
{
    if(mode) {
        mode->deactivate();
    }
    activeMode = nullptr;
}

std::vector<Mode*> ModeHandler::getModes()
{
    return modes;
}

void ModeHandler::setCurrentIndex(int index)
{
    if (index >= 0) {
        currentModeIndex = index;
        auto m = getMode(getCurrentIndex());
        activate(m);
        emit CurrentModeChanged(getCurrentIndex());
    }
}

void ModeHandler::currentModeMoved(int from, int to)
{
    auto modeFrom = modes.at(from);
    auto modeTo = modes.at(to);
    modes[from] = modeTo;
    modes[to] = modeFrom;
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

    if (getActiveMode() == modes.at(index)) {
         deactivate(getActiveMode());
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
        // skip active mode unless it is the last remaining mode
        if(activeMode == modes[0] && modes.size() > 1) {
            closeMode(1);
        } else {
            closeMode(0);
        }
    }
}

void ModeHandler::setStatusBarMessageChanged(const QString &msg)
{
   QObject* mode = sender();
   if ( getActiveMode() == mode) {
        emit StatusBarMessageChanged(msg);
    }
}

bool ModeHandler::nameAllowed(const QString &name, unsigned int ignoreIndex)
{
    for(unsigned int i=0;i<modes.size();i++) {
        if(i == ignoreIndex) {
            // ignore possible collision at this index
            continue;
        }
        if(modes[i]->getName() == name) {
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
