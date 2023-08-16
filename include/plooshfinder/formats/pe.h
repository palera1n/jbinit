#ifndef _PE_H
#define _PE_H
#include <stdbool.h>
#include <stdint.h>
#include "defs/pe_defs.h"

bool dos_check(void *buf);
struct COFF_Header *get_pe_header(void *buf);
bool pe_check(void *buf);
bool is_pe(void *buf);
struct PE64_Optional_Header *get_pe_opt_header(void *buf);
struct Symbol_Header *pe_get_symtab(void *buf);
void *pe_get_strtab(void *buf);
struct Section_Header *pe_get_section(void *buf, char *name);
void *pe_va_to_ptr(void *buf, uint64_t addr);
uint64_t pe_ptr_to_va(void *buf, void *ptr);
struct Symbol_Header *pe_find_symbol(void *buf, char *name);

#endif