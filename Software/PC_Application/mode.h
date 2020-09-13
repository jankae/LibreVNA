#ifndef MODE_H
#define MODE_H

#include <QString>
#include <QWidget>
#include <QButtonGroup>
#include <QToolBar>
#include <QDockWidget>
#include <set>
#include "appwindow.h"

class Mode : public QWidget
{
public:
    Mode(AppWindow *window, QString name);
    virtual void activate();
    virtual void deactivate();
    QString getName() const;
    static Mode *getActiveMode();

    virtual void initializeDevice() = 0;

protected:
    // call once the derived class is fully initialized
    void finalize(QWidget *centralWidget);
    AppWindow *window;
    std::set<QAction*> actions;
    std::set<QToolBar*> toolbars;
    std::set<QDockWidget*> docks;

private:
    static Mode *activeMode;
    static QWidget *cornerWidget;
    static QButtonGroup *modeButtonGroup;
    const QString name;
    QWidget *central;
};

#endif // MODE_H
