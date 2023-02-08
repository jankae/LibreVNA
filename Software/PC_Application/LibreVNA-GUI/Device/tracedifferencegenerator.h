#ifndef TRACEDIFFERENCEGENERATOR_H
#define TRACEDIFFERENCEGENERATOR_H

#include <vector>
#include <functional>

#include <QDebug>

template<typename T, int minUnchanged>
class TraceDifferenceGenerator {
public:
    TraceDifferenceGenerator(std::function<void(const T&)> changeCallback) :
        last{},
        callback(changeCallback),
        nextCallbackIndex(0)
    {}

    void reset() {
        last.clear();
        nextCallbackIndex = 0;
    }

    void newTrace(const std::vector<T> &trace) {
        if(trace.size() > last.size()) {
            // definitely got more points than last time. Find first point that is hasn't been transmitted and generate callbacks for it and all subsequent points
            unsigned int i=nextCallbackIndex;
            while(i < trace.size()) {
                callback(trace[i]);
                i++;
            }
            nextCallbackIndex = 0;
        } else if(trace.size() < last.size()) {
            // got less points than last time. This must be a completely new trace, generate callbacks for all points
            for(auto &i : trace) {
                callback(i);
            }
            nextCallbackIndex = 0;
        } else {
            // still the same amount of points.
            unsigned int i = nextCallbackIndex;
            unsigned int changedPoints = 0;
            do {
                if(i > 0) {
                    i--;
                } else {
                    i = trace.size() - 1;
                }
                bool unchanged = last[i] == trace[i];
                if(!unchanged) {
                    changedPoints = (i + trace.size() - nextCallbackIndex + 1);
                    if(changedPoints > trace.size()) {
                        changedPoints -= trace.size();
                    }
                    break;
                }
            } while (i != nextCallbackIndex);
            i = nextCallbackIndex;
            while(changedPoints--) {
                callback(trace[i]);
                i = (i + 1) % trace.size();
            }
            nextCallbackIndex = i;
        }
        last = trace;
    }

private:
    unsigned int nextCallbackIndex;
    std::vector<T> last;
    std::function<void(const T&)> callback;
};

#endif // TRACEDIFFERENCEGENERATOR_H
