#include <stdbool.h>
#include <jbinit.h>
#include <stdint.h>
#include "plooshfinder.h"
#include "plooshfinder32.h"
#include "patches/platform/patch.h"
#include "patches/platform/shellcode.h"

int _internal_platform = 0;
void *_internal_rbuf;
uint32_t *_internal_shc_loc;

bool inject_shc(struct pf_patch32_t patch, uint32_t *stream) {
    if (!_internal_shc_loc) {
        _internal_shc_loc = get_shc_region(_internal_rbuf);
        if (!_internal_shc_loc) {
            return false;
        }
    }

    uint32_t *shc_loc = copy_shc(_internal_platform, stream[0]);
    
    if (!shc_loc) {
        printf("%s: no shellcode location??\n", __FUNCTION__);
        return false;
    }

    uint32_t b_base = 0x94000000;

    if ((stream[0] & 0xfffffc1f) == 0xd61f0000) {
        b_base = 0x14000000;
    }

    stream[0] = b_base | (uint32_t) (shc_loc - stream); // branch to our shellcode to determine if we should change platform or leave it

    LOG("%s: Patched platform check (shc b: 0x%x)\n", __FUNCTION__, b_base | (uint32_t) (_internal_shc_loc - stream));

    return true;
}

void patch_platform_check(void *real_buf, void *dyld_buf, size_t dyld_len, uint32_t platform) {
    _internal_rbuf = real_buf;
    _internal_platform = platform;

    // r2: /x 00001fd6:1ffcdfff
    uint32_t matches[] = {
        0xd61f0000 // b{l}r
    };

    uint32_t masks[] = {
        0xffdffc1f
    };

    struct pf_patch32_t patch = pf_construct_patch32(matches, masks, sizeof(matches) / sizeof(uint32_t), (void *) inject_shc);

    struct pf_patch32_t patches[] = {
        patch
    };

    struct pf_patchset32_t patchset = pf_construct_patchset32(patches, sizeof(patches) / sizeof(struct pf_patch32_t), (void *) pf_find_maskmatch32);

    pf_patchset_emit32(dyld_buf, dyld_len, patchset);
}
