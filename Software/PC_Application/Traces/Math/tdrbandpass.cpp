#include "tdrbandpass.h"

#include "Traces/fftcomplex.h"


#include <QLabel>
using namespace Math;
using namespace std;

TDRBandpass::TDRBandpass()
{

}

TraceMath::DataType TDRBandpass::outputType(TraceMath::DataType inputType)
{
    if(inputType == DataType::Frequency) {
        return DataType::Time;
    } else {
        return DataType::Invalid;
    }
}

QString TDRBandpass::description()
{
    return "TDR (bandpass mode)";
}

void TDRBandpass::edit()
{
    // nothing to do for now
}

QWidget *TDRBandpass::createExplanationWidget()
{
    return new QLabel("Test");
}

void TDRBandpass::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    Q_UNUSED(begin);
    const double PI = 3.141592653589793238463;
    if(input->rData().size() >= 2) {
        // TDR is computationally expensive, only update at the end of sweep
        if(end != input->rData().size()) {
            // not the end, do nothing
            return;
        }
        // create vector for frequency data
        vector<complex<double>> dft(end);
        // window data and perform FFTshift ("zero"-bin expected at index 0 but is in the middle of input data)
        for(unsigned int i=0;i<end;i++) {
            // TODO implement other windows as well
            constexpr double alpha0 = 0.54;
            auto hamming = alpha0 - (1.0 - alpha0) * cos(2 * PI * i / end);
            auto windowed = input->rData()[i].y * hamming;
            // swap upper and lower half
            if(i < end/2) {
                dft[i + (end+1)/2] = windowed;
            } else {
                dft[i - end/2] = windowed;
            }
        }
        // perform IFT operation on data
        Fft::transform(dft, true);

        // calculate sample distance
        auto freqStep = input->rData()[1].x - input->rData()[0].x;
        auto timeStep = 1.0 / (freqStep * end);

        // copy DFT data to output data
        data.resize(dft.size());
        for(unsigned int i=0;i<dft.size();i++) {
            data[i].y = dft[i];
            data[i].x = i *  timeStep;
        }
        emit outputSamplesChanged(0, data.size());
        success();
    } else {
        // not enough input data
        data.clear();
        emit outputSamplesChanged(0, 0);
        warning("Not enough input samples");
    }
}
