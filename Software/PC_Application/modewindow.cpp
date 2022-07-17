#include "modewindow.h"

#include "mode.h"
#include "appwindow.h"
#include "CustomWidgets/informationbox.h"

#include <QInputDialog>
#include <QPushButton>
#include <QMenuBar>

ModeWindow::ModeWindow(ModeHandler* handler, AppWindow* aw, QWidget* parent):
    QWidget(parent),
    handler(handler),
    aw(aw)
{

    SetupUi();

    connect(handler, &ModeHandler::ModeCreated, this, &ModeWindow::ModeCreated);
    connect(handler, &ModeHandler::ModeClosed, this, &ModeWindow::ModeClosed);
    connect(handler, &ModeHandler::CurrentModeChanged, this, &ModeWindow::CurrentModeChanged);

    connect(tabBar, &QTabBar::currentChanged, handler, &ModeHandler::setCurrentIndex);
    connect(tabBar, &QTabBar::tabCloseRequested, handler, &ModeHandler::closeMode);
    connect(tabBar, &QTabBar::tabMoved, handler, &ModeHandler::currentModeMoved);
}

ModeWindow::~ModeWindow()
{
}

void ModeWindow::SetupUi()
{
    auto cornerWidget = new QWidget();
    cornerWidget->setLayout(new QHBoxLayout);
    cornerWidget->layout()->setSpacing(0);
    cornerWidget->layout()->setMargin(0);
    cornerWidget->layout()->setContentsMargins(0,0,0,0);
    cornerWidget->setMaximumHeight(aw->menuBar()->height());

    tabBar = new QTabBar;
    tabBar->setStyleSheet("QTabBar::tab { height: " + QString::number(aw->menuBar()->height()) + "px;}");
    tabBar->setTabsClosable(true);
    cornerWidget->layout()->addWidget(tabBar);

    auto bAdd = new QPushButton();
    QIcon icon;
    QString iconThemeName = QString::fromUtf8("list-add");

    if (QIcon::hasThemeIcon(iconThemeName))
        icon = QIcon::fromTheme(iconThemeName);
    else
        icon.addFile(QString::fromUtf8(":/icons/add.png"), QSize(), QIcon::Normal, QIcon::Off);

    bAdd->setIcon(icon);
    bAdd->setMaximumHeight(aw->menuBar()->height());
    bAdd->setMaximumWidth(40);

    auto mAdd = new QMenu();
    for(unsigned int i=0;i<(int) Mode::Type::Last;i++) {
        auto type = (Mode::Type) i;
        auto action = new QAction(Mode::TypeToName(type));
        mAdd->addAction(action);
        connect(action, &QAction::triggered, [=](){
            bool ok;
            QString text = QInputDialog::getText(this,
                                                "Create new "+Mode::TypeToName(type)+" tab",
                                                "Name:", QLineEdit::Normal,
                                                Mode::TypeToName(type), &ok);
            if(ok) {
                if(!handler->nameAllowed(text)) {
                    InformationBox::ShowError("Name collision", "Unable to create tab, " \
                                              "no duplicate names allowed");
                } else {
                    handler->createMode(text, type);
                }
            }
        });
    }
    bAdd->setMenu(mAdd);

    cornerWidget->layout()->addWidget(bAdd);

    aw->menuBar()->setCornerWidget(cornerWidget);
}

void ModeWindow::ModeCreated(int modeIndex)
{
    auto mode = handler->getMode(modeIndex);

    if (mode)
    {
        const auto name = mode->getName();

        tabBar->blockSignals(true);
        tabBar->insertTab(modeIndex, name);
        tabBar->blockSignals(false);
        tabBar->setMovable(true);
        tabBar->setCurrentIndex(modeIndex);
    }
}

void ModeWindow::ModeClosed(int modeIndex)
{
    tabBar->blockSignals(true);
    tabBar->removeTab(modeIndex);
    tabBar->blockSignals(false);
}


void ModeWindow::CurrentModeChanged(int modeIndex)
{
    if (modeIndex != tabBar->currentIndex())
    {
        tabBar->setCurrentIndex(modeIndex);
    }
}
