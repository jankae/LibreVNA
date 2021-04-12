#include "informationbox.h"
#include <QCheckBox>
#include <QSettings>
#include <QDebug>

void InformationBox::ShowMessage(QString title, QString message, QString messageID)
{
    // check if the user still wants to see this message
    unsigned int hash;
    if(messageID.isEmpty()) {
        hash = qHash(message);
    } else {
        hash = qHash(messageID);
    }

    QSettings s;
    if(!s.contains(hashToSettingsKey(hash))) {
        auto box = new InformationBox(title, message, hash, nullptr);
        box->show();
    }
}

bool InformationBox::AskQuestion(QString title, QString question, bool defaultAnswer, QString messageID)
{
    // check if the user still wants to see this message
    unsigned int hash;
    if(messageID.isEmpty()) {
        hash = qHash(question);
    } else {
        hash = qHash(messageID);
    }

    QSettings s;
    if(!s.contains(hashToSettingsKey(hash))) {
        auto box = new InformationBox(title, question, hash, nullptr);
        box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = box->exec();
        if(ret == QMessageBox::Yes) {
            return true;
        } else {
            return false;
        }
    } else {
        // don't show this question anymore
        return defaultAnswer;
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
