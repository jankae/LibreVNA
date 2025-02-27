#ifndef MANUALCONTROLDIALOGVFD_H
#define MANUALCONTROLDIALOGVFD_H

#include "librevnadriver.h"

#include <QDialog>
#include <complex>

namespace Ui {
class ManualControlDialogVFD;
}

class ManualControlDialogVFD : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialogVFD(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~ManualControlDialogVFD();

    void setHighSourceChipEnable(bool enable);
    bool getHighSourceChipEnable();
    void setHighSourceRFEnable(bool enable);
    bool getHighSourceRFEnable();
    bool getHighSourceLocked();
    bool setHighSourcePower(int dBm);
    int getHighSourcePower();
    void setHighSourceFrequency(double f);
    double getHighSourceFrequency();

    enum class LPF {
        M947,
        M1880,
        M3500,
        None,
    };

    void setHighSourceLPF(LPF lpf);
    LPF getHighSourceLPF();
    void setLowSourceEnable(bool enable);
    bool getLowSourceEnable();
    bool setLowSourcePower(int mA);
    int getLowSourcePower();
    void setLowSourceFrequency(double f);
    double getLowSourceFrequency();
    void setHighband(bool high);
    bool getHighband();
    void setAttenuator(double att);
    double getAttenuator();
    void setAmplifierEnable(bool enable);
    bool getAmplifierEnable();
    bool setPortSwitch(int port);
    int getPortSwitch();
    void setLO1ChipEnable(bool enable);
    bool getLO1ChipEnable();
    void setLO1RFEnable(bool enable);
    bool getLO1RFEnable();
    bool getLO1Locked();
    void setLO1Frequency(double f);
    double getLO1Frequency();
    void setIF1Frequency(double f);
    double getIF1Frequency();
    void setLO2Enable(bool enable);
    bool getLO2Enable();
    void setLO2Frequency(double f);
    double getLO2Frequency();
    void setIF2Frequency(double f);
    double getIF2Frequency();


private:
    void UpdateDevice();
    Ui::ManualControlDialogVFD *ui;
    LibreVNADriver &dev;
};

#endif // MANUALCONTROLDIALOGV1_H
