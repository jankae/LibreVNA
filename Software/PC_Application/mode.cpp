#include "mode.h"

#include "Generator/generator.h"
#include "VNA/vna.h"
#include "SpectrumAnalyzer/spectrumanalyzer.h"
#include "CustomWidgets/informationbox.h"

#include "ui_main.h"

#include <QPushButton>
#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>

std::vector<Mode*> Mode::modes;
Mode* Mode::activeMode = nullptr;
QTabBar* Mode::tabbar = nullptr;
QWidget* Mode::cornerWidget = nullptr;
//QButtonGroup* Mode::modeButtonGroup = nullptr;

Mode::Mode(AppWindow *window, QString name, QString SCPIname)
    : QObject(window),
      SCPINode(SCPIname),
      window(window),
      name(name),
      central(nullptr)
{    
    if(!nameAllowed(name)) {
        throw std::runtime_error("Unable to create mode, name already taken");
    }
    // Create mode switch button
    if(!cornerWidget) {
        // this is the first created mode, initialize corner widget and set this mode as active
        cornerWidget = new QWidget();
        cornerWidget->setLayout(new QHBoxLayout);
        cornerWidget->layout()->setSpacing(0);
        cornerWidget->layout()->setMargin(0);
        cornerWidget->layout()->setContentsMargins(0,0,0,0);
        cornerWidget->setMaximumHeight(window->menuBar()->height());

        tabbar = new QTabBar;
        tabbar->setTabsClosable(true);
        tabbar->setStyleSheet("QTabBar::tab { height: "+QString::number(window->menuBar()->height())+"px;}");
        cornerWidget->layout()->addWidget(tabbar);

        auto bAdd = new QPushButton();
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("list-add");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8(":/icons/add.png"), QSize(), QIcon::Normal, QIcon::Off);
        }
        bAdd->setIcon(icon);

        auto mAdd = new QMenu();
        for(unsigned int i=0;i<(int) Type::Last;i++) {
            auto type = (Type) i;
            auto action = new QAction(TypeToName(type));
            mAdd->addAction(action);
            connect(action, &QAction::triggered, [=](){
                bool ok;
                QString text = QInputDialog::getText(window, "Create new "+TypeToName(type)+" tab",
                                                    "Name:", QLineEdit::Normal,
                                                    TypeToName(type), &ok);
                if(ok) {
                    if(!nameAllowed(text)) {
                        InformationBox::ShowError("Name collision", "Unable to create tab, no duplicate names allowed");
                    } else {
                        auto mode = Mode::createNew(window, text, type);
                        mode->activate();
                    }
                }
            });
        }
        bAdd->setMenu(mAdd);
        bAdd->setMaximumHeight(window->menuBar()->height());
        bAdd->setMaximumWidth(40);
        cornerWidget->layout()->addWidget(bAdd);

        window->menuBar()->setCornerWidget(cornerWidget);

        connect(tabbar, &QTabBar::currentChanged, [=](int index){
            modes[index]->activate();
        });
        connect(tabbar, &QTabBar::tabCloseRequested, [=](int index){
            delete modes[index];
        });
    }
    connect(this, &Mode::statusbarMessage, window, &AppWindow::setModeStatus);
    modes.push_back(this);
    tabbar->blockSignals(true);
    tabbar->insertTab(tabbar->count(), name);
    tabbar->blockSignals(false);
    window->getSCPI()->add(this);
}

Mode::~Mode()
{
    window->getSCPI()->remove(this);
    if(activeMode == this) {
        deactivate();
    }
    auto index = findTabIndex();
    tabbar->blockSignals(true);
    tabbar->removeTab(index);
    tabbar->blockSignals(false);
    modes.erase(modes.begin() + index);
    if(modes.size() > 0) {
        modes[tabbar->currentIndex()]->activate();
    }
    window->getCentral()->removeWidget(central);
    delete central;
    for(auto d : docks) {
        delete d;
    }
    for(auto t : toolbars) {
        delete t;
    }
}

void Mode::activate()
{
    if(activeMode == this) {
        // already active;
        return;
    } else if(activeMode) {
        activeMode->deactivate();
    }

    qDebug() << "Activating mode" << name;
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
    window->restoreState(settings.value("windowState_"+name).toByteArray());

    // restore visibility of toolbars and docks
    for(auto d : docks) {
        bool hidden = settings.value("dock_"+name+"_"+d->windowTitle(), d->isHidden()).toBool();
        if(hidden) {
            d->hide();
        } else {
            d->show();
        }
    }
    for(auto t : toolbars) {
        bool hidden = settings.value("toolbar_"+name+"_"+t->windowTitle(), t->isHidden()).toBool();
        if(hidden) {
            t->hide();
        } else {
            t->show();
        }
    }

    activeMode = this;
    // force activation of correct tab in case the mode switch was done via script/setup load.
    // This will trigger a second activation of this mode in the signal of the tab bar, but since it is
    // already the active mode, this function will just return -> no recursion
    tabbar->setCurrentIndex(findTabIndex());

    if(window->getDevice()) {
        initializeDevice();
    }

    emit statusbarMessage(statusbarMsg);
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

    qDebug() << "Deactivated mode" << name;
    if(window->getDevice()) {
        window->getDevice()->SetIdle();
    }
    activeMode = nullptr;
}

Mode *Mode::getActiveMode()
{
    return activeMode;
}

QString Mode::TypeToName(Mode::Type t)
{
    switch(t) {
    case Type::VNA: return "Vector Network Analyzer";
    case Type::SG: return "Signal Generator";
    case Type::SA: return "Spectrum Analyzer";
    default: return "Invalid";
    }
}

Mode::Type Mode::TypeFromName(QString s)
{
    for(unsigned int i=0;i<(int)Type::Last;i++) {
        if(s == TypeToName((Type) i)) {
            return (Type) i;
        }
    }
    return Type::Last;
}

void Mode::saveSreenshot()
{
    auto filename = QFileDialog::getSaveFileName(nullptr, "Save plot image", "", "PNG image files (*.png)", nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) {
        // aborted selection
        return;
    }
    if(filename.endsWith(".png")) {
        filename.chop(4);
    }
    filename += ".png";
    central->grab().save(filename);
}

Mode *Mode::createNew(AppWindow *window, QString name, Mode::Type t)
{
    switch(t) {
    case Type::VNA: return new VNA(window, name);
    case Type::SG: return new Generator(window, name);
    case Type::SA: return new SpectrumAnalyzer(window, name);
    default: return nullptr;
    }
}

bool Mode::nameAllowed(QString name)
{
    for(auto m : modes) {
        if(m->getName() == name) {
            // name already taken, no duplicates allowed
            return false;
        }
    }
    return true;
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

int Mode::findTabIndex()
{
    auto it = std::find(modes.begin(), modes.end(), this);
    return it - modes.begin();
}

std::vector<Mode *> Mode::getModes()
{
    return modes;
}

Mode *Mode::findFirstOfType(Mode::Type t)
{
    for(auto m : modes) {
        if(m->getType() == t) {
            return m;
        }
    }
    return nullptr;
}

void Mode::setStatusbarMessage(QString msg)
{
    statusbarMsg = msg;
    if(this == activeMode) {
        emit statusbarMessage(msg);
    }
}

QString Mode::getName() const
{
    return name;
}

void Mode::setName(const QString &value)
{
    if(!nameAllowed(value)) {
        // unable to use this name
        return;
    }
    name = value;
    tabbar->setTabText(findTabIndex(), name);
}

void Mode::updateGraphColors()
{
    if ((getType() ==  Type::SA) || getType() ==  Type::VNA) {
        for (auto p : TracePlot::getPlots()) {
            p->updateGraphColors();
        }
    }
}
