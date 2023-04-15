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

void copy_shc(int platform, char target_reg, char base_reg, char reg1, char reg2) {
    if (!shc_loc) {
        LOG("%s: No shellcode location!\n", __FUNCTION__);
        return;
    }

    uint32_t shellcode[] = {
        0xb9400800, // ldr w{reg2}, [x{base}, 0x8]
        0x52800000, // mov w{reg1}, {platform}
        0x7100141f, // cmp w{reg2}, #5
        0x1a80d000, // csel w{target}, w{reg1}, w{reg2}, le
        ret
    };

    uint32_t insert_base = base_reg << 5;
    uint32_t insert_plat = platform << 5;
    uint32_t insert_reg1 = reg1 << 5;
    uint32_t insert_reg2 = reg2 << 5;

    // insert the registers and values into the shellcode
    shellcode[0] |= reg2 | insert_base;
    shellcode[1] |= insert_plat | reg1;
    shellcode[2] |= insert_reg2;
    shellcode[3] |= target_reg | insert_reg1 | (reg2 << 16); 

    for (int i = 0; i < (sizeof(shellcode) / sizeof(uint32_t)); i++) {
        shc_loc[i] = shellcode[i];
    }
}