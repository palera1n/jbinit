#include <stdbool.h>
#include <stdint.h>
#include <jbinit.h>
#include <plooshfinder.h>

int _internal_platform = 0;

bool platform_check_callback15(uint32_t *stream) {
    stream[3] = 0x52800001 | (_internal_platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (_internal_platform << 5));

    return true;
}

void patch_platform_check15(void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal_platform = platform;

    uint32_t ios15_matches[] = {
        0xf9400260, // ldr x0, [x*, 0x20]
        0x29400000, // ldp
        0xf9400000, // ldr x*, [x0, 0x10]
        0x52800001, // mov w1, *
        0x14000000  // b 
    };

    uint32_t ios15_masks[] = {
        0xffc003e0,
        0xffc00000,
        0xffc003e0,
        0xffe0001f,
        0xfc000000
    };
    pf_find_maskmatch32(dyld_buf, dyld_len, ios15_matches, ios15_masks, sizeof(ios15_matches) / sizeof(uint32_t), (void *)platform_check_callback15);

    ios15_matches[4] = 0xd63f0000; // blr x*
    ios15_masks[4] = 0xfffffc1f;

    pf_find_maskmatch32(dyld_buf, dyld_len, ios15_matches, ios15_masks, sizeof(ios15_matches) / sizeof(uint32_t), (void *)platform_check_callback15);

    // this codegen SUCKS
    uint32_t ios15_matches2[] = {
        0x1a800000, // csel w*, w*, w*, eq
        0xf9400260, // ldr x0, [x*, 0x20]
        0xf9400000, // ldr x*, [x0, 0x10]
        0x52800001, // mov w1, *
        0x14000000  // b 
    };

    uint32_t ios15_masks2[] = {
        0xffe0fc00,
        0xffc003e0,
        0xffc003e0,
        0xffe0001f,
        0xfc000000
    };

    pf_find_maskmatch32(dyld_buf, dyld_len, ios15_matches2, ios15_masks2, sizeof(ios15_matches2) / sizeof(uint32_t), (void *)platform_check_callback15);
}
