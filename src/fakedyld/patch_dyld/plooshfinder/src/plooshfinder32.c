#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "plooshfinder32.h"

bool pf_maskmatch32(uint32_t insn, uint32_t match, uint32_t mask) {
    return (insn & mask) == match;
}

void pf_find_maskmatch32(void *buf, size_t size, struct pf_patchset32_t patchset) {
    uint32_t *stream = buf;
    uint64_t uint_count = size >> 2;
    
    for (uint64_t i = 0; i < uint_count; i++) {
        for (int p = 0; p < patchset.count; p++) {
            struct pf_patch32_t *patch = patchset.patches + p;
            if (patch->disabled) continue;

            uint32_t x;
            for (x = 0; x < patch->count; x++) {
                if (!pf_maskmatch32(stream[i + x], patch->matches[x], patch->masks[x])) {
                    break;
                }
            }

            if (x == patch->count) {
                patch->callback(patch, stream + i);
            }
        }
    }
}

int32_t pf_signextend_32(int32_t val, uint8_t bits) {
    val = (uint32_t) val << (32 - bits);
    val >>= 32 - bits;

    return val;
}
