#ifndef MANUALCONTROLDIALOGVE1_H
#define MANUALCONTROLDIALOGVE1_H

#include "librevnadriver.h"

#include <QDialog>
#include <complex>

namespace Ui {
class ManualControlDialogVE1;
}

class ManualControlDialogVE1 : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialogVE1(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~ManualControlDialogVE1();

public slots:
    void NewStatus(Protocol::ManualStatus status);

private:
    void UpdateDevice();
    Ui::ManualControlDialogVE1 *ui;
    LibreVNADriver &dev;
    std::complex<double> port1referenced;
    std::complex<double> port2referenced;

    std::vector<SCPICommand*> commands;
};

#endif // MANUALCONTROLDIALOGVE1_H
