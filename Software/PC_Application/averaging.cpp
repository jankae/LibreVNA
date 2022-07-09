#include "averaging.h"

using namespace std;

Averaging::Averaging()
{
    averages = 1;
    mode = Mode::Mean;
}

void Averaging::reset(unsigned int points)
{
    avg.clear();
    for(unsigned int i = 0;i<points;i++) {
        avg.push_back(deque<array<complex<double>, 4>>());
    }
}

void Averaging::setAverages(unsigned int a)
{
    averages = a;
    reset(avg.size());
}

VNAData Averaging::process(VNAData d)
{
    auto S11 = d.S.m11;
    auto S12 = d.S.m12;
    auto S21 = d.S.m21;
    auto S22 = d.S.m22;

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

        switch(mode) {
        case Mode::Mean: {
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
            break;
        case Mode::Median: {
            auto size = deque->size();
            // create sorted arrays
            std::vector<complex<double>> S11sorted, S12sorted, S21sorted, S22sorted;
            S11sorted.reserve(size);
            S12sorted.reserve(size);
            S21sorted.reserve(size);
            S22sorted.reserve(size);

            auto comp = [=](const complex<double>&a, const complex<double>&b){
                return abs(a) < abs(b);
            };

            for(auto d : *deque) {
                S11sorted.insert(upper_bound(S11sorted.begin(), S11sorted.end(), d[0], comp), d[0]);
                S12sorted.insert(upper_bound(S12sorted.begin(), S12sorted.end(), d[1], comp), d[1]);
                S21sorted.insert(upper_bound(S21sorted.begin(), S21sorted.end(), d[2], comp), d[2]);
                S22sorted.insert(upper_bound(S22sorted.begin(), S22sorted.end(), d[3], comp), d[3]);
            }
            if(size & 0x01) {
                // odd number of samples
                S11 = S11sorted[size / 2];
                S12 = S12sorted[size / 2];
                S21 = S21sorted[size / 2];
                S22 = S22sorted[size / 2];
            } else {
                // even number, use average of middle samples
                S11 = (S11sorted[size / 2 - 1] + S11sorted[size / 2]) / 2.0;
                S12 = (S12sorted[size / 2 - 1] + S12sorted[size / 2]) / 2.0;
                S21 = (S21sorted[size / 2 - 1] + S21sorted[size / 2]) / 2.0;
                S22 = (S22sorted[size / 2 - 1] + S22sorted[size / 2]) / 2.0;
            }
        }
            break;
        }
    }

    d.S = Sparam(S11, S12, S21, S22);

    return d;
}

Protocol::SpectrumAnalyzerResult Averaging::process(Protocol::SpectrumAnalyzerResult d)
{
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
        array<complex<double>, 4> sample = {d.port1, d.port2, 0, 0};
        deque->push_back(sample);
        if(deque->size() > averages) {
            deque->pop_front();
        }

        switch(mode) {
        case Mode::Mean: {
            // calculate average
            complex<double> sum[2];
            for(auto s : *deque) {
                sum[0] += s[0];
                sum[1] += s[1];
            }
            d.port1 = abs(sum[0] / (double) (deque->size()));
            d.port2 = abs(sum[1] / (double) (deque->size()));
        }
            break;
        case Mode::Median: {
            auto size = deque->size();
            // create sorted arrays
            std::vector<double> port1, port2;
            port1.reserve(size);
            port2.reserve(size);
            for(auto d : *deque) {
                port1.insert(upper_bound(port1.begin(), port1.end(), abs(d[0])), abs(d[0]));
                port2.insert(upper_bound(port2.begin(), port2.end(), abs(d[0])), abs(d[0]));
            }
            if(size & 0x01) {
                // odd number of samples
                d.port1 = port1[size / 2];
                d.port2 = port1[size / 2];
            } else {
                // even number, use average of middle samples
                d.port1 = (port1[size / 2 - 1] + port1[size / 2]) / 2;
                d.port2 = (port2[size / 2 - 1] + port2[size / 2]) / 2;
            }
        }
            break;
        }
    }

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

unsigned int Averaging::currentSweep()
{
    if(avg.size() > 0) {
        return avg.front().size();
    } else {
        return 0;
    }
}

Averaging::Mode Averaging::getMode() const
{
    return mode;
}

void Averaging::setMode(const Mode &value)
{
    mode = value;
}
