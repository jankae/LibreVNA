#include "tracemath.h"

TraceMath::TraceMath()
{
    input = nullptr;
    dataType = DataType::Invalid;
    error("Invalid input");
}

TraceMath::Data TraceMath::getSample(unsigned int index)
{
    return data.at(index);
}

unsigned int TraceMath::numSamples()
{
    return data.size();
}

void TraceMath::removeInput()
{
    if(input) {
        // disconnect everything from the input
        disconnect(input, nullptr, this, nullptr);
        input = nullptr;
        data.clear();
        dataType = DataType::Invalid;
        emit outputTypeChanged(dataType);
    }
}

void TraceMath::assignInput(TraceMath *input)
{
    Q_ASSERT(input != nullptr);
    if(input != this->input) {
        removeInput();
        this->input = input;
        inputTypeChanged(input->dataType);
        // do initial calculation
        inputSamplesChanged(0, input->data.size());
        // connect to input
        connect(input, &TraceMath::outputSamplesChanged, this, &TraceMath::inputSamplesChanged);
        connect(input, &TraceMath::outputTypeChanged, this, &TraceMath::inputTypeChanged);
    }
}

void TraceMath::inputTypeChanged(TraceMath::DataType type)
{
    auto newType = outputType(type);
    if(newType != dataType) {
        dataType = newType;
        data.clear();
        inputSamplesChanged(0, input->data.size());
        emit outputTypeChanged(dataType);
        if(dataType == DataType::Invalid) {
            error("Invalid input data");
        }
    }
}

void TraceMath::warning(QString warn)
{
    statusString = warn;
    status = Status::Warning;
    emit statusChanged();
}

void TraceMath::error(QString err)
{
    statusString = err;
    status = Status::Error;
    emit statusChanged();
}

void TraceMath::success()
{
    if(status != Status::Ok) {
        status = Status::Ok;
        emit statusChanged();
    }
}

QString TraceMath::getStatusDescription() const
{
    return statusString;
}

TraceMath::Status TraceMath::getStatus() const
{
    return status;
}

TraceMath::DataType TraceMath::getDataType() const
{
    return dataType;
}
