#ifndef _ELF_H
#define _ELF_H
#include <stdbool.h>
#include "elf_defs.h"

bool is_elf(void *buf);
struct elf_sheader_64 *elf_get_section(void *buf, char *name);
uint64_t elf_get_offset(void *buf);
struct elf_symbol_64 *elf_find_symbol_stype(void *buf, char *name, uint32_t type);
struct elf_symbol_64 *elf_find_symbol(void *buf, char *name);

#endif