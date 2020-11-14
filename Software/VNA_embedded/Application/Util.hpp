#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <cstdint>

namespace Util {
/*
 * Returns the frequency f appears to be when sampled with f_s
 */
uint32_t Alias(int64_t f, uint32_t f_s);

}

#endif
