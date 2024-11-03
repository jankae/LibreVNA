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

//QButtonGroup* Mode::modeButtonGroup = nullptr;

Mode::Mode(AppWindow *window, QString name, QString SCPIname)
    : QObject(window),
      SCPINode(SCPIname),
      isActive(false),
      window(window),
      name(name),
      central(nullptr)
{    
    window->getSCPI()->add(this);
}

Mode::~Mode()
{
    window->getSCPI()->remove(this);
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
    isActive = true;
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

    if(window->getDevice()) {
        initializeDevice();
    }

    emit statusbarMessage(statusbarMsg);
}

void Mode::deactivate()
{
    isActive = false;
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
        window->getDevice()->setIdle();
    }
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
    auto filename = QFileDialog::getSaveFileName(nullptr, "Save plot image", "", "PNG image files (*.png)", nullptr, Preferences::QFileDialogOptions());
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

void Mode::setStatusbarMessage(QString msg)
{
    statusbarMsg = msg;
    emit statusbarMessage(msg);
}

QString Mode::getName() const
{
    return name;
}

void Mode::setName(const QString &value)
{
    name = value;
}

void Mode::updateGraphColors()
{
    if ((getType() ==  Type::SA) || getType() ==  Type::VNA) {
        for (auto p : TracePlot::getPlots()) {
            p->updateGraphColors();
        }
    }
}
