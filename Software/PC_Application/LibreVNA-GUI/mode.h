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
    friend class ModeHandler;
public:
    enum class Type {
        VNA,
        SG,
        SA,
        Last,
    };

    Mode(AppWindow *window, QString name, QString SCPIname);
    virtual ~Mode();

    virtual void shutdown(){} // called when the application is about to exit
    QString getName() const;
    void setName(const QString &value);
    void updateGraphColors();
    static QString TypeToName(Type t);
    static Type TypeFromName(QString s);
    virtual Type getType() = 0;

    virtual void resetSettings(){}

    virtual void initializeDevice() = 0;
    virtual void deviceDisconnected(){}

    virtual void saveSreenshot();

    virtual void setAveragingMode(Averaging::Mode mode) = 0;

    virtual void preset() = 0;
    virtual QList<QAction*> getImportOptions() { return {};}
    virtual QList<QAction*> getExportOptions() { return {};}

signals:
    void statusbarMessage(QString msg);
public slots:
    virtual void deviceInfoUpdated() {} // If the mode has settings that depend on the device capabilites, it should update these in here

protected:

    virtual void activate(); // derived classes must call Mode::activate before doing anything
    virtual void deactivate(); // derived classes must call Mode::deactivate before returning
    bool isActive;

    void setStatusbarMessage(QString msg);
    // call once the derived class is fully initialized
    void finalize(QWidget *centralWidget);
    AppWindow *window;
    std::set<QAction*> actions;
    std::set<QToolBar*> toolbars;
    QToolBar *tb_trackgen;
    SIUnitEdit *tgOffset;
    std::set<QDockWidget*> docks;

private:
//    static QButtonGroup *modeButtonGroup;
    QString name;
    QString statusbarMsg;

    QWidget *central;
};

#endif // MODE_H
