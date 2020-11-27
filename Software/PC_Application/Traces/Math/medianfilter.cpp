#include "medianfilter.h"

using namespace Math;
using namespace std;

MedianFilter::MedianFilter()
{
    kernelSize = 3;
    order = Order::AbsoluteValue;
}

TraceMath::DataType MedianFilter::outputType(TraceMath::DataType inputType)
{
    // domain stays the same
    return inputType;
}

QString MedianFilter::description()
{
    return "Median filter";
}

void MedianFilter::inputSamplesChanged(unsigned int begin, unsigned int end) {
    if(data.size() != input->rData().size()) {
        data.resize(input->rData().size());
    }
    int start = (int) begin - (kernelSize-1)/2;
    unsigned int stop = (int) end + (kernelSize-1)/2;
    if(start < 0) {
        start = 0;
    }
    if(stop >= input->rData().size()) {
        stop = input->rData().size();
    }
    for(unsigned int i=start;i<stop;i++) {
        updateSample(i);
    }
    emit outputSamplesChanged(start, stop);
}

void MedianFilter::updateSample(int index)
{
    vector<complex<double>> values;
    for(int i=index - (kernelSize-1)/2;i<=index+(kernelSize-1)/2;i++) {
        unsigned int inputSample;
        if(i<0) {
            inputSample = 0;
        } else if(i>=(int) input->rData().size()) {
            inputSample = input->rData().size() - 1;
        } else {
            inputSample = i;
        }
        values.push_back(input->rData().at(inputSample).y);
    }
    sort(values.begin(), values.end(), [=](const complex<double>&a, const complex<double>&b){
       switch(order) {
       case Order::AbsoluteValue: return abs(a) > abs(b);
       case Order::Phase: return arg(a) > arg(b);
       case Order::Real: return real(a) > real(b);
       case Order::Imag: return imag(a) > imag(b);
       }
    });
    data.at(index).y = values[(kernelSize-1)/2];
    data.at(index).x = input->rData().at(index).x;
}
