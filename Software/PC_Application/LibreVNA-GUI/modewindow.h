#ifndef MODEWINDOW_H
#define MODEWINDOW_H

#include "modehandler.h"

#include <QMenu>

class ModeWindow: public QWidget
{
    Q_OBJECT
public:
    explicit ModeWindow(ModeHandler* handler, AppWindow* aw);
    ~ModeWindow();

    QMenu *getMenu() const;

private:
    ModeHandler* handler;
    void SetupUi();

    void updateMenuActions();
    AppWindow* aw;
    QTabBar* tabBar;
    QMenu *menu;
    QList<QAction*> menuActions;
    QActionGroup *modeMenuGroup;

private slots:
    void ModeCreated(int modeIndex);
    void ModeClosed(int modeIndex);
    void CurrentModeChanged(int modeIndex);
    void renameMode(int modeIndex);
};

#endif // MODEWINDOW_H
