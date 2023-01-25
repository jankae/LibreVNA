#ifndef FREQUENCYCALDIALOG_H
#define FREQUENCYCALDIALOG_H

#include "librevnadriver.h"

#include <QDialog>

namespace Ui {
class FrequencyCalDialog;
}

class FrequencyCalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FrequencyCalDialog(LibreVNADriver *dev, QWidget *parent = nullptr);
    ~FrequencyCalDialog();

private:
    Ui::FrequencyCalDialog *ui;
    LibreVNADriver *dev;
};

#endif // FREQUENCYCALDIALOG_H
