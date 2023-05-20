#include <stdint.h>
#include <jbinit.h>
#include <stdbool.h>
#include "plooshfinder.h"
#include "plooshfinder32.h"
#include "macho.h"
#include "patches/platform/shellcode.h"

uint32_t *shc_loc;
int shc_copied = 0;

uint32_t *get_shc_region(void *buf) {
    // we use a zero region in the the __TEXT segment of dyld
    // this is done so our shellcode is in an executable region
    // __unwind_info is the last section in __TEXT,
    // so we just go into the zero region and
    // place our shellcode about half way in
    
    struct segment_command_64 *segment = macho_get_segment(buf, "__TEXT");
    if (!segment) return 0;
    struct section_64 *section = macho_get_section(buf, segment, "__unwind_info");
    if (!section) return 0;

    void *section_addr = buf + section->offset;
    uint64_t section_len = section->size;

    void *shc_base = section_addr + section_len;
    uint64_t shc_off_base = (segment->filesize - section->offset) / 2;
    uint64_t shc_off_aligned = shc_off_base & ~0x3;

    void *shc_addr = shc_base + shc_off_aligned;

    shc_loc = (uint32_t *) shc_addr;

    if (!shc_loc) {
        printf("%s: no shellcode location!\n", __FUNCTION__);
    }

    return shc_loc;
}

uint32_t shellcode_br[] = {
    0xf100283f, // cmp x1, 10
    0x54000040, // b.eq 0x8
    0xd2800001, // mov x1, {plat}
    0x0,        // br {reg}
};

uint32_t shellcode_blr[] = {
    0xa9bf7bfd, // stp fp, lr, [sp, -0x10]!
    0x910003fd, // mov fp, sp
    0xf100283f, // cmp x1, 10
    0x54000040, // b.eq 0x8
    0xd2800001, // mov x1, {plat}
    0x0,        // blr {reg}
    0xa8c17bfd, // ldp fp, lr, [sp], 0x10
    ret
};

uint32_t *copy_shc(int platform, uint32_t jmp) {
    if (!shc_loc) {
        printf("%s: No shellcode location!\n", __FUNCTION__);
        return 0;
    }

    uint32_t *shellcode = 0;
    size_t shc_size = 0;

    if (pf_maskmatch32(jmp, 0xd61f0000, 0xfffffc1f)) {
        if (shc_copied != 0) return shc_loc;

        shellcode = shellcode_br;
        shc_size = sizeof(shellcode_br) / sizeof(uint32_t);

        shellcode[2] |= platform << 5;
        shellcode[3] = jmp;
    } else {
        shellcode = shellcode_blr;
        shc_size = sizeof(shellcode_blr) / sizeof(uint32_t);

        shellcode[4] |= platform << 5;
        shellcode[5] = jmp;
    }

    uint32_t shc_off = shc_copied * shc_size;

    for (int i = 0; i < shc_size; i++) {
        shc_loc[shc_off + i] = shellcode[i];
    }

    shc_copied += 1;
    return shc_loc + shc_off;
}
