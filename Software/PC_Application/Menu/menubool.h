#ifndef MENUBOOL_H
#define MENUBOOL_H

#include <QObject>
#include "menuitem.h"
#include <QVBoxLayout>
#include "CustomWidgets/toggleswitch.h"

class MenuBool : public MenuItem
{
    Q_OBJECT
public:
    MenuBool(QString name, bool defaultValue = false);

signals:
    void valueChanged(bool value);
public slots:
    void setValue(bool value);
    void userSelected() override;
protected:
    void mouseReleaseEvent(QMouseEvent *me) override;
    void mousePressEvent(QMouseEvent *me) override;
private:
    QVBoxLayout layout;
    ToggleSwitch *sw;
};

#endif // MENUBOOL_H
