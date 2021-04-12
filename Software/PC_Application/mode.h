#ifndef MODE_H
#define MODE_H

#include <QString>
#include <QWidget>
#include <QButtonGroup>
#include <QToolBar>
#include <QDockWidget>
#include <set>
#include "appwindow.h"
#include "savable.h"

class Mode : public QObject, public Savable
{
public:
    Mode(AppWindow *window, QString name);

    virtual void activate(); // derived classes must call Mode::activate before doing anything
    virtual void deactivate(); // derived classes must call Mode::deactivate before returning
    virtual void shutdown(){}; // called when the application is about to exit
    QString getName() const;
    static Mode *getActiveMode();

    virtual void initializeDevice() = 0;
    virtual void deviceDisconnected(){};

    virtual void saveSreenshot();

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
