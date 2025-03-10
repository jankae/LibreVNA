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

    void setSourceChipEnable(bool enable);
    bool getSourceChipEnable();
    void setSourceRFEnable(bool enable);
    bool getSourceRFEnable();
    bool getSourceLocked();
    void setSourceFrequency(double f);
    double getSourceFrequency();

    enum class LPF {
        M81,
        M130,
        M213,
        M345,
        M550,
        M910,
        M1480,
        M2370,
        M3840,
        M6200,
    };

    void setSourceLPF(LPF lpf);
    LPF getSourceLPF();

    void setStepAttenuator(double att);
    double getStepAttenuator();

    void setVariableAttenuator(double voltage);
    double getVariableAttenuator();

    void setAmplifier1Enable(bool enable);
    bool getAmplifier1Enable();

    void setAmplifier2Enable(bool enable);
    bool getAmplifier2Enable();

    enum class Band {
        HF,
        LF,
        LF_20dB,
        LF_40dB,
    };

    void setBand(Band band);
    Band getBand();

    enum class PortSwitch {
        Off,
        Port1,
        Port2,
    };

    void setPortSwitch(PortSwitch port);
    PortSwitch getPortSwitch();

    void setLOChipEnable(bool enable);
    bool getLOChipEnable();
    void setLORFEnable(bool enable);
    bool getLORFEnable();
    bool getLOLocked();
    void setLOFrequency(double f);
    double getLOFrequency();

    enum class LOMode {
        InternalHF,
        InternalLF,
        External,
    };

    void setLOMode(LOMode mode);
    LOMode getLOMode();

    void setLOAmplifierEnable(bool enable);
    bool getLOAmplifierEnable();

    void setDACEnable(bool enable);
    bool getDACEnable();

    void setDACAFrequency(double f);
    double getDACAFrequency();
    void setDACAAmplitude(int a);
    int getDACAAmplitude();

    void setDACBFrequency(double f);
    double getDACBFrequency();
    void setDACBAmplitude(int a);
    int getDACBAmplitude();

private:
    void UpdateDevice();
    Ui::ManualControlDialogVFD *ui;
    LibreVNADriver &dev;

    std::vector<SCPICommand*> commands;
};

#endif // MANUALCONTROLDIALOGV1_H
