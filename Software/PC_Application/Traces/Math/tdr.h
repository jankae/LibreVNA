#ifndef TDRLOWPASS_H
#define TDRLOWPASS_H

#include "tracemath.h"
#include "windowfunction.h"

namespace Math {

class TDR : public TraceMath
{
public:
    TDR();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::TDR;};

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private:
    void updateTDR();
    enum class Mode {
        Lowpass,
        Bandpass,
    };
    Mode mode;
    WindowFunction window;
    bool stepResponse;
    bool automaticDC;
    std::complex<double> manualDC;
};

}

#endif // TDRBANDPASS_H
