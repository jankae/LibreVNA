#ifndef MANUALCONTROLDIALOGVD0_H
#define MANUALCONTROLDIALOGVD0_H

#include "librevnadriver.h"

#include <QDialog>
#include <complex>

namespace Ui {
class ManualControlDialogVD0;
}

class ManualControlDialogVD0 : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialogVD0(LibreVNADriver &dev, QWidget *parent = nullptr);
    ~ManualControlDialogVD0();

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
    bool setPortSwitch(int port);
    int getPortSwitch();
    void setLOHighChipEnable(bool enable);
    bool getLOHighChipEnable();
    bool getLOHighLocked();
    void setLOHighFrequency(double f);
    double getLOHighFrequency();
    void setIFHighFrequency(double f);
    double getIFHighFrequency();
    void setLOLowEnable(bool enable);
    bool getLOLowEnable();
    void setLOLowFrequency(double f);
    double getLOLowFrequency();
    void setIFLowFrequency(double f);
    double getIFLowFrequency();
    void setPort1Enable(bool enable);
    bool getPort1Enable();
    void setPort2Enable(bool enable);
    bool getPort2Enable();
    void setRefEnable(bool enable);
    bool getRefEnable();
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

    int getPort1MinADC();
    int getPort1MaxADC();
    double getPort1Magnitude();
    double getPort1Phase();
    std::complex<double> getPort1Referenced();

    int getPort2MinADC();
    int getPort2MaxADC();
    double getPort2Magnitude();
    double getPort2Phase();
    std::complex<double> getPort2Referenced();

    int getRefMinADC();
    int getRefMaxADC();
    double getRefMagnitude();
    double getRefPhase();

public slots:
    void NewStatus(Protocol::ManualStatus status);

private:
    void UpdateDevice();
    Ui::ManualControlDialogVD0 *ui;
    LibreVNADriver &dev;
    std::complex<double> port1referenced;
    std::complex<double> port2referenced;

    std::vector<SCPICommand*> commands;
};

#endif // MANUALCONTROLDIALOGVD0_H
