#include "tracemath.h"

#include "medianfilter.h"
#include "tdr.h"
#include "dft.h"
#include "expression.h"
#include "timegate.h"
#include "Traces/trace.h"
#include "ui_timedomaingatingexplanationwidget.h"

#include <QMutexLocker>

TraceMath::TraceMath()
{
    input = nullptr;
    dataType = DataType::Invalid;
    error("Invalid input");
}

std::vector<TraceMath *> TraceMath::createMath(TraceMath::Type type)
{
    std::vector<TraceMath*> ret;
    switch(type) {
    case Type::MedianFilter:
        ret.push_back(new Math::MedianFilter());
        break;
    case Type::TDR:
        ret.push_back(new Math::TDR());
        break;
    case Type::DFT:
        ret.push_back(new Math::DFT());
        break;
    case Type::Expression:
        ret.push_back(new Math::Expression());
        break;
    case Type::TimeGate:
        ret.push_back(new Math::TimeGate());
        break;
    case Type::TimeDomainGating:
        ret.push_back(new Math::TDR());
        ret.push_back(new Math::TimeGate());
        ret.push_back(new Math::DFT());
        break;
    default:
        break;
    }
    return ret;
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
    case Type::DFT:
        ret.name = "DFT";
        ret.explanationWidget = Math::DFT::createExplanationWidget();
        break;
    case Type::Expression:
        ret.name = "Custom Expression";
        ret.explanationWidget = Math::Expression::createExplanationWidget();
        break;
    case Type::TimeGate:
        ret.name = "Time Gate";
        ret.explanationWidget = Math::TimeGate::createExplanationWidget();
        break;
    case Type::TimeDomainGating: {
        ret.name = "Time Domain Gating";
        ret.explanationWidget = new QWidget();
        auto ui = new Ui::TimeDomainGatingExplanationWidget;
        ui->setupUi(ret.explanationWidget);
        connect(ret.explanationWidget, &QWidget::destroyed, [=](){
            delete ui;
        });
    }
        break;
    default:
        break;
    }
    return ret;
}

TraceMath::Data TraceMath::getSample(unsigned int index)
{
    TraceMath::Data d;
    dataMutex.lock();
    if(index < data.size()) {
        d = data[index];
    }
    dataMutex.unlock();
    return d;
}

double TraceMath::getStepResponse(unsigned int index)
{
    if(stepResponse.size() > index) {
        return stepResponse[index];
    } else {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

double TraceMath::getInterpolatedStepResponse(double x)
{
    QMutexLocker locker(&dataMutex);
    if(stepResponse.size() != data.size()) {
        // make sure all the step response data is available
        return std::numeric_limits<double>::quiet_NaN();
    }

    double ret = std::numeric_limits<double>::quiet_NaN();

    if(data.size() == 0 || x < data.front().x || x > data.back().x) {
        ret = std::numeric_limits<double>::quiet_NaN();
    } else {
        auto it = lower_bound(data.begin(), data.end(), x, [](const Data &lhs, const double x) -> bool {
            return lhs.x < x;
        });
        if(it->x == x || it == data.begin()) {
            ret = stepResponse[it - data.begin()];
        } else {
            // no exact match, needs to interpolate
            unsigned int highIndex = it - data.begin();
            unsigned int lowIndex = highIndex - 1;
            auto high = *it;
            it--;
            auto low = *it;
            double alpha = (x - low.x) / (high.x - low.x);
            ret = stepResponse[lowIndex] * (1 - alpha) + stepResponse[highIndex] * alpha;
        }
    }
    return ret;
}

TraceMath::Data TraceMath::getInterpolatedSample(double x)
{
    Data ret;
    QMutexLocker locker(&dataMutex);
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
    dataMutex.lock();
    auto size = data.size();
    dataMutex.unlock();
    return size;
}

QString TraceMath::dataTypeToString(TraceMath::DataType type)
{
    switch(type) {
    case DataType::Frequency:
        return "Frequency";
    case DataType::Power:
        return "Power";
    case DataType::Time:
        return "Time";
    case DataType::TimeZeroSpan:
        return "Time (Zero Span)";
    default:
        return "Invalid";
    }
}

TraceMath::DataType TraceMath::dataTypeFromString(QString s)
{
    for(unsigned int i=0;i<(int) DataType::Invalid;i++) {
        if(s.compare(dataTypeToString((DataType) i), Qt::CaseInsensitive) == 0) {
            return (DataType) i;
        }
    }
    // not found
    return DataType::Invalid;
}

void TraceMath::removeInput()
{
    if(input) {
        // disconnect everything from the input
        disconnect(input, nullptr, this, nullptr);
        input = nullptr;
        dataMutex.lock();
        data.clear();
        dataMutex.unlock();
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
    dataMutex.lock();
    data.clear();
    dataMutex.unlock();
    if(dataType == DataType::Invalid) {
        error("Invalid input data");
        disconnect(input, &TraceMath::outputSamplesChanged, this, &TraceMath::inputSamplesChanged);
        updateStepResponse(false);
    } else {
        connect(input, &TraceMath::outputSamplesChanged, this, &TraceMath::inputSamplesChanged, Qt::UniqueConnection);
        input->dataMutex.lock();
        auto inputSize = input->data.size();
        input->dataMutex.unlock();
        inputSamplesChanged(0, inputSize);
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

QMutex& TraceMath::mutex()
{
    return dataMutex;
}

void TraceMath::updateStepResponse(bool valid)
{
    QMutexLocker locker(&dataMutex);
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

TraceMath *TraceMath::getInput() const
{
    return input;
}

QString TraceMath::getStatusDescription() const
{
    return statusString;
}

Trace *TraceMath::root()
{
    auto root = this;
    while(root->input) {
        root = root->input;
    }
    return static_cast<Trace*>(root);
}

TraceMath::Status TraceMath::getStatus() const
{
    return status;
}

TraceMath::DataType TraceMath::getDataType() const
{
    return dataType;
}

std::vector<TraceMath::Data> TraceMath::getData()
{
    dataMutex.lock();
    auto ret = data;
    dataMutex.unlock();
    return ret;
}

double TraceMath::minX()
{
    double ret;
    dataMutex.lock();
    if(data.size() > 0) {
        ret = data.front().x;
    } else {
        ret = std::numeric_limits<double>::max();
    }
    dataMutex.unlock();
    return ret;
}

double TraceMath::maxX()
{
    double ret;
    dataMutex.lock();
    if(data.size() > 0) {
        ret = data.back().x;
    } else {
        ret = std::numeric_limits<double>::min();
    }
    dataMutex.unlock();
    return ret;
}
