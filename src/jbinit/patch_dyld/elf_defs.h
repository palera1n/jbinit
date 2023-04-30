#ifndef _ELF_DEFS_H
#define _ELF_DEFS_H
#include <stdint.h>

#define SHT_SYMTAB 0x2
#define SHT_DYNSYM 0xb
#define PT_LOAD 0x1

struct elf_ident_64 {
    char signature[4];
    uint8_t file_class;
    uint8_t encoding;
    uint8_t version;
    uint8_t os;
    uint8_t abi_version;
    char pad[7];
};

struct elf_header_64 {
    struct elf_ident_64 ident;
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t ph_off;
    uint64_t sh_off;
    uint32_t flags;
    uint16_t head_size;
    uint16_t ph_size;
    uint16_t ph_count;
    uint16_t sh_size;
    uint16_t sh_count;
    uint16_t sect_table_index;
};

struct elf_pheader_64 {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t file_size;
    uint64_t memory_size;
    uint64_t align;
};

struct elf_sheader_64 {
    uint32_t name_off;
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t align;
    uint64_t entry_count;
};

struct elf_symbol_64 {
    uint32_t name;
    unsigned char info;
    unsigned char other;
    uint16_t sh_index;
    uint64_t offset;
    uint64_t size;
};

#endif