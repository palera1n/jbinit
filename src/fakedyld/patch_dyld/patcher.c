#include <plooshfinder/formats/macho.h>
#include <patches/platform/patch.h>
#include <fakedyld/fakedyld.h>

#define symbol_to_patch "____ZNK5dyld39MachOFile24forEachSupportedPlatformEU13block_pointerFvNS_8PlatformEjjE_block_invoke"

void platform_check_patch(void* arm64_dyld_buf, int platform) {
    // this patch tricks dyld into thinking everything is for the current platform
    struct nlist_64 *forEachSupportedPlatform = macho_find_symbol(arm64_dyld_buf, symbol_to_patch);

    void *func_addr = arm64_dyld_buf + forEachSupportedPlatform->offset;
    uint64_t func_len = macho_get_symbol_size(forEachSupportedPlatform);

    patch_platform_check(arm64_dyld_buf, func_addr, func_len, platform);
}

void check_dyld(const memory_file_handle_t* dyld_handle) {
    if (!dyld_handle->file_p) {
        panic("refusing to patch dyld buf at NULL");
    }
    uint32_t magic = macho_get_magic(dyld_handle->file_p);
    if (!magic) {
        panic("detected corrupted dyld");
    }
    if (magic == 0xbebafeca) {
        void* arm64_dyld_buf = macho_find_arch(dyld_handle->file_p, CPU_TYPE_ARM64);
        if (!arm64_dyld_buf) {
            panic("detected unsupported or invalid dyld architecture\n");
        }
    }
    return;
}

int get_platform(const memory_file_handle_t* dyld_handle) {
    int platform = macho_get_platform(macho_find_arch(dyld_handle->file_p, CPU_TYPE_ARM64));
    if (platform == 0) {
        panic("detected unsupported or invalid platform\n");
    }
    return platform;
}

void patch_dyld(memory_file_handle_t* dyld_handle, int platform) {
    platform_check_patch(macho_find_arch(dyld_handle->file_p, CPU_TYPE_ARM64), platform);
    LOG("done patching dyld");
}
