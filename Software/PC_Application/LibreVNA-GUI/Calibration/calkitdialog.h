#ifndef CALKITDIALOG_H
#define CALKITDIALOG_H

#include "calkit.h"

#include <QDialog>
#include <QAbstractButton>
#include <iostream>
#include <iomanip>

namespace Ui {
class CalkitDialog;
}

class CalkitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalkitDialog(Calkit &c, QWidget *parent = nullptr);
    ~CalkitDialog();

signals:
    void settingsChanged();
private slots:
    void updateListEditButtons();
private:
    void parseEntries();
    void updateEntries();
    void updateStandardList();
    Ui::CalkitDialog *ui;

    Calkit &kit;
};

#endif // CALKITDIALOG_H
