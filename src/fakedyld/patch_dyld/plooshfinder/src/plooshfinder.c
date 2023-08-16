// plooshfinder
// WIP patchfinder
// Made by Ploosh

#include <stdint.h>
#include <fakedyld/fakedyld.h>
#include "formats/macho.h"
#include "plooshfinder.h"
#include "plooshfinder32.h"
#include "plooshfinder64.h"

void *pf_zero_buf;

struct pf_patch_t pf_construct_patch(void *matches, void *masks, uint32_t count, bool (*callback)(struct pf_patch_t *patch, void *stream)) {
    struct pf_patch_t patch;

    // construct the patch
    patch.matches = matches;
    patch.masks = masks;
    patch.disabled = false;
    patch.count = count;
    patch.callback = callback;

    return patch;
}

struct pf_patchset_t pf_construct_patchset(struct pf_patch_t *patches, uint32_t count, void (*handler)(void *buf, size_t size, struct pf_patchset_t patchset)) {
    struct pf_patchset_t patchset;

    patchset.patches = patches;
    patchset.count = count;
    patchset.handler = handler;

    return patchset;
}

void pf_patchset_emit(void *buf, size_t size, struct pf_patchset_t patchset) {
    patchset.handler(buf, size, patchset);
}

void pf_disable_patch(struct pf_patch_t *patch) {
    patch->disabled = true;
}

uint32_t *pf_find_next(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask) {
    uint32_t *find_stream = 0;
    
    for (int i = 0; i < count; i++) {
        if (pf_maskmatch32(stream[i], match, mask)) {
            find_stream = stream + i;
            break;
        }
    }

    return find_stream;
}

uint32_t *pf_find_prev(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask) {
    uint32_t *find_stream = 0;

    for (int neg_count = -count; count > 0; count--) {
        int ind = neg_count + count;
        if (pf_maskmatch32(stream[ind], match, mask)) {
            find_stream = stream + ind;
            break;
        }
    }

    return find_stream;
}

int64_t pf_adrp_offset(uint32_t adrp) {
    if (!pf_maskmatch32(adrp, 0x90000000, 0x9f000000)) {
        printf("%s: is not adrp!\n", __FUNCTION__);
        return 0;
    }
    
    int64_t immhi = (((uint64_t) adrp >> 5) & 0x7ffffULL) << 2;
    uint64_t immlo = ((uint64_t) adrp >> 29) & 0x3ULL;

    return pf_signextend_64((immhi | immlo) << 12, 33);
}

uint32_t *pf_follow_veneer(void *buf, uint32_t *stream) {
    // determine if this function is a veneer
    if (!pf_maskmatch32(stream[0], 0x90000010, 0x9f00001f)) {
        return stream;
    } else if (!pf_maskmatch32(stream[1], 0xf9400210, 0xffc003ff)) {
        return stream;
    } else if (!pf_maskmatch32(stream[2], 0xd61f0200, 0xffffffff)) {
        return stream;
    }

    uint16_t buf_offset = (uint64_t) buf & 0xfff;
    uint64_t unaligned_stream = (uint64_t) stream - buf_offset;
    uint64_t stream_addr = (unaligned_stream & ~0xfffUL) + buf_offset;

    uint64_t adrp_addr = stream_addr + pf_adrp_offset(stream[0]);
    uint32_t ldr_offset = ((stream[1] >> 10) & 0xfff) << 3;
    uint64_t target_addr = *(uint64_t *) (adrp_addr + ldr_offset);

    uint32_t *ptr = macho_va_to_ptr(buf, target_addr);

    if (!ptr) {
        ptr = stream;
    }

    return ptr;
}

uint32_t *pf_follow_branch(void *buf, uint32_t *stream) {
    uint32_t branch = stream[0];
    uint8_t imm = 0;

    if (pf_maskmatch32(branch, 0x54000000, 0xff000010)) {
        // b.cond
        imm = 19;
    } else if (pf_maskmatch32(branch, 0x14000000, 0x7c000000)) {
        // b / bl
        imm = 26;
    } else {
        printf("%s: is not branch!\n", __FUNCTION__);
        return 0;
    }

    uint32_t *target = stream + pf_signextend_32(branch, imm);

    return pf_follow_veneer(buf, target);
}

void *pf_follow_xref(void *buf, uint32_t *stream) {
    // this is marked as void * so it can be casted to a different type later
    if (!pf_maskmatch32(stream[0], 0x90000000, 0x9f000000)) {
        printf("%s: is not adrp!\n", __FUNCTION__);
        return 0;
    } else if (!pf_maskmatch32(stream[1], 0x91000000, 0xff800000)) {
        printf("%s: is not add!\n", __FUNCTION__);
        return 0;
    }

    int64_t adrp_addr = pf_adrp_offset(stream[0]);
    uint32_t add_offset = (stream[1] >> 10) & 0xfff;

    uint64_t stream_va = macho_ptr_to_va(buf, stream);
    uint64_t stream_addr = stream_va & ~0xfffUL;
    uint64_t followed_addr = stream_addr + adrp_addr + add_offset;

    return macho_va_to_ptr(buf, followed_addr);
}

bool pf_set_zero_buf(struct pf_patch_t *patch, uint32_t *stream) {
    pf_zero_buf = stream;

    pf_disable_patch(patch);
    return true;
}

void *pf_find_zero_buf(void *buf, size_t size, size_t shc_count) {
    pf_zero_buf = NULL;

    uint32_t matches[shc_count];
    uint32_t masks[shc_count];

    for (size_t i = 0; i < shc_count; i++) {
        matches[i] = 0;
        masks[i] = 0xffffffff;
    }

    struct pf_patch_t patch = pf_construct_patch(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) pf_set_zero_buf);

    struct pf_patch_t patches[] = {
        patch
    };

    struct pf_patchset_t patchset = pf_construct_patchset(patches, sizeof(patches) / sizeof(struct pf_patch_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit(buf, size, patchset);

    if (!pf_zero_buf) {
        printf("%s: Unable to find zero buf!\n", __FUNCTION__);
    }
    return pf_zero_buf;
}

uint32_t *fileset_follow_veneer(void *buf, void *kext, uint32_t *stream) {
    // determine if this function is a veneer
    if (!pf_maskmatch32(stream[0], 0x90000010, 0x9f00001f)) {
        return stream;
    } else if (!pf_maskmatch32(stream[1], 0xf9400210, 0xffc003ff)) {
        return stream;
    } else if (!pf_maskmatch32(stream[2], 0xd61f0200, 0xffffffff)) {
        return stream;
    }

    uint16_t buf_offset = (uint64_t) buf & 0xfff;
    uint64_t unaligned_stream = (uint64_t) stream - buf_offset;
    uint64_t stream_addr = (unaligned_stream & ~0xfffUL) + buf_offset;

    uint64_t adrp_addr = stream_addr + pf_adrp_offset(stream[0]);
    uint32_t ldr_offset = ((stream[1] >> 10) & 0xfff) << 3;
    uint64_t target_addr = *(uint64_t *) (adrp_addr + ldr_offset);

    uint32_t *ptr = fileset_va_to_ptr(buf, kext, target_addr);

    if (!ptr) {
        ptr = stream;
    }

    return ptr;
}

uint32_t *fileset_follow_branch(void *buf, void *kext, uint32_t *stream) {
    uint32_t branch = stream[0];
    uint8_t imm = 0;

    if (pf_maskmatch32(branch, 0x54000000, 0xff000010)) {
        // b.cond
        imm = 19;
    } else if (pf_maskmatch32(branch, 0x14000000, 0x7c000000)) {
        // b / bl
        imm = 26;
    } else {
        printf("%s: is not branch!\n", __FUNCTION__);
        return 0;
    }

    uint32_t *target = stream + pf_signextend_32(branch, imm);

    return fileset_follow_veneer(buf, kext, target);
}

void *fileset_follow_xref(void *buf, void *kext, uint32_t *stream) {
    // this is marked as void * so it can be casted to a different type later
    if (!pf_maskmatch32(stream[0], 0x90000000, 0x9f000000)) {
        printf("%s: is not adrp!\n", __FUNCTION__);
        return 0;
    } else if (!pf_maskmatch32(stream[1], 0x91000000, 0xff800000)) {
        printf("%s: is not add!\n", __FUNCTION__);
        return 0;
    }

    int64_t adrp_addr = pf_adrp_offset(stream[0]);
    uint32_t add_offset = (stream[1] >> 10) & 0xfff;

    uint64_t stream_va = fileset_ptr_to_va(buf, kext, stream);
    uint64_t stream_addr = stream_va & ~0xfffUL;
    uint64_t followed_addr = stream_addr + adrp_addr + add_offset;

    return fileset_va_to_ptr(buf, kext, followed_addr);
}
