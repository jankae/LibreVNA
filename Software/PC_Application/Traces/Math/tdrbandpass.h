#ifndef TDRBANDPASS_H
#define TDRBANDPASS_H

#include "tracemath.h"

namespace Math {

class TDRBandpass : public TraceMath
{
public:
    TDRBandpass();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;
};

}

#endif // TDRBANDPASS_H
