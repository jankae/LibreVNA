#ifndef ICZT_H
#define ICZT_H

#include "tracemath.h"
#include "windowfunction.h"

#include <QThread>
#include <QSemaphore>

namespace Math {

class ICZT;

class ICZTThread : public QThread
{
    Q_OBJECT
public:
    ICZTThread(ICZT &iczt);
    ~ICZTThread(){}
private:
    void run() override;
    ICZT &iczt;
};

class ICZT : public TraceMath
{
    friend class ICZTThread;
    Q_OBJECT
public:
    ICZT();
    ~ICZT();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::ICZT;}

    enum class Mode {
        LowpassStep,        // Best for impedance - direct step response
        LowpassImpulse,     // Experimental impulse response
        BandpassImpulse,    // Good for locating reflections
    };
    
    void setMode(Mode m);
    Mode getMode() const;
    const WindowFunction& getWindow() const;
    double getVelocityFactor() const;
    void setVelocityFactor(double vf);

    // For UI configuration
    bool getAutomaticDC() const { return automaticDC; }
    void setAutomaticDC(bool auto_dc);
    std::complex<double> getManualDC() const { return manualDC; }
    void setManualDC(std::complex<double> dc);
    
    double getGammaClip() const { return gammaClip; }
    void setGammaClip(double clip);

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private:
    void updateICZT();
    void clearOutput();
    
    // Algorithm functions
    std::vector<std::complex<double>> calculateBandpassImpulse(
        const std::vector<double>& freq_hz,
        const std::vector<std::complex<double>>& s11,
        const std::vector<double>& t_s
    );
    
    std::vector<std::complex<double>> calculateLowpassImpulse(
        const std::vector<double>& freq_hz,
        const std::vector<std::complex<double>>& s11,
        const std::vector<double>& t_s
    );
    
    std::vector<std::complex<double>> calculateLowpassStep(
        const std::vector<double>& freq_hz,
        const std::vector<std::complex<double>>& s11,
        const std::vector<double>& t_s
    );
    
    // Helper functions
    std::vector<double> makeLowpassWindow(size_t n);
    std::complex<double> estimateDC(const std::vector<std::complex<double>>& s11);
    double checkLinearFrequency(const std::vector<double>& freq_hz);
    
    Mode mode;
    WindowFunction window;
    double velocityFactor;
    bool automaticDC;
    std::complex<double> manualDC;
    double gammaClip;
    
    ICZTThread *thread;
    bool destructing;
    QSemaphore semphr;
    
    // Cache for impedance calculation
    std::vector<double> impedanceData;
};

}

#endif // ICZT_H
