#ifndef PLOOSHFINDER_MACHO_H
#define PLOOSHFINDER_MACHO_H
#include "macho-defs.h"

uint32_t macho_get_magic(void *buf);
void *macho_find_arch(void *buf, uint32_t arch);
uint32_t macho_get_platform(void *buf);
struct segment_command_64 *macho_get_segment(void *buf, char *name);
struct section_64 *macho_get_section(void *buf, struct segment_command_64 *segment, char *name);
struct section_64 *macho_find_section(void *buf, char *segment_name, char *section_name);

#endif
