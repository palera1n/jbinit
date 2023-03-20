#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <stdlib.h>
#endif

#include "plooshfinder.h"

int _internal16_platform = 0;

bool platform_check_callback16(struct pf_patch32_t patch, uint32_t *stream) {
    stream[2] = 0x52800001 | (_internal16_platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (_internal16_platform << 5));

    return true;
}

bool platform_check_callback16_alt(struct pf_patch32_t patch, uint32_t *stream) {
    stream[1] = 0x52800008 | (_internal16_platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800008 | (_internal16_platform << 5));

    return true;
}

void patch_platform_check16(void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal16_platform = platform;

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

    uint32_t matches2[] = {
        0x52800009, // mov w9, *
        0x1a880108  // csel w8, w{8/9}, w{8/9}, eq
    };

    uint32_t masks2[] = {
        0xffc0001f,
        0xfffefd0e
    };

    struct pf_patch32_t patch2 = pf_construct_patch32(matches2, masks2, sizeof(matches2) / sizeof(uint32_t), (void *) platform_check_callback16_alt);

    struct pf_patch32_t patches[] = {
        patch,
        patch2
    };

    struct pf_patchset32_t patchset = pf_construct_patchset32(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit32(dyld_buf, dyld_len, patchset);
}
