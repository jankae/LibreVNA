#ifndef UTILH_H
#define UTILH_H

namespace Util {
    template<typename T> T Scale(T value, T from_low, T from_high, T to_low, T to_high) {
        value -= from_low;
        value *= (to_high - to_low) / (from_high - from_low);
        value += to_low;
        return value;
    }
}

#endif // UTILH_H
