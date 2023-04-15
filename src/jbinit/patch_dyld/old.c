#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <stdlib.h>
#endif
#include "plooshfinder.h"

int _internal_old_platform = 0;

bool platform_check_callback_old(struct pf_patch32_t patch, uint32_t *stream) {
    stream[0] = 0x52800001 | (_internal_old_platform << 5);

    LOG("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (_internal_old_platform << 5));

    return true;
}

void patch_platform_check_old(void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal_old_platform = platform;

    // r2: /x 0100805280001fd6:1f00e0ffffffffff
    uint32_t matches[] = {
        0x52800001, // mov w1, *
        0xd61f0080  // br x4
    };

    uint32_t masks[] = {
        0xffe0001f,
        0xffffffff
    };

    struct pf_patch32_t patch = pf_construct_patch32(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) platform_check_callback_old);

    // ios 12 version
    // r2: /x e101003280001fd6:ff01e0ffffffffff
    uint32_t matches2[] = {
        0x320001e1, // orr w1, wzr, *
        0xd61f0080  // br x4
    };

    uint32_t masks2[] = {
        0xffe001ff,
        0xffffffff
    };

    struct pf_patch32_t patch2 = pf_construct_patch32(matches2, masks2, sizeof(matches2) / sizeof(uint32_t), (void *) platform_check_callback_old);

    struct pf_patch32_t patches[] = {
        patch,
        patch2
    };

    struct pf_patchset32_t patchset = pf_construct_patchset32(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit32(dyld_buf, dyld_len, patchset);
}