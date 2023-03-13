#include <stdbool.h>
#include <stdint.h>
#include <jbinit.h>
#include <plooshfinder.h>

int _internal_old_platform = 0;

bool platform_check_callback_old(uint32_t *stream) {
    stream[0] = 0x52800001 | (_internal_old_platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (_internal_old_platform << 5));

    return true;
}

void patch_platform_check_old(void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal_old_platform = platform;

    uint32_t matches_old[] = {
        0x52800001, // mov w1, *
        0xd61f0080  // br x4
    };

    uint32_t masks_old[] = {
        0xffe0001f,
        0xffffffff
    };

    pf_find_maskmatch32(dyld_buf, dyld_len, matches_old, masks_old, sizeof(matches_old) / sizeof(uint32_t), (void *)platform_check_callback_old);

    // ios 12 version
    matches_old[0] = 0x320001e1; // orr w1, wzr, *
    masks_old[0] = 0xffe001ff;

    pf_find_maskmatch32(dyld_buf, dyld_len, matches_old, masks_old, sizeof(matches_old) / sizeof(uint32_t), (void *)platform_check_callback_old);
}