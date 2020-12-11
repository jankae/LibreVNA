#ifndef DFT_H
#define DFT_H

#include "tracemath.h"
#include "windowfunction.h"

namespace Math {

class DFT : public TraceMath
{
public:
    DFT();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::DFT;};

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private:
    void updateDFT();
    bool automaticDC;
    double DCfreq;
    WindowFunction window;
};

}

#endif // DFT_H
