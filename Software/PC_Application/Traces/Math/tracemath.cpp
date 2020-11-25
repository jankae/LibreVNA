#include "tracemath.h"

TraceMath::TraceMath()
{
    input = nullptr;
    dataType = DataType::Invalid;
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
        inputDataChanged();
        // connect to input
        connect(input, &TraceMath::outputDataChanged, this, &TraceMath::inputDataChanged);
        connect(input, &TraceMath::outputSampleChanged, this, &TraceMath::inputSampleChanged);
        connect(input, &TraceMath::outputTypeChanged, this, &TraceMath::inputTypeChanged);
    }
}

void TraceMath::inputTypeChanged(TraceMath::DataType type)
{
    auto newType = outputType(type);
    if(newType != dataType) {
        dataType = newType;
        data.clear();
        inputDataChanged();
        emit outputTypeChanged(dataType);
    }
}

TraceMath::DataType TraceMath::getDataType() const
{
    return dataType;
}
