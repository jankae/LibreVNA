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

    void removeInput();
    void assignInput(TraceMath *input);

    DataType getDataType() const;
    std::vector<Data>& rData() { return data;};

public slots:
    // a single value of the input data has changed, index determines which sample has changed
    virtual void inputSampleChanged(unsigned int index){Q_UNUSED(index)};
    // the complete input data has changed (e.g. cleared or all data modified by some operation)
    virtual void inputDataChanged(){};

    void inputTypeChanged(DataType type);

signals:
    // emit this whenever a sample changed (alternatively, if all samples are about to change, emit outputDataChanged after they have changed)
    void outputSampleChanged(unsigned int index);
    void outputDataChanged();
    // emit when the output type changed
    void outputTypeChanged(DataType type);

protected:
    std::vector<Data> data;
    TraceMath *input;
    DataType dataType;
};

#endif // TRACEMATH_H
