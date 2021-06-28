#include "Util/app_common.h"
#include "about.h"
#include "ui_aboutdialog.h"

About About::instance;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowTitle("About " + qlibrevnaApp->applicationName());
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
    dialog->exec();
}
