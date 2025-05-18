#include "modewindow.h"

#include "mode.h"
#include "appwindow.h"
#include "CustomWidgets/informationbox.h"

#include <QInputDialog>
#include <QPushButton>
#include <QMenuBar>
#include <QActionGroup>

ModeWindow::ModeWindow(ModeHandler* handler, AppWindow* aw):
    QWidget(nullptr),
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
    connect(tabBar, &QTabBar::tabMoved, this, [=](int from, int to) {
        std::swap(menuActions[from], menuActions[to]);
        updateMenuActions();
    });
}

ModeWindow::~ModeWindow()
{
}

void ModeWindow::SetupUi()
{
    auto cornerWidget = new QWidget();
    cornerWidget->setLayout(new QHBoxLayout);
    cornerWidget->layout()->setSpacing(0);
    cornerWidget->layout()->setContentsMargins(0,0,0,0);
    cornerWidget->setMaximumHeight(aw->menuBar()->height());

    tabBar = new QTabBar;
    tabBar->setStyleSheet("QTabBar::tab { height: " + QString::number(aw->menuBar()->height()) + "px;}");
    tabBar->setTabsClosable(true);
    cornerWidget->layout()->addWidget(tabBar);
    connect(tabBar, &QTabBar::tabBarDoubleClicked, this, [=](int index) {
        renameMode(index);
    });

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

    auto createNew = [=](Mode::Type type) {
        bool ok;
        QString text = QInputDialog::getText(aw,
                                            "Create new "+Mode::TypeToName(type)+" tab",
                                            "Name:", QLineEdit::Normal,
                                            Mode::TypeToName(type), &ok);
        if(ok) {
            if(!handler->nameAllowed(text)) {
                InformationBox::ShowError("Name collision", "Unable to create tab, " \
                                          "no duplicate names allowed");
            } else {
                auto index = handler->createMode(text, type);
                handler->setCurrentIndex(index);
            }
        }
    };

    menu = new QMenu("Mode");
    menu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(menu, &QMenu::customContextMenuRequested, this, [=](const QPoint &p) {
        auto action = menu->actionAt(p);
        if(menuActions.contains(action)) {
            QMenu contextMenu(tr("Context menu"), this);
            QAction del("Delete", this);
            connect(&del, &QAction::triggered, this, [=](){
                handler->closeMode(menuActions.indexOf(action));
            });
            contextMenu.addAction(&del);
            contextMenu.exec(menu->mapToGlobal(p));
        }
    });
    modeMenuGroup = new QActionGroup(menu);
    menu->addSeparator();
    auto submenuAdd = new QMenu("Create new");
    menu->addMenu(submenuAdd);
    auto rename = new QAction("Rename active mode");
    connect(rename, &QAction::triggered, this, [=](){
        renameMode(handler->getCurrentIndex());
    });
    menu->addAction(rename);

    auto mAdd = new QMenu();
    for(unsigned int i=0;i<(int) Mode::Type::Last;i++) {
        auto type = (Mode::Type) i;
        auto action = new QAction(Mode::TypeToName(type));
        mAdd->addAction(action);
        connect(action, &QAction::triggered, this, [=](){
            createNew(type);
        });
        auto action2 = new QAction(Mode::TypeToName(type));
        submenuAdd->addAction(action2);
        connect(action2, &QAction::triggered, this, [=](){
            createNew(type);
        });
    }
    bAdd->setMenu(mAdd);

    cornerWidget->layout()->addWidget(bAdd);

    aw->menuBar()->setCornerWidget(cornerWidget);
}

void ModeWindow::updateMenuActions()
{
    // remove currently assigned actions from menu
    while(!menu->actions()[0]->isSeparator()) {
        menu->removeAction(menu->actions()[0]);
    }
    // add actions in correct order
    auto before = menu->actions()[0];
    for(auto a : menuActions) {
        menu->insertAction(before, a);
    }
}

QMenu *ModeWindow::getMenu() const
{
    return menu;
}

void ModeWindow::ModeCreated(int modeIndex)
{
    auto mode = handler->getMode(modeIndex);

    if (mode)
    {
        const auto name = mode->getName();

        auto action = new QAction(name, modeMenuGroup);
        action->setCheckable(true);
        action->setChecked(true);
        menuActions.insert(modeIndex, action);
        updateMenuActions();
        connect(action, &QAction::triggered, this, [=](){
             auto index = menuActions.indexOf(action);
             if(index >= 0) {
                 tabBar->setCurrentIndex(index);
             }
        });

        tabBar->blockSignals(true);
        tabBar->insertTab(modeIndex, name);
        tabBar->blockSignals(false);
        tabBar->setMovable(true);
    }
}

void ModeWindow::ModeClosed(int modeIndex)
{
    tabBar->blockSignals(true);
    tabBar->removeTab(modeIndex);
    tabBar->blockSignals(false);

    delete menuActions.takeAt(modeIndex);
    updateMenuActions();
}


void ModeWindow::CurrentModeChanged(int modeIndex)
{
    if (modeIndex != tabBar->currentIndex())
    {
        tabBar->setCurrentIndex(modeIndex);
    }
    menuActions[modeIndex]->setChecked(true);
}

void ModeWindow::renameMode(int modeIndex)
{
    auto mode = handler->getMode(modeIndex);
    auto newName = QInputDialog::getText(this, "Rename", "Enter new name for mode \""+mode->getName()+"\":");
    if(newName.isEmpty()) {
        return;
    }
    if(handler->nameAllowed(newName, modeIndex)) {
        mode->setName(newName);
        tabBar->setTabText(modeIndex, newName);
        menu->actions()[modeIndex]->setText(newName);
    } else {
        InformationBox::ShowError("Error", "Unable to set name. Mode names must be unique.");
    }
}
