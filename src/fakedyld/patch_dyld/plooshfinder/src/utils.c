#include "utils.h"

// the compiler will just use a bswap (optimizations) if possible
// but it's here if it's needed (i.e. there's no intrinsic)
uint32_t convert_endianness32(uint32_t val) {
    uint32_t swapped = 0;
    for (uint32_t i = 0; i < 4; i++) {
        uint32_t sh = i << 3;
        swapped |= ((val >> sh) & 0xff) << (sh ^ 24);
    }

    return swapped;
}