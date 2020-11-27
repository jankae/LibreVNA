#ifndef TRACEMATH_H
#define TRACEMATH_H

#include <QObject>
#include <vector>
#include <complex>

class TraceMath : public QObject {
    Q_OBJECT
public:
    TraceMath();

    class Data {
    public:
        double x;
        std::complex<double> y;
    };

    enum class DataType {
        Frequency,
        Time,
        Invalid,
    };

    Data getSample(unsigned int index);
    unsigned int numSamples();

    // indicate whether this function produces time or frequency domain data
    virtual DataType outputType(DataType inputType) = 0;
    virtual QString description() = 0;
    virtual void edit(){};

    void removeInput();
    void assignInput(TraceMath *input);

    DataType getDataType() const;
    std::vector<Data>& rData() { return data;};

public slots:
    // some values of the input data have changed, begin/end determine which sample(s) has changed
    virtual void inputSamplesChanged(unsigned int begin, unsigned int end){Q_UNUSED(begin) Q_UNUSED(end)};

    void inputTypeChanged(DataType type);

signals:
    // emit this whenever a sample changed (alternatively, if all samples are about to change, emit outputDataChanged after they have changed)
    void outputSamplesChanged(unsigned int begin, unsigned int end);
    // emit when the output type changed
    void outputTypeChanged(DataType type);

protected:
    std::vector<Data> data;
    TraceMath *input;
    DataType dataType;
};

#endif // TRACEMATH_H
