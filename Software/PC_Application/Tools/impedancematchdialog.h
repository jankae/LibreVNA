#ifndef IMPEDANCEMATCHDIALOG_H
#define IMPEDANCEMATCHDIALOG_H

#include "Traces/Marker/markermodel.h"

#include <QDialog>

namespace Ui {
class ImpedanceMatchDialog;
}

class ImpedanceMatchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImpedanceMatchDialog(MarkerModel &model, Marker *marker = nullptr, QWidget *parent = nullptr);
    ~ImpedanceMatchDialog();

private slots:
    void on_cSource_currentIndexChanged(int index);
    void calculateMatch();

private:
    double Z0;
    Ui::ImpedanceMatchDialog *ui;
};

#endif // IMPEDANCEMATCHDIALOG_H
