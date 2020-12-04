#include "medianfilter.h"
#include "ui_medianfilterdialog.h"
#include "ui_medianexplanationwidget.h"

#include <QMessageBox>

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
    return "Median filter, size "+QString::number(kernelSize)+", sorting: " + orderToString(order);
}

void MedianFilter::edit()
{
    auto d = new QDialog();
    auto ui = new Ui::MedianFilterDialog();
    ui->setupUi(d);
    ui->kernelSize->setValue(kernelSize);
    ui->sortingMethod->setCurrentIndex((int) order);

    connect(ui->kernelSize, qOverload<int>(&QSpinBox::valueChanged), [=](int newval) {
        if((newval & 0x01) == 0) {
            QMessageBox::information(d, "Median filter", "Only odd values are allowed for the kernel size");
            newval++;
        }
        ui->kernelSize->setValue(newval);
        kernelSize = newval;
    });

    connect(ui->sortingMethod, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
        order = (Order) index;
    });
    d->show();
}

QWidget *MedianFilter::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::MedianFilterExplanationWidget;
    ui->setupUi(w);
    return w;
}

nlohmann::json MedianFilter::toJSON()
{
    nlohmann::json j;
    j["kernel"] = kernelSize;
    j["order"] = order;
    return j;
}

void MedianFilter::fromJSON(nlohmann::json j)
{
    kernelSize = j.value("kernel", 3);
    order = j.value("order", Order::AbsoluteValue);
}

void MedianFilter::inputSamplesChanged(unsigned int begin, unsigned int end) {
    if(data.size() != input->rData().size()) {
        data.resize(input->rData().size());
    }
    if(data.size() > 0) {
        auto kernelOffset = (kernelSize-1)/2;
        int start = (int) begin - (int) kernelOffset;
        unsigned int stop = end + kernelOffset;
        if(start < 0) {
            start = 0;
        }
        if(stop > input->rData().size()) {
            stop = input->rData().size();
        }

        auto comp = [=](const complex<double>&a, const complex<double>&b){
           switch(order) {
           case Order::AbsoluteValue: return abs(a) < abs(b);
           case Order::Phase: return arg(a) < arg(b);
           case Order::Real: return real(a) < real(b);
           case Order::Imag: return imag(a) < imag(b);
           default: return false;
           }
        };

        vector<complex<double>> kernel(kernelSize);
        for(unsigned int out=start;out<stop;out++) {
            if(out == (unsigned int) start) {
                // this is the first sample to update, fill initial kernel
                for(unsigned int in=0;in<kernelSize;in++) {
                    unsigned int inputSample;
                    if(kernelOffset > in + out) {
                        inputSample = 0;
                    } else if(in + out >= input->rData().size() + kernelOffset) {
                        inputSample = input->rData().size() - 1;
                    } else {
                        inputSample = in + out - kernelOffset;
                    }
                    auto sample = input->rData().at(inputSample).y;
                    kernel[in] = sample;
                }
                // sort initial kernel
                sort(kernel.begin(), kernel.end(), comp);
            } else {
                // kernel already filled and sorted from last output sample. Only remove the one input sample that
                // is no longer needed for this output and add the one additional input sample
                int toRemove = out - kernelOffset - 1;
                unsigned int toAdd = out + kernelOffset;
                if(toRemove < 0) {
                    toRemove = 0;
                }
                if(toAdd >= input->rData().size()) {
                    toAdd = input->rData().size() - 1;
                }
                auto sampleToRemove = input->rData().at(toRemove).y;
                auto remove_iterator = lower_bound(kernel.begin(), kernel.end(), sampleToRemove, comp);
                kernel.erase(remove_iterator);

                auto sampleToAdd = input->rData().at(toAdd).y;
                // insert sample at correct position in vector
                kernel.insert(upper_bound(kernel.begin(), kernel.end(), sampleToAdd, comp), sampleToAdd);
            }
            data.at(out).y = kernel[kernelOffset];
            data.at(out).x = input->rData().at(out).x;
        }
        emit outputSamplesChanged(start, stop);
        success();
    } else {
        warning("No input data");
    }
}

QString MedianFilter::orderToString(MedianFilter::Order o)
{
    switch(o) {
    case Order::AbsoluteValue: return "Absolute";
    case Order::Phase: return "Phase";
    case Order::Real: return "Real";
    case Order::Imag: return "Imag";
    default: return QString();
    }
}
