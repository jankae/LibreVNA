#include "prbs.h"

#include <array>
#include <stdexcept>

PRBS::PRBS(unsigned int bits)
{
    this->bits = bits;

    // from https://www.eetimes.com/tutorial-linear-feedback-shift-registers-lfsrs-part-1/
    const std::array<unsigned int, 12> polynoms = {{
        0x00000000,
        0x00000000,
        0x00000003,
        0x00000005,
        0x00000009,
        0x00000012,
        0x00000021,
        0x00000041,
        0x0000008E,
        0x00000108,
        0x00000204,
        0x00000402,
    }};
    if(bits < 2 || bits >= polynoms.size()) {
        throw std::runtime_error("Bit size not supported");
    }
    polynom = polynoms[bits];
    shiftReg = 0xFFFFFFFF;
}

bool PRBS::next()
{
    bool newbit = false;
    unsigned int mask = 0x01;
    for(unsigned int i=0;i<bits;i++) {
        if(polynom & mask & shiftReg) {
            newbit = !newbit;
        }
        mask <<= 1;
    }
    shiftReg = (shiftReg << 1) | (newbit ? 0x01 : 0x00);
    return newbit;
}
