#ifndef MODEWINDOW_H
#define MODEWINDOW_H

#include "modehandler.h"

class ModeWindow: public QWidget
{
    Q_OBJECT
public:
    explicit ModeWindow(ModeHandler* handler, AppWindow* aw);
    ~ModeWindow();

private:
    ModeHandler* handler;
    void SetupUi();
    AppWindow* aw;
    QTabBar* tabBar;

private slots:
    void ModeCreated(int modeIndex);
    void ModeClosed(int modeIndex);
    void CurrentModeChanged(int modeIndex);
};

#endif // MODEWINDOW_H
