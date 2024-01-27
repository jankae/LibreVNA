#include "informationbox.h"

#include <QCheckBox>
#include <QSettings>
#include <QDebug>

bool InformationBox::has_gui = true;

void InformationBox::ShowMessage(QString title, QString message, QString messageID, bool block, QWidget *parent)
{
    if(!has_gui) {
        // no gui option active, do not show any messages
        return;
    }

    // check if the user still wants to see this message
    unsigned int hash;
    if(messageID.isEmpty()) {
        hash = qHash(message);
    } else {
        hash = qHash(messageID);
    }

    QSettings s;
    if(!s.contains(hashToSettingsKey(hash))) {
        auto box = new InformationBox(title, message, QMessageBox::Information, hash, parent);
        if(block) {
            box->exec();
        } else {
            box->show();
        }
    }
}

void InformationBox::ShowMessageBlocking(QString title, QString message, QString messageID, QWidget *parent)
{
    ShowMessage(title, message, messageID, true, parent);
}

void InformationBox::ShowError(QString title, QString message, QWidget *parent)
{
    if(!has_gui) {
        // no gui option active, do not show any messages
        return;
    }
    auto box = new InformationBox(title, message, QMessageBox::Information, 0, parent);
    box->show();
}

bool InformationBox::AskQuestion(QString title, QString question, bool defaultAnswer, QString messageID, QWidget *parent)
{
    if(!has_gui) {
        // no gui option active, do not show any messages
        return defaultAnswer;
    }

    // check if the user still wants to see this message
    unsigned int hash;
    if(messageID.isEmpty()) {
        hash = qHash(question);
    } else {
        hash = qHash(messageID);
    }

    QSettings s;
    if(!s.contains(hashToSettingsKey(hash))) {
        auto box = new InformationBox(title, question, QMessageBox::Question, hash, parent);
        box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = box->exec();
        if(ret == QMessageBox::Yes) {
            return true;
        } else {
            return false;
        }
    } else {
        // don't show this question anymore
        return s.value(hashToSettingsKey(hash)).toBool();
    }
}

void InformationBox::setGUI(bool enable)
{
    has_gui = enable;
}

InformationBox::InformationBox(QString title, QString message, Icon icon, unsigned int hash, QWidget *parent)
    : QMessageBox(parent),
      hash(hash)
{
    setWindowTitle(title);
    setText(message);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setModal(true);
    setIcon(icon);

    auto cb = new QCheckBox("Do not show this message again");
    setCheckBox(cb);

    if(hash == 0) {
        cb->setVisible(false);
    }
}

InformationBox::~InformationBox()
{
    auto cb = checkBox();
    if(cb->isChecked()) {
        auto value = true;
        auto clicked = clickedButton();
        if(clicked && QMessageBox::standardButton(clicked) == StandardButton::No) {
            value = false;
        }
        QSettings s;
        s.setValue(hashToSettingsKey(hash), value);
    }
}

QString InformationBox::hashToSettingsKey(unsigned int hash)
{
    return QString("DoNotShowDialog/") + QString::number(hash);
}
