#include <stdbool.h>
#include <stdint.h>
#include <jbinit.h>
#include <plooshfinder.h>

int _internal16_platform = 0;

bool platform_check_callback16(uint32_t *stream) {
    stream[2] = 0x52800001 | (_internal16_platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (_internal16_platform << 5));

    return true;
}

bool platform_check_callback16_alt(uint32_t *stream) {
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

    pf_find_maskmatch32(dyld_buf, dyld_len, matches, masks, sizeof(matches) / sizeof(uint32_t), (void *)platform_check_callback16);

    uint32_t matches2[] = {
        0x52800009, // mov w9, *
        0x1a880108  // csel w8, w{8/9}, w{8/9}, eq
    };

    uint32_t masks2[] = {
        0xffc0001f,
        0xfffefd0e
    };

    pf_find_maskmatch32(dyld_buf, dyld_len, matches2, masks2, sizeof(matches2) / sizeof(uint32_t), (void *)platform_check_callback16_alt);

    uint32_t matches3[] = {
        0xf9400260, // ldr x0, [x*, 0x20]
        0x29400000, // ldp
        0xb9400000, // ldr x*, [x0, 0x10]
        0x14000000  // b 
    };

    uint32_t masks3[] = {
        0xffc003e0,
        0xffc00000,
        0xffc00000,
        0xfc000000
    };

    pf_find_maskmatch32(dyld_buf, dyld_len, matches3, masks3, sizeof(matches3) / sizeof(uint32_t), (void *)platform_check_callback16_alt);
}