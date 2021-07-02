#ifndef MANUALCONTROLDIALOG_H
#define MANUALCONTROLDIALOG_H

#include <QDialog>
#include "device.h"
#include <complex>

namespace Ui {
class ManualControlDialog;
}

class ManualControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualControlDialog(Device &dev, QWidget *parent = nullptr);
    ~ManualControlDialog();

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
    Ui::ManualControlDialog *ui;
    Device &dev;
    std::complex<double> port1referenced;
    std::complex<double> port2referenced;
};

#endif // MANUALCONTROLDIALOG_H
