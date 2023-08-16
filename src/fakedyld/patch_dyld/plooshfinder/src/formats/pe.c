#include <stdbool.h>
#include <fakedyld/fakedyld.h>
#include "formats/pe.h"

bool dos_check(void *buf) {
    struct DOS_Header *dos_hdr = buf;

    if (strncmp(dos_hdr->e_magic, "MZ", 2) == 0) {
        return true;
    }

    return false;
}

struct COFF_Header *get_pe_header(void *buf) {
    if (!dos_check(buf)) {
        printf("No DOS stub!\n");
        return NULL;
    }

    struct DOS_Header *dos_hdr = buf;

    return buf + dos_hdr->e_lfanew;
}

bool pe_check(void *buf) {
    struct COFF_Header *pe_hdr = get_pe_header(buf);

    if (!pe_hdr) {
        return false;
    }

    if (strcmp(pe_hdr->magic, "PE") == 0) {
        return true;
    }
    
    return false;
}

bool is_pe(void *buf) {
    bool pe = pe_check(buf);
    
    if (!pe) {
        printf("%s: Not a PE!\n", __FUNCTION__);
    }

    return pe;
}

struct PE64_Optional_Header *get_pe_opt_header(void *buf) {
    if (!dos_check(buf)) {
        printf("No DOS stub!\n");
        return NULL;
    }

    if (!pe_check(buf)) return NULL;

    struct COFF_Header *pe_hdr = get_pe_header(buf);

    return (void *) pe_hdr + sizeof(struct COFF_Header);
}

struct Symbol_Header *pe_get_symtab(void *buf) {
    if (!pe_check(buf)) return NULL;

    struct COFF_Header *pe_hdr = get_pe_header(buf);

    return buf + pe_hdr->pointerToSymbolTable;
}

void *pe_get_strtab(void *buf) {
    if (!pe_check(buf)) return NULL;

    struct COFF_Header *pe_hdr = get_pe_header(buf);
    void *symtab = pe_get_symtab(buf);
    
    return symtab + (pe_hdr->numberOfSymbols * 18); // 18 is the correct size, but struct alignment makes it 20.
}

struct Section_Header *pe_get_section(void *buf, char *name) {
    if (!pe_check(buf)) return NULL;

    struct COFF_Header *pe_hdr = get_pe_header(buf);
    struct Section_Header *sects_start = (void *) pe_hdr + sizeof(struct COFF_Header) + pe_hdr->sizeOfOptionalHeader;
    void *strtab = pe_get_strtab(buf);

    for (int i = 0; i < pe_hdr->numberOfSections; i++) {
        struct Section_Header *section = sects_start + i;
        char *real_name = section->name;

        if (real_name[0] == '/') {
            real_name = strtab + strtoul(real_name + 1, 0, 0);
        }

        if (strncmp(real_name, name, 8) == 0) {
            return section;
        }
    }

    return NULL;
}

void *pe_va_to_ptr(void *buf, uint64_t addr) {
    if (!pe_check(buf)) return NULL;

    struct COFF_Header *pe_hdr = get_pe_header(buf);
    struct PE64_Optional_Header *opt_hdr = get_pe_opt_header(buf);
    struct Section_Header *sects_start = (void *) opt_hdr + pe_hdr->sizeOfOptionalHeader;

    for (int i = 0; i < pe_hdr->numberOfSections; i++) {
        struct Section_Header *section = sects_start + i;

        uint64_t section_start = opt_hdr->imageBase + section->virtualAddress;
        uint64_t section_end = section_start + section->virtualSize;

        if (section_start <= addr && section_end > addr) {
            uint64_t offset = addr - section_start;
            return buf + section->pointerToRawData + offset;
        }
    }

    return NULL;
}

uint64_t pe_ptr_to_va(void *buf, void *ptr) {
    if (!pe_check(buf)) return 0;

    struct COFF_Header *pe_hdr = get_pe_header(buf);
    struct PE64_Optional_Header *opt_hdr = get_pe_opt_header(buf);
    struct Section_Header *sects_start = (void *) opt_hdr + pe_hdr->sizeOfOptionalHeader;
    uint64_t ptr_addr = (uint64_t) ptr;

    for (int i = 0; i < pe_hdr->numberOfSections; i++) {
        struct Section_Header *section = sects_start + i;

        uint64_t section_start = (uint64_t) buf + section->pointerToRawData;
        uint64_t section_end = section_start + section->sizeOfRawData;

        if (section_start <= ptr_addr && section_end > ptr_addr) {
            uint64_t offset = ptr_addr - section_start;
            return opt_hdr->imageBase + section->virtualAddress + offset;
        }
    }

    return 0;
}

struct Symbol_Header *pe_find_symbol(void *buf, char *name) {
    if (!pe_check(buf)) return 0;

    struct COFF_Header *pe_hdr = get_pe_header(buf);
    void *symtab = pe_get_symtab(buf);
    void *strtab = pe_get_strtab(buf);

    for (int i = 0; i < pe_hdr->numberOfSymbols; i++) {
        struct Symbol_Header *symbol = symtab + (i * 18); // 18 is the correct size, but struct alignment makes it 20.
        char *real_name = symbol->name.name;

        if (symbol->name.over_8b.zeros == 0 && symbol->name.over_8b.strtab_off != 0) {
            real_name = strtab + symbol->name.over_8b.strtab_off;
        }

        if (strcmp(real_name, name) == 0) {
            return symbol;
        }
    }

    return NULL;
}
