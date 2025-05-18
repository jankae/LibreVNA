#ifndef MODEHANDLER_H
#define MODEHANDLER_H

#include "mode.h"
#include "appwindow.h"

#include <vector>
#include <QObject>

class ModeHandler: public QObject
{
    Q_OBJECT
public:
    ModeHandler(AppWindow *window);
    ~ModeHandler() = default;

    void shutdown();
    int createMode(QString name, Mode::Type t);
    void closeMode(int index);
    void currentModeMoved(int from, int to);
    void closeModes();
    int getCurrentIndex();

    Mode* getActiveMode();

    void activate(Mode * mode);
    void deactivate(Mode* mode);

    Mode* getMode(int index);
    std::vector<Mode*> getModes();

    bool nameAllowed(const QString &name, unsigned int ignoreIndex=-1);
    int findIndex(Mode *targetMode);
    Mode* findFirstOfType(Mode::Type t);

signals:
    void StatusBarMessageChanged(const QString &msg);

    void ModeCreated(int modeIndex);
    void ModeClosed(int modeIndex);
    void CurrentModeChanged(int modeIndex);

public slots:
    void setCurrentIndex(int modeIndex);

private:
    std::vector<Mode*> modes;
    int currentModeIndex;
    int addMode(Mode *mode);
    Mode *createNew(AppWindow *window, QString name, Mode::Type t);
    AppWindow *aw;
    Mode *activeMode;

private slots:
    void setStatusBarMessageChanged(const QString &msg);
};

#endif // MODEHANDLER_H
