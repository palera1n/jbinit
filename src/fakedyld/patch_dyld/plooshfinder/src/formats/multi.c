#include <stdbool.h>
#include <stdint.h>
#include <fakedyld/fakedyld.h>
#include "formats/macho.h"
#include "formats/elf.h"
#include "formats/pe.h"
#include "utils.h"
// right now this is just ptr & virtual address conversion

void *pf_va_to_ptr(void *buf, uint64_t addr) {
    void *ptr = NULL;

    if (macho_check(buf)) {
        ptr = macho_va_to_ptr(buf, addr);
    } else if (elf_check(buf)) {
        ptr = elf_va_to_ptr(buf, addr);
    } else if (pe_check(buf)) {
        ptr = pe_va_to_ptr(buf, addr);
    } else {
        printf("%s: Unknown binary format!\n", __FUNCTION__);
    }

    return ptr;
}

uint64_t pf_ptr_to_va(void *buf, void *ptr) {
    uint64_t va = 0;

    if (macho_check(buf)) {
        va = macho_ptr_to_va(buf, ptr);
    } else if (elf_check(buf)) {
        va = elf_ptr_to_va(buf, ptr);
    } else if (pe_check(buf)) {
        va = pe_ptr_to_va(buf, ptr);
    } else {
        printf("%s: Unknown binary format!\n", __FUNCTION__);
    }

    return va;
}
