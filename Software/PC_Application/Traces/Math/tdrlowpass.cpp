#include "tdrlowpass.h"

#include "Traces/fftcomplex.h"


#include <QLabel>
using namespace Math;
using namespace std;

TDRLowpass::TDRLowpass()
{

}

TraceMath::DataType TDRLowpass::outputType(TraceMath::DataType inputType)
{
    if(inputType == DataType::Frequency) {
        return DataType::Time;
    } else {
        return DataType::Invalid;
    }
}

QString TDRLowpass::description()
{
    return "TDR (lowpass mode)";
}

void TDRLowpass::edit()
{
    // nothing to do for now
}

QWidget *TDRLowpass::createExplanationWidget()
{
    return new QLabel("Test");
}

void TDRLowpass::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    Q_UNUSED(begin);
    if(input->rData().size() >= 2) {
        // TDR is computationally expensive, only update at the end of sweep
        if(end != input->rData().size()) {
            // not the end, do nothing
            return;
        }
        auto steps = input->rData().size();
        auto firstStep = input->rData().front().x;
        if(firstStep == 0) {
            // zero as first step would result in infinite number of points, skip and start with second
            firstStep = input->rData()[1].x;
            steps--;
        }
        if(firstStep * steps != input->rData().back().x) {
            // data is not available with correct frequency spacing, calculate required steps
            steps = input->rData().back().x / firstStep;
        }
        const double PI = 3.141592653589793238463;
        // reserve vector for negative frequenies and DC as well
        vector<complex<double>> frequencyDomain(2*steps + 1);
        // copy frequencies, use the flipped conjugate for negative part
        for(unsigned int i = 1;i<=steps;i++) {
            auto S = input->getInterpolatedSample(firstStep * i).y;
            constexpr double alpha0 = 0.54;
            auto hamming = alpha0 - (1.0 - alpha0) * -cos(PI * i / steps);
            S *= hamming;
            frequencyDomain[2 * steps - i + 1] = conj(S);
            frequencyDomain[i] = S;
        }
        // use simple extrapolation from lowest two points to extract DC value
        auto abs_DC = 2.0 * abs(frequencyDomain[1]) - abs(frequencyDomain[2]);
        auto phase_DC = 2.0 * arg(frequencyDomain[1]) - arg(frequencyDomain[2]);
        frequencyDomain[0] = polar(abs_DC, phase_DC);

        auto fft_bins = frequencyDomain.size();
        const double fs = 1.0 / (firstStep * fft_bins);

        Fft::transform(frequencyDomain, true);

        data.clear();
        data.resize(fft_bins);

        for(unsigned int i = 0;i<fft_bins;i++) {
            data[i].x = fs * i;
            data[i].y = frequencyDomain[i] / (double) fft_bins;
        }
        updateStepResponse(true);
        emit outputSamplesChanged(0, data.size());
        success();
    } else {
        // not enough input data
        data.clear();
        updateStepResponse(false);
        emit outputSamplesChanged(0, 0);
        warning("Not enough input samples");
    }
}
