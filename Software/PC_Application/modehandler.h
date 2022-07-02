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
    void createMode(QString name, Mode::Type t);
    void closeMode(int index);
    void closeModes();
    int getCurrentIndex();

    Mode* getMode(int index);
    std::vector<Mode*> getModes();

    bool nameAllowed(const QString &name);
    Mode* findFirstOfType(Mode::Type t);

    void setAveragingMode(Averaging::Mode m);

signals:
    void StatusBarMessageChanged(const QString &msg);

    void ModeCreated(int modeIndex);
    void ModeClosed(int modeIndex);

public slots:
    void setCurrentIndex(int modeIndex);

private:
    std::vector<Mode*> modes;
    int currentModeIndex;
    void createMode(Mode *mode);
    AppWindow *aw;

private slots:
    void setStatusBarMessageChanged(const QString &msg);
};

#endif // MODEHANDLER_H
