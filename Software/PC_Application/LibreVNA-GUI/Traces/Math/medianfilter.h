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

    virtual void edit() override;

    static QWidget *createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::MedianFilter;};

public slots:
    // a single value of the input data has changed, index determines which sample has changed
    virtual void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private:
    unsigned int kernelSize;
    enum class Order {
        AbsoluteValue = 0,
        Phase = 1,
        Real = 2,
        Imag = 3,
    } order;
    static QString orderToString(Order o);
};

}

#endif // MEDIANFILTER_H
