#include "utils.h"

uint32_t convert_endianness32(uint32_t val) {
    uint32_t val1 = (val & 0x000000ff) << 24;
    uint32_t val2 = (val & 0x0000ff00) << 8;
    uint32_t val3 = (val & 0x00ff0000) >> 8;
    uint32_t val4 = (val & 0xff000000) >> 24;

    return val1 | val2 | val3 | val4;
}
