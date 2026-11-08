#ifndef TDRLOWPASS_H
#define TDRLOWPASS_H

#include "tracemath.h"
#include "windowfunction.h"

#include <QThread>
#include <QSemaphore>

namespace Math {

class TDR;

class TDRThread : public QThread
{
    Q_OBJECT
public:
    TDRThread(TDR &tdr);
    ~TDRThread(){}
private:
    void run() override;
    TDR &tdr;
};

class TDR : public TraceMath
{
    friend class TDRThread;
    Q_OBJECT
public:
    TDR();
    ~TDR();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::TDR;}

    enum class Mode {
        Lowpass,
        Bandpass,
    };
    void setMode(Mode m);
    Mode getMode() const;
    const WindowFunction& getWindow() const;

    unsigned int getUnpaddedInputSize() const;

    unsigned int getDCExtrapolationPoints() const;
    void setDCExtrapolationPoints(unsigned int points);
    // returns the last calculated (extrapolated or manual) DC value
    std::complex<double> getDCValue();

signals:
    // emitted (from the calculation thread) when a new DC value has been calculated
    void DCValueUpdated();

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private:
    void updateTDR();
    void clearOutput();
    Mode mode;
    WindowFunction window;
    unsigned int padding;
    unsigned int unpaddedInputSize;
    bool stepResponse;
    bool automaticDC;
    std::complex<double> manualDC;
    unsigned int DCExtrapolationPoints;
    std::complex<double> lastDC;
    bool lastDCValid;
    TDRThread *thread;
    bool destructing;
    QSemaphore semphr;
};

}

#endif // TDRBANDPASS_H
