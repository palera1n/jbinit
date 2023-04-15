#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <stdlib.h>
#endif

#include "plooshfinder.h"
#include "ios16_shc.h"

int _internal16_platform = 0;
void *_internal16_rbuf;

bool platform_check_callback16(struct pf_patch32_t patch, uint32_t *stream) {
    stream[2] = 0x52800001 | (_internal16_platform << 5);

    LOG("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (_internal16_platform << 5));

    return true;
}

bool platform_check_callback16_alt(struct pf_patch32_t patch, uint32_t *stream) {
    stream[1] = 0x52800008 | (_internal16_platform << 5);

    LOG("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800008 | (_internal16_platform << 5));

    return true;
}

bool platform_check_callback16_bv(struct pf_patch32_t patch, uint32_t *stream) {
    uint32_t orig_ldp = stream[1];
    char target_reg = orig_ldp & 0x1f;

    uint32_t *shc_loc = get_shc_region(_internal16_rbuf);
    copy_shc(_internal16_platform, target_reg);

    if (!shc_loc) {
        return false;
    }

    stream[1] = 0x94000000 | (shc_loc - stream - 1); // branch to our shellcode to determine if we should change platform or leave it

    // assemble the new ldp
    // we have to extract a lot of things for this to work properly
    char ldr_reg = stream[2] & 0x1f;
    char ldp_reg1 = (orig_ldp >> 5) & 0x1f;
    char ldp_reg2 = (orig_ldp >> 10) & 0x1f;
    char ldp_imm = (orig_ldp >> 13) & 0x7f;
    char new_imm = ldp_imm + 4;

    uint32_t insert_imm = (new_imm >> 2) << 15;
    uint32_t insert_reg1 = ldp_reg1 << 5;
    uint32_t insert_reg2 = ldp_reg2;
    uint32_t insert_reg3 = ldr_reg << 10;

    uint32_t new_ldp = 0x29400000 | insert_imm | insert_reg1 | insert_reg2 | insert_reg3;

    stream[2] = new_ldp;

    LOG("%s: Patched platform check (shc b: 0x%x, ldp: 0x%x)\n", __FUNCTION__, 0x94000000 | (shc_loc - stream - 1), new_ldp);

    return true;
}

void patch_platform_check16(void *real_buf, void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal16_platform = platform;
    _internal16_rbuf = real_buf;

    // r2: /x 00008052000080520000801a00000014:0000c0ff0000c0ff00fce0ff000000fc
    uint32_t matches[] = {
        0x52800000, // mov w*, *
        0x52800000, // mov w*, *
        0x1a800000, // csel w*, w*, w*, eq
        0x14000000  // b
    };

    uint32_t masks[] = {
        0xffc00000,
        0xffc00000,
        0xffe0fc00,
        0xfc000000
    };

    struct pf_patch32_t patch = pf_construct_patch32(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) platform_check_callback16);

    // r2: /x 090080520801881a:1f00c0ff0efdfeff
    uint32_t matches2[] = {
        0x52800009, // mov w9, *
        0x1a880108  // csel w8, w{8/9}, w{8/9}, eq
    };

    uint32_t masks2[] = {
        0xffc0001f,
        0xfffefd0e
    };

    struct pf_patch32_t patch2 = pf_construct_patch32(matches2, masks2, sizeof(matches2) / sizeof(uint32_t), (void *) platform_check_callback16_alt);

    // r2: /x 600240f900004029000040b900000014:e003c0ff0000c0ff0000c0bf000000fc
    uint32_t bv_matches[] = {
        0xf9400260, // ldr x0, [x*, 0x20]
        0x29400000, // ldp
        0xb9400000, // ldr x*, [x*, 0x10]
        0x14000000  // b
    };

    uint32_t bv_masks[] = {
        0xffc003e0,
        0xffc00000,
        0xbfc00000,
        0xfc000000
    };

    struct pf_patch32_t bv_patch = pf_construct_patch32(bv_matches, bv_masks, sizeof(bv_matches) / sizeof(uint32_t), (void *) platform_check_callback16_bv);

    struct pf_patch32_t patches[] = {
        patch,
        patch2,
        bv_patch
    };

    struct pf_patchset32_t patchset = pf_construct_patchset32(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit32(dyld_buf, dyld_len, patchset);
}