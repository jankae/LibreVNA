#include "modewindow.h"

#include "mode.h"
#include "ui_modewindow.h"
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
    ui = new Ui::ModeWindow;
    ui->setupUi(this);

    SetupUi();

    connect(handler, &ModeHandler::ModeCreated, this, &ModeWindow::ModeCreated);
    connect(handler, &ModeHandler::ModeClosed, this, &ModeWindow::ModeClosed);

    connect(ui->tabwidgetModes, &ModeTabWidget::currentChanged, handler, &ModeHandler::setCurrentIndex);
    connect(ui->tabwidgetModes, &ModeTabWidget::tabCloseRequested, handler, &ModeHandler::closeMode);

}

ModeWindow::~ModeWindow()
{
    delete ui;
    ui = nullptr;
}

void ModeWindow::SetupUi()
{
    ui->horizontalLayout->setSpacing(0);
    ui->horizontalLayout->setMargin(0);
    ui->horizontalLayout->setContentsMargins(0,0,0,0);
    ui->tabwidgetModes->setUsesScrollButtons(true);

    auto bAdd = new QPushButton();
    QIcon icon;
    QString iconThemeName = QString::fromUtf8("list-add");

    if (QIcon::hasThemeIcon(iconThemeName))
        icon = QIcon::fromTheme(iconThemeName);
    else
        icon.addFile(QString::fromUtf8(":/icons/add.png"), QSize(), QIcon::Normal, QIcon::Off);

    bAdd->setIcon(icon);
    bAdd->setMaximumHeight(450);
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
    aw->menuBar()->setCornerWidget(bAdd);
}

void ModeWindow::ModeCreated(int modeIndex)
{
    auto mode = handler->getMode(modeIndex);

    if (mode)
    {
        const auto name = mode->getName();
        auto central = mode->getCentral();
        const auto tabIndex = ui->tabwidgetModes->insertTab(modeIndex, central, name);
        ui->tabwidgetModes->setCurrentIndex(tabIndex);
    }
}

void ModeWindow::ModeClosed(int modeIndex)
{
    auto modeWidget = ui->tabwidgetModes->widget(modeIndex);
    ui->tabwidgetModes->removeTab(modeIndex);
    delete modeWidget;
}
