#ifndef MODEWINDOW_H
#define MODEWINDOW_H

#include "modehandler.h"

#include <QMenu>

class QPushButton;

class ModeWindow: public QWidget
{
    Q_OBJECT
public:
    explicit ModeWindow(ModeHandler* handler, AppWindow* aw);
    ~ModeWindow();

    QMenu *getMenu() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ModeHandler* handler;
    void SetupUi();
    void updateTabBarHeight();

    void updateMenuActions();
    AppWindow* aw;
    QTabBar* tabBar;
    QWidget* cornerWidget;
    QPushButton* bAdd;
    int lastTabBarHeight = 0;
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
