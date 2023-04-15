#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <stdlib.h>
#endif

#include "plooshfinder.h"

uint32_t *shc_loc;

uint32_t *get_shc_region(void *buf) {
    // we use a zero region in the the __TEXT segment of dyld
    // this is done so our shellcode is in an executable region
    // __unwind_info is the last section in __TEXT,
    // so we just go into the zero region and
    // place our shellcode 0x1000 bytes in
    
    struct section_64 *section = macho_find_section(buf, "__TEXT", "__unwind_info");
    if (!section) return 0;

    void *section_addr = buf + section->offset;
    uint64_t section_len = section->size;

    void *shc_addr = section_addr + section_len + 0x1000;

    shc_loc = (uint32_t *) shc_addr;

    return shc_loc;
}

void copy_shc(int platform) {
    if (!shc_loc) {
        LOG("%s: No shellcode location!\n", __FUNCTION__);
        return;
    }

    uint32_t shellcode[] = {
        0xd2800001, // mov x1, {plat}
        0xf100151f, // cmp x8, 5
        0x9a889021, // csel x1, x1, x8, ls
        ret
    };

    shellcode[0] |= platform << 5;

    for (int i = 0; i < (sizeof(shellcode) / sizeof(uint32_t)); i++) {
        shc_loc[i] = shellcode[i];
    }
}