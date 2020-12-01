#include "tracemath.h"

#include "medianfilter.h"
#include "tdr.h"

TraceMath::TraceMath()
{
    input = nullptr;
    dataType = DataType::Invalid;
    error("Invalid input");
}

TraceMath *TraceMath::createMath(TraceMath::Type type)
{
    switch(type) {
    case Type::MedianFilter:
        return new Math::MedianFilter();
    case Type::TDR:
        return new Math::TDR();
    default:
        return nullptr;
    }
}

TraceMath::TypeInfo TraceMath::getInfo(TraceMath::Type type)
{
    TypeInfo ret = {};
    switch(type) {
    case Type::MedianFilter:
        ret.name = "Median filter";
        ret.explanationWidget = Math::MedianFilter::createExplanationWidget();
        break;
    case Type::TDR:
        ret.name = "TDR";
        ret.explanationWidget = Math::TDR::createExplanationWidget();
        break;
    default:
        break;
    }
    return ret;
}

TraceMath::Data TraceMath::getSample(unsigned int index)
{
    return data.at(index);
}

double TraceMath::getStepResponse(unsigned int index)
{
    if(stepResponse.size() > index) {
        return stepResponse[index];
    } else {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

TraceMath::Data TraceMath::getInterpolatedSample(double x)
{
    Data ret;

    if(data.size() == 0 || x < data.front().x || x > data.back().x) {
        ret.y = std::numeric_limits<std::complex<double>>::quiet_NaN();
        ret.x = std::numeric_limits<double>::quiet_NaN();
    } else {
        auto it = lower_bound(data.begin(), data.end(), x, [](const Data &lhs, const double x) -> bool {
            return lhs.x < x;
        });
        if(it->x == x) {
            ret = *it;
        } else {
            // no exact match, needs to interpolate
            auto high = *it;
            it--;
            auto low = *it;
            double alpha = (x - low.x) / (high.x - low.x);
            ret.y = low.y * (1 - alpha) + high.y * alpha;
            ret.x = x;
        }
    }
    return ret;
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
        connect(input, &TraceMath::outputTypeChanged, this, &TraceMath::inputTypeChanged);
        inputTypeChanged(input->dataType);
    }
}

void TraceMath::inputTypeChanged(TraceMath::DataType type)
{
    auto newType = outputType(type);
    dataType = newType;
    data.clear();
    if(dataType == DataType::Invalid) {
        error("Invalid input data");
        disconnect(input, &TraceMath::outputSamplesChanged, this, &TraceMath::inputSamplesChanged);
        updateStepResponse(false);
    } else {
        connect(input, &TraceMath::outputSamplesChanged, this, &TraceMath::inputSamplesChanged);
        inputSamplesChanged(0, input->data.size());
    }
    emit outputTypeChanged(dataType);
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

void TraceMath::updateStepResponse(bool valid)
{
    if(valid) {
        stepResponse.resize(data.size());
        double accumulate = 0.0;
        for(unsigned int i=0;i<data.size();i++) {
            accumulate += data[i].y.real();
            stepResponse[i] = accumulate;
        }
    } else {
        stepResponse.clear();
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
