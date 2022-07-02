#ifndef MODETABWIDGET_H
#define MODETABWIDGET_H

#include <QTabWidget>
#include <QTabBar>

class ModeTabWidget: public QTabWidget
{
    Q_OBJECT
public:
    explicit ModeTabWidget(QWidget* parent = nullptr);
    ~ModeTabWidget() = default;

private:
    QTabBar * tabBar = nullptr;
};

#endif // MODETABWIDGET_H
