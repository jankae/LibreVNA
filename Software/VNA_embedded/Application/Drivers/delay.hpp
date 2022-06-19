#pragma once

#include <stdint.h>

namespace Delay {

void Init();

uint64_t get_us();

void ms(uint32_t t);
void us(uint32_t t);

}
