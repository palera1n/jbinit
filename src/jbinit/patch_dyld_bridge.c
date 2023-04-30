#include <jbinit.h>
#include <stdint.h>
#include "patch_dyld/plooshfinder.h"
#include "patch_dyld/macho_defs.h"
#include "patch_dyld/macho.h"
#include "patch_dyld/patches/platform/patch.h"

#define symbol_to_patch "____ZNK5dyld39MachOFile24forEachSupportedPlatformEU13block_pointerFvNS_8PlatformEjjE_block_invoke"

void *dyld_buf;
size_t dyld_len;
int platform = 0;

void platform_check_patch() {
    // this patch tricks dyld into thinking everything is for the current platform
    struct nlist_64 *forEachSupportedPlatform = macho_find_symbol(dyld_buf, symbol_to_patch);

    void *func_addr = dyld_buf + forEachSupportedPlatform->offset;
    uint64_t func_len = macho_get_symbol_size(forEachSupportedPlatform);

    patch_platform_check(dyld_buf, func_addr, func_len, platform);
}

void patch_dyld() {
    LOG("Plooshi(TM) libDyld64Patcher starting up...\n");
    LOG("patching dyld...\n");
    
    if (!dyld_buf) {
        LOG("refusing to patch dyld buf at NULL\n");
        spin();   
    }
    uint32_t magic = macho_get_magic(dyld_buf);
    if (!magic) {
        LOG("detected corrupted dyld\n");
        spin();
    }
    void *orig_dyld_buf = dyld_buf;
    if (magic == 0xbebafeca) {
        dyld_buf = macho_find_arch(dyld_buf, CPU_TYPE_ARM64);
        if (!dyld_buf) {
            LOG("detected unsupported or invalid dyld architecture\n");
            spin();
        }
    }
    platform = macho_get_platform(dyld_buf);
    if (platform == 0) {
        LOG("detected unsupported or invalid platform\n");
        spin();
    }
    platform_check_patch();
    LOG("done patching dyld\n");
}

void get_and_patch_dyld(void) {
    dyld_buf = read_file("/usr/lib/dyld", &dyld_len);
    patch_dyld();
    write_file("/cores/dyld", dyld_buf, dyld_len);
}
