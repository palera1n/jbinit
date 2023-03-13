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

void patch_platform_check() {
    // this patch tricks dyld into thinking everything is for the current platform
    patch_platform_check_old(dyld_buf, dyld_len, platform);
    patch_platform_check15(dyld_buf, dyld_len, platform);
    patch_platform_check16(dyld_buf, dyld_len, platform);
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
