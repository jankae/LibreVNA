#ifndef MANUALCONTROLDIALOGVE0_H
#define MANUALCONTROLDIALOGVE0_H

#include "librevnadriver.h"

#include <QDialog>
#include <complex>

namespace Ui {
class ManualControlDialogVE0;
}

class ManualControlDialogVE0 : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialogVE0(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~ManualControlDialogVE0();

public slots:
    void NewStatus(Protocol::ManualStatus status);

private:
    void UpdateDevice();
    Ui::ManualControlDialogVE0 *ui;
    LibreVNADriver &dev;
    std::complex<double> port1referenced;
    std::complex<double> port2referenced;

    std::vector<SCPICommand*> commands;
};

#endif // MANUALCONTROLDIALOGVE0_H
