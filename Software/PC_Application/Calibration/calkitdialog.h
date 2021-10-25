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

private:
    void parseEntries();
    void updateEntries();
    Ui::CalkitDialog *ui;

    bool open_ok, short_ok, load_ok, through_ok;

    Calkit ownKit;
    Calkit &editKit;
};

#endif // CALKITDIALOG_H
