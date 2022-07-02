#ifndef MODE_H
#define MODE_H

#include "appwindow.h"
#include "savable.h"
#include "scpi.h"

#include <QString>
#include <QWidget>
#include <QButtonGroup>
#include <QToolBar>
#include <QTabBar>
#include <QDockWidget>
#include <set>

class Mode : public QObject, public Savable, public SCPINode
{
    Q_OBJECT
public:
    enum class Type {
        VNA,
        SG,
        SA,
        Last,
    };

    Mode(AppWindow *window, QString name, QString SCPIname);
    ~Mode();

    virtual void activate(); // derived classes must call Mode::activate before doing anything
    virtual void deactivate(); // derived classes must call Mode::deactivate before returning
    virtual void shutdown(){}; // called when the application is about to exit
    QString getName() const;
    void setName(const QString &value);
    void updateGraphColors();
    static Mode *getActiveMode();
    static QString TypeToName(Type t);
    static Type TypeFromName(QString s);
    virtual Type getType() = 0;

    virtual void initializeDevice() = 0;
    virtual void deviceDisconnected(){};

    virtual void saveSreenshot();

    static Mode *createNew(AppWindow *window, QString name, Type t);

    virtual QWidget *getCentral() const;

signals:
    void statusbarMessage(QString msg);
protected:
    void setStatusbarMessage(QString msg);
    // call once the derived class is fully initialized
    void finalize(QWidget *centralWidget);
    AppWindow *window;
    std::set<QAction*> actions;
    std::set<QToolBar*> toolbars;
    std::set<QDockWidget*> docks;

private:
    static std::vector<Mode*> modes;
    static Mode *activeMode;
//    static QButtonGroup *modeButtonGroup;
    QString name;
    QString statusbarMsg;

    QWidget *central;
};

#endif // MODE_H
