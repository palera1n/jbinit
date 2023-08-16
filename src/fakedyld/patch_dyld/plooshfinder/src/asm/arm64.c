#include "asm/arm64.h"

uint32_t arm64_branch(void *caller, void *target, bool link) {
    uint32_t insn = 0x14000000;

    if (link) {
        insn |= 1 << 31;
    }

    uint32_t offset = (target - caller) >> 2;

    // so that we can fit in the imm field
    offset <<= 6;
    offset >>= 6;

    return insn | offset;
}
