#pragma once
#include <stdint.h>
namespace Delay {
static inline void Init() {}
static inline uint64_t get_us() { return 0; }
static inline void ms(uint32_t) {}
static inline void us(uint16_t) {}
}
