#include "averaging.h"

using namespace std;

Averaging::Averaging()
{
    averages = 1;
    numMeasurements = 0;
    mode = Mode::Mean;
}

void Averaging::reset(unsigned int points)
{
    avg.clear();
    for(unsigned int i = 0;i<points;i++) {
        avg.push_back(deque<vector<complex<double>>>());
    }
}

void Averaging::setAverages(unsigned int a)
{
    averages = a;
    // throw away additional stored data if averaging has been reduced
    for(auto &d : avg) {
        while(d.size() > averages) {
            d.pop_front();
        }
    }
}

DeviceDriver::VNAMeasurement Averaging::process(DeviceDriver::VNAMeasurement d)
{
    if(d.measurements.size() != numMeasurements) {
        numMeasurements = d.measurements.size();
        reset(avg.size());
    }

    vector<complex<double>> data;
    for(auto m : d.measurements) {
        data.push_back(m.second);
    }
    process(d.pointNum, data);
    int i=0;
    for(auto &m : d.measurements) {
        m.second = data[i++];
    }
    return d;
}

DeviceDriver::SAMeasurement Averaging::process(DeviceDriver::SAMeasurement d)
{
    if(d.measurements.size() != numMeasurements) {
        numMeasurements = d.measurements.size();
        reset(avg.size());
    }

    vector<complex<double>> data;
    for(auto m : d.measurements) {
        data.push_back(m.second);
    }
    process(d.pointNum, data);
    int i=0;
    for(auto &m : d.measurements) {
        m.second = data[i++].real();
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

bool Averaging::settled()
{
    return getLevel() == averages;
}

Averaging::Mode Averaging::getMode() const
{
    return mode;
}

void Averaging::setMode(const Mode &value)
{
    mode = value;
}

void Averaging::process(unsigned int pointNum, std::vector<std::complex<double>> &data)
{
    if(data.size() != numMeasurements) {
        numMeasurements = data.size();
        reset(avg.size());
    }

    if (pointNum == avg.size()) {
        // add moving average entry
        deque<vector<complex<double>>> deque;
        avg.push_back(deque);
    }

    if (pointNum < avg.size()) {
        // can compute average
        // get correct queue
        auto deque = &avg[pointNum];
        // add newest sample to queue
        deque->push_back(data);
        // remove oldest sample if required number of averages reached
        while(deque->size() > averages) {
            deque->pop_front();
        }

        std::deque<complex<double>> averagedResults = {};

        switch(mode) {
        case Mode::Mean: {
            // calculate average
            complex<double> sum[numMeasurements];
            for(auto s : *deque) {
                for(unsigned int i=0;i<numMeasurements;i++) {
                    sum[i] += s[i];
                }
            }
            for(auto s : sum) {
                averagedResults.push_back(s / (double) (deque->size()));
            }
        }
            break;
        case Mode::Median: {
            auto size = deque->size();
            // create sorted vectors
            vector<vector<complex<double>>> vectors(numMeasurements);

            auto comp = [=](const complex<double>&a, const complex<double>&b){
                return abs(a) < abs(b);
            };
            for(auto d : *deque) {
                int i=0;
                for(auto &v : vectors) {
                    v.insert(upper_bound(v.begin(), v.end(), d[i], comp), d[i]);
                    i++;
                }
            }
            if(size & 0x01) {
                // odd number of samples
                for(auto v : vectors) {
                    averagedResults.push_back(v[size / 2]);
                }
            } else {
                // even number, use average of middle samples
                for(auto v : vectors) {
                    averagedResults.push_back((v[size / 2 - 1] + v[size / 2]) / 2.0);
                }
            }
        }
            break;
        }

        for(auto &m : data) {
            m = averagedResults.front();
            averagedResults.pop_front();
        }
    }
}
