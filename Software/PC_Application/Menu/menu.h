#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QStackedLayout>
#include "menuitem.h"
#include "menuaction.h"

class Menu : public QWidget
{
    Q_OBJECT
public:
    Menu(QStackedLayout &layout, QString name = QString());
    void addItem(MenuItem *i);
    void addMenu(Menu *m);
    void finalize();
signals:

protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    void leave();
    class SubmenuEntry {
    public:
        SubmenuEntry(MenuAction *label, Menu *menu, int index) :
            label(label), menu(menu), index(index){};
        MenuAction *label;
        Menu *menu;
        int index;
    };
    static constexpr int maxWidgets = 12;
    QVBoxLayout *m_layout;
    Menu *parent;
    const QString name;
    QStackedLayout &m_containingLayout;
    std::vector<SubmenuEntry> submenus;
    std::vector<MenuItem*> items;
    int m_widgetCount;
};

#endif // MENU_H
