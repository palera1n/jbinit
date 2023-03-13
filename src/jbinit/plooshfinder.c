// WIP patchfinder
// Made by ploosh

#include <plooshfinder.h>
#include <jbinit.h>

void pf_find_maskmatch32(void *buf, size_t size, uint32_t matches[], uint32_t masks[], uint32_t count, bool (*callback)(void* stream)) {
    uint32_t *stream = buf;
    uint64_t uint_count = size >> 2;
    uint32_t insn_match_cnt = 0;
    for (uint64_t i = 0; i < uint_count; i++) {
        insn_match_cnt = 0;
        for (int x = 0; x < count; x++) {
            if ((stream[i + x] & masks[x]) == matches[x]) {
                insn_match_cnt++;
            } else {
                break;
            }
        }
        if (insn_match_cnt == count) {
            uint32_t *found_stream = stream + i;
            callback(found_stream);
        }
    }
}

void pf_find_maskmatch64(void *buf, size_t size, uint64_t matches[], uint64_t masks[], uint32_t count, bool (*callback)(void* stream)) {
    uint64_t *stream = buf;
    uint64_t uint_count = size >> 2;
    uint32_t insn_match_cnt = 0;
    for (uint64_t i = 0; i < uint_count; i++) {
        insn_match_cnt = 0;
        for (int x = 0; x < count; x++) {
            if ((stream[i + x] & masks[x]) == matches[x]) {
                insn_match_cnt++;
            } else {
                break;
            }
        }
        if (insn_match_cnt == count) {
            uint64_t *found_stream = stream + i;
            callback(found_stream);
        }
    }
}

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

int32_t pf_signextend_32(int32_t val, uint8_t bits) {
    val = (uint32_t) val << (32 - bits);
    val >>= 32 - bits;

    return val;
}

int64_t pf_signextend_64(int64_t val, uint8_t bits) {
    val = (uint64_t) val << (64 - bits);
    val >>= 64 - bits;

    return val;
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
    if ((adrp & 0xbf000000) != 0x90000000) {
        printf("%s: is not adrp!\n", __FUNCTION__);
        return 0;
    }
    
    uint64_t immhi = (((uint64_t) adrp >> 5) & 0x7ffffULL) << 2;
    uint64_t immlo = ((uint64_t) adrp >> 29) & 0x3ULL;

    return pf_signextend_64((immhi | immlo) << 12, 33);
}

void *pf_follow_xref(uint32_t *stream) {
    // this is marked as void * so it can be casted to a different type later
    if ((stream[0] & 0xbf000000) != 0x90000000) {
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
