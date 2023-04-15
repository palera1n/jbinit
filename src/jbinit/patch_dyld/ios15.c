#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <stdlib.h>
#endif
#include "plooshfinder.h"

int _internal15_platform = 0;

bool platform_check_callback15(struct pf_patch32_t patch, uint32_t *stream) {
    stream[3] = 0x52800001 | (_internal15_platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (_internal15_platform << 5));

    return true;
}

void patch_platform_check15(void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal15_platform = platform;

    // r2: /x 600240f900004029000040f901008052:e003c0ff0000c0ffe003c0ff1f00e0ff
    uint32_t matches[] = {
        0xf9400260, // ldr x0, [x*, 0x20]
        0x29400000, // ldp
        0xf9400000, // ldr x*, [x0, 0x10]
        0x52800001  // mov w1, *
    };

    uint32_t masks[] = {
        0xffc003e0,
        0xffc00000,
        0xffc003e0,
        0xffe0001f
    };
    struct pf_patch32_t patch = pf_construct_patch32(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) platform_check_callback15);

    // r2: /x 0000801a600240f9000040f901008052:00fce0ffe003c0ffe003c0ff1f00e0ff
    uint32_t matches2[] = {
        0x1a800000, // csel w*, w*, w*, eq
        0xf9400260, // ldr x0, [x*, 0x20]
        0xf9400000, // ldr x*, [x0, 0x10]
        0x52800001  // mov w1, *
    };

    uint32_t masks2[] = {
        0xffe0fc00,
        0xffc003e0,
        0xffc003e0,
        0xffe0001f
    };

    struct pf_patch32_t patch2 = pf_construct_patch32(matches2, masks2, sizeof(matches2) / sizeof(uint32_t), (void *) platform_check_callback15);

    struct pf_patch32_t patches[] = {
        patch,
        patch2
    };

    struct pf_patchset32_t patchset = pf_construct_patchset32(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit32(dyld_buf, dyld_len, patchset);
}