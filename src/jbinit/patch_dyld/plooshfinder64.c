#include <stdint.h>
#include <stdbool.h>
#include "plooshfinder64.h"

struct pf_patch64_t pf_construct_patch64(uint64_t matches[], uint64_t masks[], uint32_t count, bool (*callback)(struct pf_patch64_t patch, void *stream)) {
    struct pf_patch64_t patch;

    // construct the patch
    patch.matches = matches;
    patch.masks = masks;
    patch.count = count;
    patch.disabled = false;
    patch.callback = callback;

    return patch;
}

struct pf_patchset64_t pf_construct_patchset64(struct pf_patch64_t *patches, uint32_t count, void (*handler)(void *buf, size_t size, struct pf_patchset64_t patchset)) {
    struct pf_patchset64_t patchset;

    patchset.patches = patches;
    patchset.count = count;
    patchset.handler = handler;

    return patchset;
}

void pf_patchset_emit64(void *buf, size_t size, struct pf_patchset64_t patchset) {
    patchset.handler(buf, size, patchset);
}

void pf_disable_patch64(struct pf_patch64_t patch) {
    patch.disabled = true;
}

void pf_find_maskmatch64(void *buf, size_t size, struct pf_patchset64_t patchset) {
    uint64_t *stream = buf;
    uint64_t uint_count = size >> 2;
    uint32_t insn_match_cnt = 0;
    for (uint64_t i = 0; i < uint_count; i++) {
        for (int p = 0; p < patchset.count; p++) {
            struct pf_patch64_t patch = patchset.patches[p];

            insn_match_cnt = 0;
            if (!patch.disabled) {
                for (int x = 0; x < patch.count; x++) {
                    if ((stream[i + x] & patch.masks[x]) == patch.matches[x]) {
                        insn_match_cnt++;
                    } else {
                        break;
                    }
                }
                
                if (insn_match_cnt == patch.count) {
                    uint64_t *found_stream = stream + i;
                    patch.callback(patch, found_stream);
                }
            }
        }
    }
}

int64_t pf_signextend_64(int64_t val, uint8_t bits) {
    val = (uint64_t) val << (64 - bits);
    val >>= 64 - bits;

    return val;
}