#include <plooshfinder/formats/macho.h>
#include <patches/platform/patch.h>
#include <fakedyld/fakedyld.h>
#include "plooshfinder/plooshfinder32.h"
#include "plooshfinder/plooshfinder.h"
#include "plooshfinder/asm/arm64.h"

#define amfi_check_dyld_policy_self_symbol "_amfi_check_dyld_policy_self"
#define platform_check_symbol "____ZNK5dyld39MachOFile24forEachSupportedPlatformEU13block_pointerFvNS_8PlatformEjjE_block_invoke"
#define platform_check_symbol_new "__ZNK6mach_o6Header19loadableIntoProcessENS_8PlatformE7CStringb"
#define appleinternal_symbol "__ZNK5dyld415SyscallDelegate15internalInstallEv"
#define start_symbol "start"

#ifdef DEV_BUILD
#define dev_panic(...) panic(__VA_ARGS__)
#else
#define dev_panic(...) LOG(__VA_ARGS__)
#endif

static void* arm64_dyld_buf = NULL;

extern bool has_found_platform_patch;
void platform_check_patch(void* arm64_dyld_buf, int platform) {
    struct nlist_64 *platform_symbol = macho_find_symbol(arm64_dyld_buf, platform_check_symbol);
    int generation = 1;
    
    if (!platform_symbol) {
        platform_symbol = macho_find_symbol(arm64_dyld_buf, platform_check_symbol_new);
        generation = 2;
    }
    
    if (!platform_symbol)
        panic("failed to find symbol %s or %s", platform_check_symbol, platform_check_symbol_new);

    void *func_addr = arm64_dyld_buf + platform_symbol->offset;
    uint64_t func_len = macho_get_symbol_size(platform_symbol);

    // this patch tricks dyld into thinking everything is for the current platform
    if (generation == 1)
        patch_platform_check(arm64_dyld_buf, func_addr, func_len, platform);
    // and this patch just skips the platform check
    else if (generation == 2)
        patch_platform_check_new(arm64_dyld_buf, func_addr, func_len, platform);
    if (!has_found_platform_patch) {
        panic("failed to find dyld platform check");
    }
}

bool has_found_dyld_in_cache = false;
bool patch_dyld_in_cache(struct pf_patch_t __attribute__((unused)) *patch, uint32_t *stream) {
    char* env_name = pf_follow_xref(arm64_dyld_buf, &stream[2]);
    char* env_value = pf_follow_xref(arm64_dyld_buf, &stream[6]);

    if (!env_name || !env_value) return false;
    if (strcmp(env_name, "DYLD_IN_CACHE") != 0 || strcmp(env_value, "0") != 0)
        return false;

    stream[5] = 0xd503201f; /* nop */
    stream[8] = 0x52800000; /* mov w0, #0 */
    has_found_dyld_in_cache = true;
    return true;
}

bool patch_dyld_in_cache_new(struct pf_patch_t __attribute__((unused)) *patch, uint32_t *stream) {
    struct nlist_64 *appleinternal_sym = macho_find_symbol(arm64_dyld_buf, appleinternal_symbol);

    if (!appleinternal_sym) {
        LOG("%s: failed to find %s\n", __func__, appleinternal_symbol);
        return false;
    }

    void* appleinternal = arm64_dyld_buf + appleinternal_sym->offset;

    uint32_t* cbz = pf_find_next(stream + 3, 5, 0x34000008, 0xff00001f); // cbz w8, ...

    if (!cbz) {
        LOG("%s: failed to find cbz\n", __func__);
        return false;
    }

    uint32_t* no_cache = cbz +((*cbz >> 5) & 0xfff);

    uint32_t* adrp = pf_find_prev(stream, 10, 0x90000001, 0x9f00001f); // adrp x1, ...
    char* env = pf_follow_xref(arm64_dyld_buf, adrp);

    if (!env)
        return false;

    if (strcmp(env, "DYLD_IN_CACHE")) {
        LOG("%s: environment variable is not DYLD_IN_CACHE\n", __func__);
        return false;
    }

    uint32_t* appleinternal_callsite = pf_find_prev(adrp - 2, 5, 0x94000000, 0xfc000000); // bl

    if (!appleinternal_callsite) {
        LOG("%s: could not find %s callsite\n", __func__, appleinternal_symbol);
        return false;
    }

    if (pf_follow_branch(arm64_dyld_buf, appleinternal_callsite) != appleinternal) {
        LOG("%s: candidate callsite is not %s callsite\n", __func__, appleinternal_symbol);
        return false;
    }

    appleinternal_callsite[0] = arm64_branch(appleinternal_callsite, no_cache, false);

    has_found_dyld_in_cache = true;
    return true;
}

void dyld_in_cache_patch(void* buf) {
    uint32_t matches[] = {
        0xaa1303e0, // mov x0, x19
        0x94000000, // bl dyld4::KernelArgs::findEnvp
        0x90000001, // adrp x1, "DYLD_IN_CACHE"@PAGE
        0x91000021, // add x1, "DYLD_IN_CACHE"@PAGEOFF
        0x94000000, // bl __simple_getenv
        0xb4000000, // cbz x0, ...
        0x90000001, // adrp x1, "0"@PAGE
        0x91000021, // add x1, "0"@PAGEOFF
        0x94000000, // bl strcmp
        0x34000000  // cbz w0, ...
    };
    uint32_t masks[] = {
        0xffffffff,
        0xfc000000,
        0x9f00001f,
        0xffc003ff,
        0xfc000000,
        0xff00001f,
        0x9f00001f,
        0xffc003ff,
        0xfc000000,
        0xff00001f
    };
    
    uint32_t matches_new[] = {
        0x7100c51f, // cmp w8, #0x31 ; w8 == '1'
        0x54000000, // b.eq
        0x7100c11f, // cmp w8, #0x30 ; w8 != '0'
        0x54000001  // b.ne
    };
    
    uint32_t masks_new[] = {
        0xffffffff,
        0xff00001f,
        0xffffffff,
        0xff00001f
    };

    struct pf_patch_t dyld_in_cache = pf_construct_patch(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) patch_dyld_in_cache);
    
    struct pf_patch_t dyld_in_cache_new = pf_construct_patch(matches_new, masks_new, sizeof(matches_new) / sizeof(uint32_t), (void *) patch_dyld_in_cache_new);

    struct pf_patch_t patches[] = {
        dyld_in_cache,
        dyld_in_cache_new
    };
    struct pf_patchset_t patchset = pf_construct_patchset(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);
    struct nlist_64 *start = macho_find_symbol(buf, start_symbol);
    void *func_addr = buf + start->offset;
    uint64_t func_len = macho_get_symbol_size(start);

    pf_patchset_emit(func_addr, func_len, patchset);

    if (!has_found_dyld_in_cache) {
        panic("failed to find dyld-in-cache");
    }
    LOG("Patched dyld-in-cache");
}

void dyld_proces_config_patch(void* buf) {
    struct nlist_64 *amfi_flags = macho_find_symbol(buf, amfi_check_dyld_policy_self_symbol);
    if (!amfi_flags) {
        dev_panic("%s not found", amfi_check_dyld_policy_self_symbol);
        return;
    }
    
    uint32_t *func_addr = buf + amfi_flags->offset;
    uint64_t func_len = macho_get_symbol_size(amfi_flags);
    if (func_len < 16) {
        dev_panic("%s too small", amfi_check_dyld_policy_self_symbol);
        return;
    }
    
    // Replace the entire func
    func_addr[0] = 0xd2801be8; // mov x8, 0xdf
    func_addr[1] = 0xf9000028; // str x8, [x1]
    func_addr[2] = 0xd2800000; // mov x0, #0
    func_addr[3] = 0xd65f03c0; // ret
    LOG("Patched dyld AMFI process config");
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
            panic("detected unsupported or invalid dyld architecture");
        }
    }
    return;
}

int get_platform(const memory_file_handle_t* dyld_handle) {
    int platform = macho_get_platform(macho_find_arch(dyld_handle->file_p, CPU_TYPE_ARM64));
    if (platform == 0) {
        panic("detected unsupported or invalid platform");
    }
    return platform;
}

void patch_dyld(memory_file_handle_t* dyld_handle, int platform) {
    check_dyld(dyld_handle);
    arm64_dyld_buf = macho_find_arch(dyld_handle->file_p, CPU_TYPE_ARM64);
    platform_check_patch(arm64_dyld_buf, platform);
    struct section_64* cstring = macho_find_section(arm64_dyld_buf, "__TEXT", "__cstring");
    if (!cstring) {
        panic("failed to find dyld cstring");
    }
    if (memmem(macho_va_to_ptr(arm64_dyld_buf, cstring->addr), cstring->size, "AMFI", sizeof("AMFI"))) {
        dyld_proces_config_patch(arm64_dyld_buf);
    }
    if (memmem(macho_va_to_ptr(arm64_dyld_buf, cstring->addr), cstring->size, "DYLD_IN_CACHE", sizeof("DYLD_IN_CACHE"))) {
        dyld_in_cache_patch(arm64_dyld_buf);
    }
    LOG("done patching dyld");
}
