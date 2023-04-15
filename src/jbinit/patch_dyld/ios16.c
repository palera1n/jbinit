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
    uint32_t *shc_loc = get_shc_region(_internal16_rbuf);
    copy_shc(_internal16_platform);

    if (!shc_loc) {
        return false;
    }

    stream[3] = 0x94000000 | (uint32_t) (shc_loc - stream - 3); // branch to our shellcode to determine if we should change platform or leave it

    LOG("%s: Patched platform check (shc b: 0x%x)\n", __FUNCTION__, 0x94000000 | (uint32_t) (shc_loc - stream - 1));

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

    // r2: /x 600240f900004029000040f9e10300aa:e003c0ff0000c0ffe003c0ffffffe0ff
    uint32_t bv_matches[] = {
        0xf9400260, // ldr x0, [x*, 0x20]
        0x29400000, // ldp
        0xf9400000, // ldr x*, [x0, 0x10]
        0xaa0003e1  // mov x1, x*
    };

    uint32_t bv_masks[] = {
        0xffc003e0,
        0xffc00000,
        0xffc003e0,
        0xffe0ffff
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