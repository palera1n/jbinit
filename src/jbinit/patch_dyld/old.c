#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <stdlib.h>
#endif
#include "plooshfinder.h"

int _internal_old_platform = 0;
void *_internal_old_rbuf;
uint32_t *_internal_old_shc_loc;

void old_copy_shc() {
    if (_internal_old_shc_loc) {
        return;
    }

    uint32_t *shc_loc = get_shc_region(_internal_old_rbuf);
    if (!shc_loc) {
        return;
    }
    _internal_old_shc_loc = copy_shc(_internal_old_platform, 0xd61f0080); // br x4
    if (!_internal_old_shc_loc) {
        printf("%s: No shellcode location??\n", __FUNCTION__);
        return;
    }
}

bool inject_shc_old(struct pf_patch32_t patch, uint32_t *stream) {
    old_copy_shc();

    stream[1] = 0x14000000 | (uint32_t) (_internal_old_shc_loc - stream - 1);

    printf("%s: Patched platform check (shc b: 0x%x)\n", __FUNCTION__, 0x14000000 | (uint32_t) (_internal_old_shc_loc - stream - 1));

    return true;
}

void patch_platform_check_old(void *real_buf, void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal_old_platform = platform;
    _internal_old_rbuf = real_buf;

    // first instruction matches all with Rd as 1
    // r2: /x 0100000080001fd6:1f000000ffffffff
    uint32_t matches[] = {
        0x00000001,
        0xd61f0080  // br x4
    };

    uint32_t masks[] = {
        0x0000001f,
        0xffffffff
    };

    struct pf_patch32_t patch = pf_construct_patch32(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) inject_shc_old);

    struct pf_patch32_t patches[] = {
        patch
    };

    struct pf_patchset32_t patchset = pf_construct_patchset32(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit32(dyld_buf, dyld_len, patchset);
}