#ifndef TDRLOWPASS_H
#define TDRLOWPASS_H

#include "tracemath.h"

namespace Math {

class TDRLowpass : public TraceMath
{
public:
    TDRLowpass();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;
};

}

#endif // TDRBANDPASS_H
