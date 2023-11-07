#ifndef MANUALCONTROLDIALOGVFE_H
#define MANUALCONTROLDIALOGVFE_H

#include "librevnadriver.h"

#include <QDialog>
#include <complex>

namespace Ui {
class ManualControlDialogVFE;
}

class ManualControlDialogVFE : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialogVFE(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~ManualControlDialogVFE();

    void setSourceChipEnable(bool enable);
    bool getSourceChipEnable();
    void setSourceRFEnable(bool enable);
    bool getSourceRFEnable();
    bool getSourceLocked();
    void setSourceFrequency(double f);
    double getSourceFrequency();

    void setAttenuator(double att);
    double getAttenuator();
    void setSourceAmplifier1Enable(bool enable);
    bool getSourceAmplifier1Enable();
    void setSourceAmplifier2Enable(bool enable);
    bool getSourceAmplifier2Enable();

    void setLOChipEnable(bool enable);
    bool getLOChipEnable();
    void setLORFEnable(bool enable);
    bool getLORFEnable();
    bool getLOLocked();
    void setLOFrequency(double f);
    double getLOFrequency();
    void setIFFrequency(double f);
    double getIFFrequency();
    void setPortEnable(bool enable);
    bool getPortEnable();
    void setRefEnable(bool enable);
    bool getRefEnable();

    enum class Gain {
        G1 = 0,
        G10 = 1,
        G20 = 2,
        G30 = 3,
        G40 = 4,
        G60 = 5,
        G80 = 6,
        G120 = 7,
        G157 = 8,
        G0_25 = 9,
    };

    void setPortGain(Gain g);
    Gain getPortGain();
    void setRefGain(Gain g);
    Gain getRefGain();

    void setNumSamples(int samples);
    int getNumSamples();

    enum class Window {
        None = 0,
        Kaiser = 1,
        Hann = 2,
        FlatTop = 3
    };

    void setWindow(Window w);
    Window getWindow();

    int getPortMinADC();
    int getPortMaxADC();
    double getPortMagnitude();
    double getPortPhase();
    std::complex<double> getPortReferenced();

    int getRefMinADC();
    int getRefMaxADC();
    double getRefMagnitude();
    double getRefPhase();

public slots:
    void NewStatus(Protocol::ManualStatus status);

private:
    void UpdateDevice();
    Ui::ManualControlDialogVFE *ui;
    LibreVNADriver &dev;
    std::complex<double> portreferenced;
};

#endif // MANUALCONTROLDIALOGVFE_H
