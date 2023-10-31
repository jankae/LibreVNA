#ifndef TRACEDIFFERENCEGENERATOR_H
#define TRACEDIFFERENCEGENERATOR_H

#include <vector>
#include <functional>

#include <QDebug>

template<typename T>
class TraceDifferenceGenerator {
public:
    TraceDifferenceGenerator(std::function<void(const T&)> changeCallback) :
        nextCallbackIndex(0),
        last{},
        callback(changeCallback)
    {}

    void reset() {
        last.clear();
        nextCallbackIndex = 0;
    }

    void newTrace(const std::vector<T> &trace) {
        if(trace.size() < last.size()) {
            // got less points than last time. This must be a completely new trace, generate callbacks for all points
            for(auto &i : trace) {
                callback(i);
            }
            nextCallbackIndex = 0;
        } else {
            // same amount or more points -> find first difference going backwards from next scheduled update point
            unsigned int lastDiff = nextCallbackIndex;
            do {
                if(lastDiff > 0) {
                    lastDiff--;
                } else {
                    // reached the beginning of the trace
                    if(last.size() == trace.size()) {
                        if(nextCallbackIndex == trace.size()) {
                            // nothing has changed in the whole trace, abort
                            break;
                        } else {
                            // wrap around to the end
                            lastDiff = trace.size() - 1;
                        }
                    } else {
                        // trace must be longer than last -> update all the way to the end
                        lastDiff = trace.size() - 1;
                        for(;nextCallbackIndex!=trace.size();nextCallbackIndex++) {
                            callback(trace[nextCallbackIndex]);
                        }
                        break;
                    }
                }
                if(!(last[lastDiff] == trace[lastDiff])) {
                    do {
                        if(nextCallbackIndex >= trace.size()) {
                            nextCallbackIndex = 0;
                        }
                        callback(trace[nextCallbackIndex]);
                        nextCallbackIndex++;
                    } while(nextCallbackIndex != lastDiff + 1);
                    break;
                }
            } while(lastDiff != nextCallbackIndex);
        }
        last = trace;
//        if(trace.size() > last.size()) {
//            // definitely got more points than last time. Find first point that hasn't been transmitted and generate callbacks for it and all subsequent points
//            unsigned int i=nextCallbackIndex;
//            while(i < trace.size()) {
//                callback(trace[i]);
//                i++;
//            }
//            nextCallbackIndex = trace.size();
//        } else i else {
//            // still the same amount of points.
//            unsigned int i = nextCallbackIndex;
//            unsigned int changedPoints = 0;
//            do {
//                if(i > 0) {
//                    i--;
//                } else {
//                    // reached the end
//                    if(nextCallbackIndex == trace.size()) {
//                        // checked the whole trace
//                        break;
//                    } else {
//                        // last callback was within trace, wrap around
//                        i = trace.size() - 1;
//                    }
//                }
//                bool unchanged = last[i] == trace[i];
//                if(!unchanged) {
//                    changedPoints = (i + trace.size() - nextCallbackIndex + 1);
//                    if(changedPoints > trace.size()) {
//                        changedPoints -= trace.size();
//                    }
//                    break;
//                }
//            } while (i != nextCallbackIndex);
//            i = nextCallbackIndex;
//            while(changedPoints--) {
//                callback(trace[i]);
//                i = (i + 1) % trace.size();
//            }
//            nextCallbackIndex = i;
//        }
//        last = trace;
    }

private:
    unsigned int nextCallbackIndex;
    std::vector<T> last;
    std::function<void(const T&)> callback;
};

#endif // TRACEDIFFERENCEGENERATOR_H
