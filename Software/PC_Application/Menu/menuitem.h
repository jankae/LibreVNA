#ifndef MENUITEM_H
#define MENUITEM_H

#include <QFrame>

class MenuItem : public QFrame
{
    Q_OBJECT
public:
    MenuItem();

public slots:
    virtual void userSelected(){};

protected:
    void mouseReleaseEvent(QMouseEvent *me) override;

};

#endif // MENUITEM_H
