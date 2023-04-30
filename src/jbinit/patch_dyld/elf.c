#include <stdbool.h>
#include <jbinit.h>
#include "elf.h"
#include "elf_defs.h"

bool is_elf(void *buf) {
    struct elf_header_64 *hdr = (struct elf_header_64 *) buf;
    char elf_magic[5] = { 0x7f, 'E', 'L', 'F' };

    if (strncmp(hdr->ident.signature, elf_magic, 4) == 0) {
        return true;
    } else {
        printf("Not an ELF!\n");
    }

    return false;
}

struct elf_sheader_64 *elf_get_section(void *buf, char *name) {
    if (!is_elf(buf)) {
        return NULL;
    }

    struct elf_header_64 *hdr = (struct elf_header_64 *) buf;
    struct elf_sheader_64 *section_hdr = buf + hdr->sh_off;

    struct elf_sheader_64 *sname_hdr = section_hdr + hdr->sect_table_index;
    void *sname_tbl = buf + sname_hdr->offset;
    

    for (uint16_t i = 0; i < hdr->sh_count; i++) {
        struct elf_sheader_64 *section = section_hdr + i;
        char *sect_name = sname_tbl + section->name_off;
        
        if (strcmp(sect_name, name) == 0) {
            return section;
        }
    }

    printf("%s: Unable to find section %s!\n", __FUNCTION__, name);
    return NULL;
}

uint64_t elf_get_offset(void *buf) {
    if (!is_elf(buf)) {
        return 0;
    }

    struct elf_header_64 *hdr = (struct elf_header_64 *) buf;
    struct elf_pheader_64 *program_hdr = buf + hdr->ph_off;

    for (int i = 0; i < hdr->ph_count; i++) {
        struct elf_pheader_64 *phdr = program_hdr + i;

        if (phdr->type == PT_LOAD) {
            return phdr->virtual_address;
        }
    }

    return 0;
}

struct elf_symbol_64 *elf_find_symbol_stype(void *buf, char *name, uint32_t type) {
    if (!is_elf(buf)) {
        return NULL;
    }

    struct elf_header_64 *hdr = (struct elf_header_64 *) buf;
    struct elf_sheader_64 *section_hdr = buf + hdr->sh_off;
    struct elf_sheader_64 *sect = NULL;
    struct elf_sheader_64 *strtab_sect;

    for (uint16_t i = 0; i < hdr->sh_count; i++) {
        struct elf_sheader_64 *section = section_hdr + i;

        if (section->type == type) {
            sect = section;
        }
    }

    if (!sect) {
        return NULL;
    }

    strtab_sect = section_hdr + sect->link;

    struct elf_symbol_64 *symtab = buf + sect->offset;
    char *strtab = buf + strtab_sect->offset;
    uint64_t count = sect->size / sizeof(struct elf_symbol_64);

    for (int i = 0; i < count; i++) {
        struct elf_symbol_64 *symbol = symtab + i;
        char *sym_name = strtab + symbol->name;

        if (strcmp(sym_name, name) == 0) {
            return symbol;
        }
    }

    return NULL;
}

struct elf_symbol_64 *elf_find_symbol(void *buf, char *name) {
    if (!is_elf(buf)) {
        return NULL;
    }

    struct elf_symbol_64 *symbol = elf_find_symbol_stype(buf, name, SHT_SYMTAB);

    if (!symbol) {
        symbol = elf_find_symbol_stype(buf, name, SHT_DYNSYM);
    }

    if (!symbol) {
        printf("%s: Failed to find symbol %s!\n", __FUNCTION__, name);
        return NULL;
    }

    return symbol;
}
