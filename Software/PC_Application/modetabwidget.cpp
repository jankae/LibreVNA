#include "modetabwidget.h"

#include <QBoxLayout>
#include <QPushButton>

ModeTabWidget::ModeTabWidget(QWidget* parent):
    QTabWidget(parent)
{
    tabBar = new QTabBar;
    tabBar->setStyleSheet("QTabBar::tab { height: " + QString::number(parent->height()) + "px;}");
    this->setTabBar(tabBar);
    this->setTabsClosable(true);
    this->setMovable(true);
}
