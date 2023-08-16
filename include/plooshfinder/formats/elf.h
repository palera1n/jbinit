#ifndef _ELF_H
#define _ELF_H
#include <stdbool.h>
#include <stdint.h>
#include "defs/elf_defs.h"

bool elf_check(void *buf);
bool is_elf(void *buf);
struct elf_sheader_64 *elf_get_section(void *buf, char *name);
void *elf_va_to_ptr(void *buf, uint64_t addr);
uint64_t elf_ptr_to_va(void *buf, void *ptr);
struct elf_symbol_64 *elf_find_symbol_stype(void *buf, char *name, uint32_t type);
struct elf_symbol_64 *elf_find_symbol(void *buf, char *name);

#endif