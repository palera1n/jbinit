#include <stdint.h>
#include <stdbool.h>
#include "plooshfinder32.h"

struct pf_patch32_t pf_construct_patch32(uint32_t matches[], uint32_t masks[], uint32_t count, bool (*callback)(struct pf_patch32_t patch, void *stream)) {
    struct pf_patch32_t patch;

    // construct the patch
    patch.matches = matches;
    patch.masks = masks;
    patch.count = count;
    patch.disabled = false;
    patch.callback = callback;

    return patch;
}

struct pf_patchset32_t pf_construct_patchset32(struct pf_patch32_t *patches, uint32_t count, void (*handler)(void *buf, size_t size, struct pf_patchset32_t patchset)) {
    struct pf_patchset32_t patchset;

    patchset.patches = patches;
    patchset.count = count;
    patchset.handler = handler;

    return patchset;
}

void pf_patchset_emit32(void *buf, size_t size, struct pf_patchset32_t patchset) {
    patchset.handler(buf, size, patchset);
}

void pf_disable_patch32(struct pf_patch32_t patch) {
    patch.disabled = true;
}

bool pf_maskmatch32(uint32_t insn, uint32_t match, uint32_t mask) {
    return (insn & mask) == match;
}

void pf_find_maskmatch32(void *buf, size_t size, struct pf_patchset32_t patchset) {
    uint32_t *stream = buf;
    uint64_t uint_count = size >> 2;
    uint32_t insn_match_cnt = 0;
    for (uint64_t i = 0; i < uint_count; i++) {
        for (int p = 0; p < patchset.count; p++) {
            struct pf_patch32_t patch = patchset.patches[p];

            insn_match_cnt = 0;
            if (!patch.disabled) {
                for (int x = 0; x < patch.count; x++) {
                    if (pf_maskmatch32(stream[i + x], patch.matches[x], patch.masks[x])) {
                        insn_match_cnt++;
                    } else {
                        break;
                    }
                }

                if (insn_match_cnt == patch.count) {
                    uint32_t *found_stream = stream + i;
                    patch.callback(patch, found_stream);
                }
            }
        }
    }
}

int32_t pf_signextend_32(int32_t val, uint8_t bits) {
    val = (uint32_t) val << (32 - bits);
    val >>= 32 - bits;

    return val;
}
