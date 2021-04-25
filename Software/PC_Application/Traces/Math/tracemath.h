#ifndef TRACEMATH_H
#define TRACEMATH_H

#include <QObject>
#include <vector>
#include <complex>
#include "savable.h"

/*
 * How to implement a new type of math operation:
 * 1. Create your new math operation class by deriving from this class. Put the new class in the namespace
 *      "Math" to avoid name collisions.
 * 2. Implement required virtual functions:
 *      a. outputType(DataType inputType):
 *          Indicates what kind of result your math operation creates, depending on the type of its input.
 *          If the input type is not applicable for your operation (e.g. frequency domain data as input for
 *          a (forward) DFT, you can return DataType::Invalid
 *      b. description():
 *          Return a short, user-readable description how the operation is set up. This will be displayed
 *          in the math edit dialog.
 *      c. edit():
 *          Optional. If your operation has customizable parameters, calling this function should start a
 *          dialog that allows the user to change these parameters.
 *      d. inputSamplesChanged(unsigned int begin, unsigned int end)
 *          This slot gets called whenever the input data has changed. Override it and implement your math
 *          operation in it. Parameters begin and end indicate which input samples have changed: If, for
 *          example, only the 2nd and third input values have changed, they are set like this: begin=1 end=3
 *          CAUTION: the size of the input vector may have changed, check before accessing it.
 *
 *          Emit the signal outputSamplesChanged(unsigned int begin, unsigned int end) after your operation is
 *          finished. Also call either success(), warning() or error() at the end of this slot, depending on
 *          whether the operation succeeded:
 *              success(): everything went well, output data contains valid values
 *              warning(): something might be wrong (e.g. not enough input samples to create meaningful data, ...).
 *                          Provide a hint by passing a short description string
 *              error(): something went wrong (called with wrong type of data, mathematical error, ...).
 *                          Provide a hint by passing a short description string
 *      e. getType(): return the type of the operation
 *      f. toJSON() and fromJSON(). Save/load all internal parameters
 * 3. Add a new type to the Type enum for your operation
 * 4. Extend the createMath(Type type) factory function to create an instance of your operation
 * 5. Add a static function "createExplanationWidget" which returns a QWidget explaining what your operation does.
 *      This will be displayed when the user chooses to add a new math operation.
 * 6. Extend the function getInfo(Type type) to set a name and create the explanation widget for your operation
 */

class Trace;

class TraceMath : public QObject, public Savable {
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

    enum class Status {
        Ok,
        Warning,
        Error,
    };

    enum class Type {
        MedianFilter,
        TDR,
        DFT,
        Expression,
        TimeGate,
        TimeDomainGating,
        // Add new math operations here, do not explicitly assign values and keep the Last entry at the last position
        Last,
    };

    static std::vector<TraceMath*> createMath(Type type);
    class TypeInfo {
    public:
        QString name;
        QWidget *explanationWidget;
    };
    static TypeInfo getInfo(Type type);

    Data getSample(unsigned int index);
    double getStepResponse(unsigned int index);
    Data getInterpolatedSample(double x);
    unsigned int numSamples();

    // indicate whether this function produces time or frequency domain data
    virtual DataType outputType(DataType inputType) = 0;
    virtual QString description() = 0;
    virtual void edit(){};

    void removeInput();
    void assignInput(TraceMath *input);

    DataType getDataType() const;
    std::vector<Data>& rData() { return data;};
    Status getStatus() const;
    QString getStatusDescription() const;
    virtual Type getType() = 0;

    // returns the trace this math operation is attached to
    Trace* root();

    TraceMath *getInput() const;

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
    // call one of these functions in the derived classes after output data has been updated
    void warning(QString warn);
    void error(QString err);
    void success();
    std::vector<Data> data;
    // buffer for time domain step response data. This makes it possible to access an arbitrary sample of the step response without having to
    // integrate the impulse response every time. Call updateStepResponse in your derived class, if step response data is valid after updating
    // data.
    std::vector<double> stepResponse;
    void updateStepResponse(bool valid);
    TraceMath *input;
    DataType dataType;

private:
    Status status;
    QString statusString;
signals:
    void statusChanged();
};

#endif // TRACEMATH_H
