#include "about.h"

#include "Util/app_common.h"
#include "ui_aboutdialog.h"
#include "appwindow.h"

#include <QClipboard>

About About::instance;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowTitle("About " + qlibrevnaApp->applicationName());
    ui->appVersionClipboard->setText("To Clipboard");
    ui->appName->setText(qlibrevnaApp->applicationName());
    ui->appVersion->setText(QString("Version: %1")
        .arg(qlibrevnaApp->applicationVersion()));
    ui->sourceCodeDescription->setText(QString("<a href='%1'>%1</a>")
        .arg("https://github.com/jankae/LibreVNA"));
    ui->sourceCodeDescription->setOpenExternalLinks(true);
    ui->contributeDescription->setOpenExternalLinks(true);
    ui->headerDescription->setOpenExternalLinks(true);
}


AboutDialog::~AboutDialog()
{
    delete ui;
}

void About::about()
{
    auto dialog = new AboutDialog();
    if(AppWindow::showGUI()) {
        dialog->show();
    }
}

void AboutDialog::on_appVersionClipboard_clicked()
{
    QApplication::clipboard()->setText(QString("LibreVNA Version (%1 bit): %2\n"
                                               "OS: %3\n"
                                               "CPU Arch: %4" )
                                       .arg(QSysInfo::WordSize)
                                       .arg(qlibrevnaApp->applicationVersion())
                                       .arg(QSysInfo::prettyProductName())
                                       .arg(QSysInfo::currentCpuArchitecture()));
}

