#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "plooshfinder64.h"

bool pf_maskmatch64(uint64_t insn, uint64_t match, uint64_t mask) {
    return (insn & mask) == match;
}

void pf_find_maskmatch64(void *buf, size_t size, struct pf_patchset64_t patchset) {
    uint64_t *stream = buf;
    uint64_t uint_count = size >> 3;
    
    for (uint64_t i = 0; i < uint_count; i++) {
        for (int p = 0; p < patchset.count; p++) {
            struct pf_patch64_t *patch = patchset.patches + p;
            if (patch->disabled) continue;

            uint32_t x;
            for (x = 0; x < patch->count; x++) {
                if (!pf_maskmatch64(stream[i + x], patch->matches[x], patch->masks[x])) {
                    break;
                }
            }
                
            if (x == patch->count) {
                patch->callback(patch, stream + i);
            }
        }
    }
}

int64_t pf_signextend_64(int64_t val, uint8_t bits) {
    val = (uint64_t) val << (64 - bits);
    val >>= 64 - bits;

    return val;
}
