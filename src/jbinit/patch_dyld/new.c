#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <stdlib.h>
#endif

#include "plooshfinder.h"
#include "shellcode.h"

int _internal_new_platform = 0;
void *_internal_new_rbuf;
uint32_t *_internal_new_shc_loc;

bool inject_shc_new(struct pf_patch32_t patch, uint32_t *stream) {
    if (!_internal_new_shc_loc) {
        _internal_new_shc_loc = get_shc_region(_internal_new_rbuf);
        if (!_internal_new_shc_loc) {
            return false;
        }
    }
    
    uint32_t *blr = pf_find_next(stream, 10, 0xd63f0000, 0xfffffc1f);;

    if (!blr) {
        uint32_t *b = pf_find_next(stream, 10, 0x14000000, 0xfc000000);
        uint32_t *followed = pf_follow_branch(b);

        if (!followed) {
            LOG("%s: failed to follow b\n", __FUNCTION__);
        }

        blr = pf_find_next(followed, 5, 0xd63f0000, 0xfffffc1f);
    }

    if (!blr) {
        LOG("%s: failed to find blr\n", __FUNCTION__);
        return false;
    }

    uint32_t *shc_loc = copy_shc(_internal_new_platform, blr[0]);
    
    if (!shc_loc) {
        LOG("%s: no shellcode location??\n", __FUNCTION__);
        return false;
    }

    blr[0] = 0x94000000 | (uint32_t) (shc_loc - blr); // branch to our shellcode to determine if we should change platform or leave it

    LOG("%s: Patched platform check (shc b: 0x%x)\n", __FUNCTION__, 0x94000000 | (uint32_t) (_internal_new_shc_loc - blr));

    return true;
}

void patch_platform_check_new(void *real_buf, void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal_new_rbuf = real_buf;
    _internal_new_platform = platform;

    // r2: /x 600240f900004029201040b9:e003c0ff0000c0ffe0ffffff
    uint32_t matches[] = {
        0xf9400260, // ldr x0, [x*, 0x20]
        0x29400000, // ldp
        0xb9401020  // ldr w*, [x1, 0x10]
    };

    uint32_t masks[] = {
        0xffc003e0,
        0xffc00000,
        0xffffffe0
    };

    struct pf_patch32_t patch = pf_construct_patch32(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) inject_shc_new);

    // r2: /x 000040f90100805200003fd6000040f9000040f9:e003c0ff1f00e0ff1ffcffff0000c0ff0000c0ff
    uint32_t matches2[] = {
        0xf9400000, // ldr x*, [x0, 0x10]
        0x52800001, // mov w1, *
        0xd63f0000, // blr
        0xf9400000, // ldr
        0xf9400000  // ldr
    };

    uint32_t masks2[] = {
        0xffc003e0,
        0xffe0001f,
        0xfffffc1f,
        0xffc00000,
        0xffc00000,
    };

    struct pf_patch32_t patch2 = pf_construct_patch32(matches2, masks2, sizeof(matches2) / sizeof(uint32_t), (void *) inject_shc_new);

    struct pf_patch32_t patches[] = {
        patch,
        patch2
    };

    struct pf_patchset32_t patchset = pf_construct_patchset32(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit32(dyld_buf, dyld_len, patchset);
}