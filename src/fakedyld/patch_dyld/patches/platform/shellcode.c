#include <stdint.h>
#include <fakedyld/fakedyld.h>
#include <stdbool.h>
#include "plooshfinder.h"
#include "plooshfinder32.h"
#include "formats/macho.h"
#include "patches/platform/shellcode.h"

uint32_t *shc_loc = NULL;
struct section_64 *section = NULL;

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

uint32_t *copy_shc(void *buf, int platform, uint32_t jmp) {
    uint32_t *shellcode = 0;
    size_t shc_size = 0;

    bool br = pf_maskmatch32(jmp, 0xd61f0000, 0xfffffc1f);

    if (br) {
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

    if (br && shc_loc != NULL) {
        return shc_loc;
    }

    if (!section) {
        section = macho_find_section(buf, "__TEXT", "__text");
    }

    if (!section) {
        printf("%s: Unable to find text section!\n", __FUNCTION__);
        return NULL;
    }

    shc_loc = pf_find_zero_buf(buf + section->offset, section->size, shc_size);
    if (!shc_loc) {
        printf("%s: No shellcode location!\n", __FUNCTION__);
        return NULL;
    }

    for (int i = 0; i < shc_size; i++) {
        shc_loc[i] = shellcode[i];
    }

    return shc_loc;
}
