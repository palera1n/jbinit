#include <jbinit.h>
#include <stdint.h>
#include "plooshfinder.h"

void *dyld_buf;
size_t dyld_len;
int platform = 0;

int get_platform() {
    void *after_header = (char *)dyld_buf + 0x20;
    void *before_platform = after_header;

    while (*(uint32_t *)before_platform != 0x32) {
        before_platform += 4;
    }

    if (*(uint8_t *)before_platform == 0x32) {
        uint32_t *platform_ptr = (uint32_t *)before_platform + 2;
        platform = *platform_ptr;
    }

    if (platform > 5) {
        printf("Unknown platform!\n");
        return 1;
    }

    return 0;
}

bool platform_check_callback(uint32_t *stream) {
    stream[4] = 0xd2800001 | (platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0xd2800001 | (platform << 5));

    return true;
}

bool platform_check_callback_old(uint32_t *stream) {
    stream[0] = 0x52800001 | (platform << 5);

    printf("%s: Patched platform check (mov: 0x%x)\n", __FUNCTION__, 0x52800001 | (platform << 5));

    return true;
}

void patch_platform_check() {
    // this patch tricks dyld into thinking everything is for the current platform
    uint32_t matches[] = {
        0x1a800000, // csel w*, w*, w*, eq
        0xf9400260, // ldr x0, [x*, 0x20]
        0x29400000, // ldp
        0xf9400000, // ldr x*, [x0, 0x10]
        0xaa0003e1, // mov x1, x*
        0xd63f0000  // blr x* 
    };

    uint32_t masks[] = {
        0xffe0fc00,
        0xffc003e0,
        0xffc00000,
        0xffc003e0,
        0xffc0ffff,
        0xfffffc1f
    };

    pf_find_maskmatch32(dyld_buf, dyld_len, matches, masks, sizeof(matches) / sizeof(uint32_t), (void *)platform_check_callback);

    // ios 14 and lower
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

    // ios 15 has even more to find, so i moved it to a seperate file & function
    patch_platform_check15(dyld_buf, dyld_len, platform);
}

void patch_dyld() {
    puts("patching dyld...");
    dyld_buf = read_file("/usr/lib/dyld", &dyld_len);
    
    if (get_platform() != 0) {
        printf("Failed to get platform!\n");
        spin();
    }

    patch_platform_check();
    write_file("/cores/dyld", dyld_buf, dyld_len);
}
