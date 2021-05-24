#ifndef INFORMATIONBOX_H
#define INFORMATIONBOX_H

#include <QMessageBox>

class InformationBox : public QMessageBox
{
    Q_OBJECT
public:
    static void ShowMessage(QString title, QString message, QString messageID = QString(), bool block = false);
    static void ShowMessageBlocking(QString title, QString message, QString messageID = QString());
    static void ShowError(QString title, QString message);
    // Display a dialog with yes/no buttons. Returns true if yes is clicked, false otherwise. If the user has selected to never see this message again, defaultAnswer is returned instead
    static bool AskQuestion(QString title, QString question, bool defaultAnswer, QString messageID = QString());
private:
    InformationBox(QString title, QString message, QMessageBox::Icon icon, unsigned int hash, QWidget *parent);
    ~InformationBox();
    static QString hashToSettingsKey(unsigned int hash);
    unsigned int hash;
};

#endif // INFORMATIONBOX_H
