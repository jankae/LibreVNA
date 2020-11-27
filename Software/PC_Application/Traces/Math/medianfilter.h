#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include "tracemath.h"

namespace Math {

class MedianFilter : public TraceMath
{
public:
    MedianFilter();

    virtual DataType outputType(DataType inputType) override;
    virtual QString description() override;

public slots:
    // a single value of the input data has changed, index determines which sample has changed
    virtual void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private:
    void updateSample(int index);
    int kernelSize;
    enum class Order {
        AbsoluteValue,
        Phase,
        Real,
        Imag,
    } order;
};

}

#endif // MEDIANFILTER_H
