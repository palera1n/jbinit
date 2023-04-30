// plooshfinder
// WIP patchfinder
// Made by Ploosh

#include <jbinit.h>
#include "plooshfinder.h"
#include "plooshfinder32.h"
#include "plooshfinder64.h"

uint32_t *pf_find_next(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask) {
    uint32_t *find_stream = 0;
    for (int i = 0; i < count; i++) {
        if ((stream[i] & mask) == match) {
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
        if ((stream[ind] & mask) == match) {
            find_stream = stream + ind;
            break;
        }
    }
    return find_stream;
}

uint32_t *pf_follow_branch(uint32_t *insn) {
    uint32_t branch = insn[0];
    uint8_t imm = 0;

    if ((branch & 0xff000010) == 0x54000000) {
        // b.cond
        imm = 19;
    } else if ((branch & 0x7c000000) == 0x14000000) {
        // b / bl
        imm = 26;
    } else {
        printf("%s: is not branch!\n", __FUNCTION__);
        return 0;
    }

    uint32_t *target = insn + pf_signextend_32(branch, imm);

    return target;
}

int64_t pf_adrp_offset(uint32_t adrp) {
    if ((adrp & 0x9f000000) != 0x90000000) {
        printf("%s: is not adrp!\n", __FUNCTION__);
        return 0;
    }
    
    uint64_t immhi = (((uint64_t) adrp >> 5) & 0x7ffffULL) << 2;
    uint64_t immlo = ((uint64_t) adrp >> 29) & 0x3ULL;

    return pf_signextend_64((immhi | immlo) << 12, 33);
}

void *pf_follow_xref(uint32_t *stream) {
    // this is marked as void * so it can be casted to a different type later
    if ((stream[0] & 0x9f000000) != 0x90000000) {
        printf("%s: is not adrp!\n", __FUNCTION__);
        return 0;
    } else if ((stream[1] & 0xff800000) != 0x91000000) {
        printf("%s: is not add!\n", __FUNCTION__);
        return 0;
    }

    uint64_t stream_addr = (uint64_t) stream & ~0xfffULL;

    uint64_t adrp_addr = stream_addr + pf_adrp_offset(stream[0]);
    uint32_t add_offset = (stream[1] >> 10) & 0xfff;

    void *xref = (void *)(adrp_addr + add_offset);

    return xref;
}
