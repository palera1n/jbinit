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

void copy_shc(int platform, char target_reg) {
    if (!shc_loc) {
        printf("%s: No shellcode location!\n", __FUNCTION__);
        return;
    }

    uint32_t shellcode[] = {
        0xb9400822, // ldr w2, [x1, 0x8]
        0x52800003, // mov w3, {platform}
        0x7100145f, // cmp w2, #5
        0x1a82d060, // csel w{target}, w3, w2, le
        ret
    };

    shellcode[1] |= (platform << 5); // set the platform in the shellcode
    shellcode[3] |= target_reg; // set the reg for the platform to be set to

    for (int i = 0; i < (sizeof(shellcode) / sizeof(uint32_t)); i++) {
        shc_loc[i] = shellcode[i];
    }
}