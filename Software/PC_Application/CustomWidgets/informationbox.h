#ifndef INFORMATIONBOX_H
#define INFORMATIONBOX_H

#include <QMessageBox>

class InformationBox : public QMessageBox
{
    Q_OBJECT
public:
    static void ShowMessage(QString title, QString message, QWidget *parent = nullptr);
private:
    InformationBox(QString title, QString message, unsigned int hash, QWidget *parent);
    ~InformationBox();
    static QString hashToSettingsKey(unsigned int hash);
    unsigned int hash;
};

#endif // INFORMATIONBOX_H
