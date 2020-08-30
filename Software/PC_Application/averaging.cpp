#include "averaging.h"

using namespace std;

Averaging::Averaging()
{
    averages = 1;
}

void Averaging::reset()
{
    avg.clear();
}

void Averaging::setAverages(unsigned int a)
{
    averages = a;
    reset();
}

Protocol::Datapoint Averaging::process(Protocol::Datapoint d)
{
    auto S11 = complex<double>(d.real_S11, d.imag_S11);
    auto S12 = complex<double>(d.real_S12, d.imag_S12);
    auto S21 = complex<double>(d.real_S21, d.imag_S21);
    auto S22 = complex<double>(d.real_S22, d.imag_S22);

    if (d.pointNum == avg.size()) {
        // add moving average entry
        deque<array<complex<double>, 4>> deque;
        avg.push_back(deque);
    }

    if (d.pointNum < avg.size()) {
        // can compute average
        // get correct queue
        auto deque = &avg[d.pointNum];
        // add newest sample to queue
        array<complex<double>, 4> sample = {S11, S12, S21, S22};
        deque->push_back(sample);
        if(deque->size() > averages) {
            deque->pop_front();
        }

        // calculate average
        complex<double> sum[4];
        for(auto s : *deque) {
            sum[0] += s[0];
            sum[1] += s[1];
            sum[2] += s[2];
            sum[3] += s[3];
        }
        S11 = sum[0] / (double) (deque->size());
        S12 = sum[1] / (double) (deque->size());
        S21 = sum[2] / (double) (deque->size());
        S22 = sum[3] / (double) (deque->size());
    }

    d.real_S11 = S11.real();
    d.imag_S11 = S11.imag();
    d.real_S12 = S12.real();
    d.imag_S12 = S12.imag();
    d.real_S21 = S21.real();
    d.imag_S21 = S21.imag();
    d.real_S22 = S22.real();
    d.imag_S22 = S22.imag();

    return d;
}

unsigned int Averaging::getLevel()
{
    if(avg.size() > 0) {
        return avg.back().size();
    } else {
        return 0;
    }
}
