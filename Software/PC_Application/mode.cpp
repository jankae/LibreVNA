#include "mode.h"

#include <QPushButton>
#include <QSettings>
#include "ui_main.h"

Mode* Mode::activeMode = nullptr;
QWidget* Mode::cornerWidget = nullptr;
QButtonGroup* Mode::modeButtonGroup = nullptr;

Mode::Mode(AppWindow *window, QString name)
    : QObject(window),
      window(window),
      name(name),
      central(nullptr)
{
    // Create mode switch button
    auto modeSwitch = new QPushButton(name);
    modeSwitch->setCheckable(true);
    modeSwitch->setMaximumHeight(window->getUi()->menubar->height());
    if(!cornerWidget) {
        // this is the first created mode, initialize corner widget and set this mode as active
        modeSwitch->setChecked(true);
        cornerWidget = new QWidget;
        cornerWidget->setLayout(new QHBoxLayout);
        cornerWidget->layout()->setSpacing(0);
        cornerWidget->layout()->setMargin(0);
        cornerWidget->layout()->setContentsMargins(0,0,0,0);
        window->menuBar()->setCornerWidget(cornerWidget);
        modeButtonGroup = new QButtonGroup;
        window->getUi()->menubar->setMaximumHeight(window->getUi()->menubar->height());
    }
    cornerWidget->layout()->addWidget(modeSwitch);
    modeButtonGroup->addButton(modeSwitch);

    connect(modeSwitch, &QPushButton::clicked, [=](){
        activate();
    });
}

void Mode::activate()
{
    if(activeMode == this) {
        // already active;
        return;
    } else if(activeMode) {
        activeMode->deactivate();
    }

    // show all mode specific GUI elements
    for(auto t : toolbars) {
        t->show();
        window->getUi()->menuToolbars->addAction(t->toggleViewAction());
    }
    for(auto d : docks) {
        d->show();
        window->getUi()->menuDocks->addAction(d->toggleViewAction());
    }
    for(auto a : actions) {
        a->setVisible(true);
    }

    QSettings settings;
    window->getCentral()->setCurrentWidget(central);

    // restore dock and toolbar positions
//    window->restoreGeometry(settings.value("geometry_"+name).toByteArray());
    window->restoreState(settings.value("windowState_"+name).toByteArray());

    // restore visibility of toolbars and docks
//    window->getUi()->menuDocks->clear();
    for(auto d : docks) {
//        window->getUi()->menuDocks->addAction(d->toggleViewAction());
        bool hidden = settings.value("dock_"+name+"_"+d->windowTitle(), d->isHidden()).toBool();
        if(hidden) {
            d->hide();
        } else {
            d->show();
        }
    }
//    window->getUi()->menuToolbars->clear();
    for(auto t : toolbars) {
//        window->getUi()->menuToolbars->addAction(t->toggleViewAction());
        bool hidden = settings.value("toolbar_"+name+"_"+t->windowTitle(), t->isHidden()).toBool();
        if(hidden) {
            t->hide();
        } else {
            t->show();
        }
    }

    activeMode = this;

    if(window->getDevice()) {
        initializeDevice();
    }
}

void Mode::deactivate()
{
    QSettings settings;
    // save dock/toolbar visibility
    for(auto d : docks) {
        settings.setValue("dock_"+name+"_"+d->windowTitle(), d->isHidden());
    }
    for(auto t : toolbars) {
        settings.setValue("toolbar_"+name+"_"+t->windowTitle(), t->isHidden());
    }
//    settings.setValue("geometry_"+name, window->saveGeometry());
    settings.setValue("windowState_"+name, window->saveState());

    // hide all mode specific GUI elements
    for(auto t : toolbars) {
        t->hide();
        window->getUi()->menuToolbars->removeAction(t->toggleViewAction());
    }
    for(auto d : docks) {
        d->hide();
        window->getUi()->menuDocks->removeAction(d->toggleViewAction());
    }
    for(auto a : actions) {
        a->setVisible(false);
    }

    activeMode = nullptr;
}

Mode *Mode::getActiveMode()
{
    return activeMode;
}

void Mode::finalize(QWidget *centralWidget)
{
    central = centralWidget;
    window->getCentral()->addWidget(central);
    // Set ObjectName for toolbars and docks
    for(auto d : docks) {
        d->setObjectName(d->windowTitle()+name);
    }
    for(auto t : toolbars) {
        t->setObjectName(t->windowTitle()+name);
    }
    // hide all mode specific GUI elements
    for(auto t : toolbars) {
        t->hide();
    }
    for(auto d : docks) {
        d->hide();
    }
    for(auto a : actions) {
        a->setVisible(false);
    }
}

QString Mode::getName() const
{
    return name;
}
