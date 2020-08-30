#ifndef MENUVALUE_H
#define MENUVALUE_H

#include "menuitem.h"
#include <QLabel>
#include <CustomWidgets/siunitedit.h>

class MenuValue : public MenuItem
{
    Q_OBJECT
public:
    MenuValue(QString name, double defaultValue = 0.0, QString unit = QString(), QString prefixes = " ", int precision = 0);

signals:
    void valueChanged(double value);
public slots:
    void setValue(double value);
    // same as setValue, except that no valueChanged signal is emitted
    void setValueQuiet(double value);
    void userSelected() override;
private:
    SIUnitEdit *lvalue;
    const QString name;
};

#endif // MENUVALUE_H
