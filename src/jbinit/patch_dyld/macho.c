#if !__STDC_HOSTED__
#include <jbinit.h>
#else
#include <string.h>
#endif

#include "plooshfinder.h"

uint32_t macho_get_magic(void *buf) {
    uint32_t *buf_ptr = (uint32_t *) buf;
    uint32_t magic = buf_ptr[0];

    if (magic == 0xfeedfacf || magic == 0xbebafeca) {
        return magic;
    } else {
        LOG("Not a mach-o!\n");
    }
    
    return 0;
}

void *macho_find_arch(void *buf, uint32_t arch) {
    uint32_t *buf_ptr = (uint32_t *) buf;
    uint32_t magic = buf_ptr[0];

    if (magic == 0xbebafeca) {
        struct fat_header *header = (struct fat_header *) buf;

        struct fat_arch *farch = (struct fat_arch *) ((char *) buf + sizeof(struct fat_header));

        for (int i = 0; i < convert_endianness32(header->nfat_arch); i++) {
            if (farch->cputype == arch) {
                return buf + convert_endianness32(farch->offset);
            }

            farch = (struct fat_arch *) ((char *) farch + sizeof(struct fat_arch));
        }

        LOG("Universal mach-o does not contain an arm64 slice!\n");
    }

    return NULL;
}

uint32_t macho_get_platform(void *buf) {
    if (!macho_get_magic(buf)) {
        return 0;
    }

    struct load_command_64 *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_BUILD_VERSION) {
            struct build_version_command *cmd = (struct build_version_command *) after_header;

            if (cmd->platform > 5) {
                LOG("%s: Invalid platform!\n", __FUNCTION__);
                return 0;
            }

            return cmd->platform; 
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    LOG("%s: Unable to get platform!\n", __FUNCTION__);
    return 0;
}

struct segment_command_64 *macho_get_segment(void *buf, char *name) {
    if (!macho_get_magic(buf)) {
        return NULL;
    }

    struct load_command_64 *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_SEGMENT_64) {
            struct segment_command_64 *segment = (struct segment_command_64 *) after_header;

            if (strcmp(segment->segname, name) == 0) {
                return segment;
            }
        } else {
            break;
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    LOG("%s: Unable to find segment %s!\n", __FUNCTION__, name);
    return NULL;
}

struct section_64 *macho_get_section(void *buf, struct segment_command_64 *segment, char *name) {
    if (!segment || !macho_get_magic(buf)) {
        return NULL;
    }

    struct section_64 *section = (struct section_64 *)((char *)segment + sizeof(struct segment_command_64));

    for (int i = 0; i < segment->nsects; i++) {
        if (strcmp(section->sectname, name) == 0) {
            return section;
        }

        section = (struct section_64 *) ((char *) section + sizeof(struct section_64));
    }

    LOG("%s: Unable to find section %s!\n", __FUNCTION__, name);
    return NULL;
}

struct section_64 *macho_find_section(void *buf, char *segment_name, char *section_name) {
    if (!macho_get_magic(buf)) {
        return NULL;
    }

    struct segment_command_64 *segment = macho_get_segment(buf, segment_name);
    if (!segment) {
        return NULL;
    }

    struct section_64 *section = macho_get_section(buf, segment, section_name);
    if (!section) {
        return NULL;
    }

    return section;
}
