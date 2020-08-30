#ifndef MENULABEL_H
#define MENULABEL_H

#include "menuitem.h"
#include <QVBoxLayout>
#include <QLabel>

class MenuAction : public MenuItem
{
    Q_OBJECT
public:
    enum class ArrowType {
        None,
        Left,
        Right,
    };

    MenuAction(const QString &l, ArrowType arrow = ArrowType::None);
    void AddSubline(const QString &l);
    void RemoveSubline();
signals:
    void triggered();
public slots:
    void userSelected() override;
private:
    QVBoxLayout layout;
    QLabel *subline;
protected:
    void mouseReleaseEvent(QMouseEvent *me) override;
    void mousePressEvent(QMouseEvent *me) override;
};

#endif // MENULABEL_H
