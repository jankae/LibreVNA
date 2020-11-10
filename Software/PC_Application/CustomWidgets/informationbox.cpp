#include "informationbox.h"
#include <QCheckBox>
#include <QSettings>
#include <QDebug>

void InformationBox::ShowMessage(QString title, QString message, QWidget *parent)
{
    // check if the user still wants to see this message
    auto hash = qHash(message);

    QSettings s;
    if(!s.contains(hashToSettingsKey(hash))) {
        auto box = new InformationBox(title, message, hash, parent);
        box->exec();
    }
}

InformationBox::InformationBox(QString title, QString message, unsigned int hash, QWidget *parent)
    : QMessageBox(parent),
      hash(hash)
{
    setWindowTitle(title);
    setText(message);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setIcon(QMessageBox::Information);

    auto cb = new QCheckBox("Do not show this message again");
    setCheckBox(cb);
}

InformationBox::~InformationBox()
{
    auto cb = checkBox();
    if(cb->isChecked()) {
        QSettings s;
        s.setValue(hashToSettingsKey(hash), true);
    }
}

QString InformationBox::hashToSettingsKey(unsigned int hash)
{
    return QString("DoNotShowDialog/") + QString::number(hash);
}
