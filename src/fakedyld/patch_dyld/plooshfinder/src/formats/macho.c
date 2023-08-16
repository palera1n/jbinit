#include <stdbool.h>
#include <stdint.h>
#include <fakedyld/fakedyld.h>
#include "formats/macho.h"
#include "utils.h"

uint32_t macho_get_magic(void *buf) {
    uint32_t *buf_ptr = (uint32_t *) buf;
    uint32_t magic = buf_ptr[0];

    if (magic == 0xfeedfacf || magic == 0xbebafeca) {
        return magic;
    } else {
        printf("%s: Not a mach-o!\n", __FUNCTION__);
    }
    
    return 0;
}

bool macho_check(void *buf) {
    uint32_t magic = macho_get_magic(buf);

    if (magic == 0xfeedfacf || magic == 0xbebafeca) {
        return true;
    }
    
    return false;
}

void *macho_find_arch(void *buf, uint32_t arch) {
    uint32_t magic = macho_get_magic(buf);

    if (magic == 0xbebafeca) {
        struct fat_header *header = (struct fat_header *) buf;
        struct fat_arch *farch = (struct fat_arch *) ((char *) buf + sizeof(struct fat_header));

        for (int i = 0; i < convert_endianness32(header->nfat_arch); i++) {
            if (farch->cputype == arch) {
                return buf + convert_endianness32(farch->offset);
            }

            farch = (struct fat_arch *) ((char *) farch + sizeof(struct fat_arch));
        }

        printf("%s: Universal mach-o does not contain a slice for the arch requested!\n", __FUNCTION__);
    }

    return buf;
}

uint32_t macho_get_platform(void *buf) {
    if (!macho_check(buf)) {
        return 0;
    }

    struct load_command_64 *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_BUILD_VERSION) {
            struct build_version_command *cmd = (struct build_version_command *) after_header;

            if (cmd->platform > 5) {
                printf("%s: Invalid platform!\n", __FUNCTION__);
                return 0;
            }

            return cmd->platform; 
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    printf("%s: Unable to get platform!\n", __FUNCTION__);
    return 0;
}

struct segment_command_64 *macho_get_segment(void *buf, char *name) {
    if (!macho_check(buf)) {
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

    return NULL;
}

struct section_64 *macho_get_section(void *buf, struct segment_command_64 *segment, char *name) {
    if (!segment || !macho_check(buf)) {
        return NULL;
    }

    struct section_64 *section = (struct section_64 *) ((char *) segment + sizeof(struct segment_command_64));

    for (int i = 0; i < segment->nsects; i++) {
        if (strcmp(section->sectname, name) == 0) {
            return section;
        }

        section = (struct section_64 *) ((char *) section + sizeof(struct section_64));
    }

    return NULL;
}

struct section_64 *macho_get_last_section(struct segment_command_64 *segment) {
    uint32_t index = segment->nsects - 1;
    struct section_64 *sections = (struct section_64 *) ((char *) segment + sizeof(struct segment_command_64));

    return sections + index;
}

struct section_64 *macho_find_section(void *buf, char *segment_name, char *section_name) {
    if (!macho_check(buf)) {
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

struct fileset_entry_command *macho_get_fileset(void *buf, char *name) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct load_command_64 *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_FILESET_ENTRY) {
            struct fileset_entry_command *entry = (struct fileset_entry_command *) after_header;
            char *entry_name = (void *) entry + entry->entry_id;

            if (strcmp(entry_name, name) == 0) {
                return entry;
            }
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    return 0;
}

struct segment_command_64 *macho_get_segment_for_va(void *buf, uint64_t addr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct load_command_64 *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;
    struct segment_command_64 *segment = NULL;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_SEGMENT_64) {
            segment = (struct segment_command_64 *) after_header;
            uint64_t segment_start = segment->vmaddr;
            uint64_t segment_end = segment_start + segment->vmsize;

            if (segment_start <= addr && segment_end > addr) {
                // segment's range contains the addr
                return segment;
            }
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    printf("%s: Unable to find segment containing 0x%lx!\n", __FUNCTION__, addr);
    return NULL;
}

struct section_64 *macho_get_section_for_va(struct segment_command_64 *segment, uint64_t addr) {
    struct section_64 *section = (struct section_64 *) ((char *) segment + sizeof(struct segment_command_64));

    for (int i = 0; i < segment->nsects; i++) {
        uint64_t section_start = section->addr;
        uint64_t section_end = section_start + section->size;

        if (section_start <= addr && section_end > addr) {
            // section's range contains the addr
            return section;
        }

        section = (struct section_64 *) ((char *) section + sizeof(struct section_64));
    }

    printf("%s: Unable to find section containing 0x%lx?\n", __FUNCTION__, addr);
    return NULL;
}

struct section_64 *macho_find_section_for_va(void *buf, uint64_t addr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct segment_command_64 *segment = macho_get_segment_for_va(buf, addr);
    if (!segment) {
        return NULL;
    }

    struct section_64 *section = macho_get_section_for_va(segment, addr);
    if (!section) {
        return NULL;
    }

    return section;
}

void *macho_va_to_ptr(void *buf, uint64_t addr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct segment_command_64 *segment = macho_get_segment_for_va(buf, addr);
    if (!segment) {
        return NULL;
    } else if (segment->vmaddr == addr) {
        return buf + segment->fileoff;
    }

    struct section_64 *section = macho_get_section_for_va(segment, addr);

    uint64_t offset = addr - section->addr;
    
    return buf + section->offset + offset;
}

struct segment_command_64 *macho_get_segment_for_ptr(void *buf, void *ptr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct load_command_64 *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;
    struct segment_command_64 *segment = NULL;
    uint64_t ptr_addr = (uint64_t) ptr;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_SEGMENT_64) {
            segment = (struct segment_command_64 *) after_header;
            uint64_t segment_start = (uint64_t) buf + segment->fileoff;
            uint64_t segment_end = segment_start + segment->filesize;

            if (segment_start <= ptr_addr && segment_end > ptr_addr) {
                // segment's range contains the ptr
                return segment;
            }
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    printf("%s: Unable to find segment containing ptr %p!\n", __FUNCTION__, ptr);
    return NULL;
}

struct section_64 *macho_get_section_for_ptr(struct segment_command_64 *segment, void *buf, void *ptr) {
    struct section_64 *section = (struct section_64 *) ((char *) segment + sizeof(struct segment_command_64));
    uint64_t ptr_addr = (uint64_t) ptr;

    for (int i = 0; i < segment->nsects; i++) {
        uint64_t section_start = (uint64_t) buf + section->offset;
        uint64_t section_end = section_start + section->size;

        if (section_start <= ptr_addr && section_end > ptr_addr) {
            // section's range contains the ptr
            return section;
        }

        section = (struct section_64 *) ((char *) section + sizeof(struct section_64));
    }

    printf("%s: Unable to find section containing %p?\n", __FUNCTION__, ptr);
    return NULL;
}

struct section_64 *macho_find_section_for_ptr(void *buf, void *ptr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct segment_command_64 *segment = macho_get_segment_for_ptr(buf, ptr);
    if (!segment) {
        return NULL;
    }

    struct section_64 *section = macho_get_section_for_ptr(segment, buf, ptr);
    if (!section) {
        return NULL;
    }

    return section;
}

uint64_t macho_ptr_to_va(void *buf, void *ptr) {
    if (!macho_check(buf)) {
        return 0;
    }

    struct section_64 *section = macho_find_section_for_ptr(buf, ptr);

    uint64_t offset = ptr - buf - section->offset;
    
    return section->addr + offset;
}

struct nlist_64 *macho_find_symbol(void *buf, char *name) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct load_command_64 *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;
    struct symtab_command *symtab_cmd;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_SYMTAB) {
            symtab_cmd = (struct symtab_command *) after_header;

            break;
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    if (!symtab_cmd) {
        printf("%s: Unable to find symbol table!\n", __FUNCTION__);
        return NULL;
    }

    struct nlist_64 *symtab = buf + symtab_cmd->symoff;
    char *strtab = buf + symtab_cmd->stroff;

    for (uint32_t i = 0; i < symtab_cmd->nsyms; i++) {
        struct nlist_64 *symbol_nlist = symtab + i;
        char *sym_name = strtab + symbol_nlist->un.str_index;

        if (strcmp(sym_name, name) == 0) {
            return symbol_nlist;
        }
    }

    //printf("%s: Unable to find symbol %s!\n", __FUNCTION__, name);
    return NULL;
}

uint64_t macho_get_symbol_size(struct nlist_64 *symbol) {
    // this is not very reliable, as symtab doesn't include stripped symbols and can be in a weird order.
    
    struct nlist_64 *next_symbol = symbol + 1;

    if (next_symbol->offset < symbol->offset) {
        // symtab is in a weird order. can't really do anything about this, return 0.

        printf("%s: Symtab is in a weird order!\n", __FUNCTION__);
        return 0;
    }

    return next_symbol->offset - symbol->offset;
}

uint64_t macho_parse_plist_integer(void *key) {
    char *key_value = strstr(key, "<integer");

    if (key_value) {
        key_value = strstr(key_value, ">");

        if (key_value) {
            return strtoull(key_value + 1, 0, 0);
        }
    }

    return 0;
}

struct mach_header_64 *macho_parse_prelink_info(void *buf, struct section_64 *kmod_info, char *bundle_name) {
    if (!macho_check(buf)) {
        return NULL;
    }

    char kext_name[256];
    struct mach_header_64 *kext = NULL;

    char *start = buf + kmod_info->offset;

    char *info_dict = strstr(start, "PrelinkInfoDictionary");
    char *last_dict = strstr(info_dict, "<array>") + 7;

    while (last_dict) {
        char *dict_end = strstr(last_dict, "</dict>");
        if (!dict_end) break;

        char *dict2 = strstr(last_dict + 1, "<dict>");
        while (dict2) {
            if (dict2 > dict_end) break;

            dict2 = strstr(dict2 + 1, "<dict>");
            dict_end = strstr(dict_end + 1, "</dict>");
        }

        char *identifier = strstr(last_dict, "CFBundleIdentifier");

        if (identifier) {
            char *value_key = strstr(identifier, "<string>");

            if (value_key) {
                value_key += strlen("<string>");
                char *key_end = strstr(value_key, "</string>");

                if (key_end) {
                    uint32_t key_len = key_end - value_key;

                    memcpy(kext_name, value_key, key_len);
                    kext_name[key_len] = 0;

                    if (strcmp(kext_name, bundle_name) == 0) {
                        char *addr_key = strstr(last_dict, "_PrelinkExecutableLoadAddr");

                        if (addr_key) {
                            kext = (struct mach_header_64 *) macho_va_to_ptr(buf, macho_parse_plist_integer(addr_key));

                            break;
                        }
                    }
                }
            }
        }

        last_dict = strstr(dict_end, "<dict>");
    }

    return kext;
}

uint64_t macho_xnu_untag_va(uint64_t addr) {
    if (((addr >> 32) & 0xffff) == 0xfff0) {
        return (0xffffULL << 48) | addr;
    } else {
        return addr;
    }
}

struct mach_header_64 *macho_parse_kmod_info(void *buf, struct section_64 *kmod_info, struct section_64 *kmod_start, char *bundle_name) {\
    if (!macho_check(buf)) {
        return NULL;
    }

    struct mach_header_64 *kext = NULL;

    uint64_t kmod_count = kmod_info->size >> 3;
    uint64_t *info_start = buf + kmod_info->offset;
    uint64_t *start = buf + kmod_start->offset;

    for (uint64_t i = 0; i < kmod_count; i++) {
        struct kmod_info *info = macho_va_to_ptr(buf, macho_xnu_untag_va(info_start[i]));

        if (strcmp(info->name, bundle_name) == 0) {
            kext = (struct mach_header_64 *) macho_va_to_ptr(buf, macho_xnu_untag_va(start[i]));
        }
    }
    
    return kext;
}

struct mach_header_64 *macho_find_kext(void *buf, char *name) {
    struct mach_header_64 *kext = NULL;

    struct segment_command_64 *prelink_info = macho_get_segment(buf, "__PRELINK_INFO");
    if (!prelink_info) return NULL;

    struct section_64 *kmod_info = macho_get_section(buf, prelink_info, "__kmod_info");

    if (!kmod_info) {
        struct section_64 *info = macho_get_section(buf, prelink_info, "__info");
        if (!info) return NULL;

        kext = macho_parse_prelink_info(buf, info, name);
    } else {
        struct section_64 *kmod_start = macho_get_section(buf, prelink_info, "__kmod_start");
        if (!kmod_start) return NULL;

        kext = macho_parse_kmod_info(buf, kmod_info, kmod_start, name);
    }

    return kext;
}


void macho_run_each_kext(void *buf, void (*function)(void *real_buf, void *kextbuf, uint64_t kext_size)) {
    struct segment_command_64 *prelink_info = macho_get_segment(buf, "__PRELINK_INFO");
    if (!prelink_info) return;

    struct section_64 *kmod_start = macho_get_section(buf, prelink_info, "__kmod_start");

    if (!kmod_start) {
        struct section_64 *kexts_text = macho_find_section(buf, "__PLK_TEXT_EXEC", "__text");
        if (!kexts_text) return;

        function(buf, buf + kexts_text->offset, kexts_text->size);
    } else {
        uint64_t kmod_count = kmod_start->size >> 3;
        uint64_t *start = buf + kmod_start->offset;

        for (uint32_t i = 0; i < kmod_count; i++) {
            struct mach_header_64 *kext = macho_va_to_ptr(buf, macho_xnu_untag_va(start[i]));

            struct section_64 *kext_text = macho_find_section(kext, "__TEXT_EXEC", "__text");

            function(buf, macho_va_to_ptr(buf, macho_xnu_untag_va(kext_text->addr)), kext_text->size);
        }
    }
}

void *fileset_va_to_ptr(void *buf, void *kext, uint64_t addr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct segment_command_64 *segment = macho_get_segment_for_va(kext, addr);
    if (!segment) {
        return NULL;
    } else if (segment->vmaddr == addr) {
        return buf + segment->fileoff;
    }

    struct section_64 *section = macho_get_section_for_va(segment, addr);

    uint64_t offset = addr - section->addr;
    
    return buf + section->offset + offset;
}

struct segment_command_64 *fileset_get_segment_for_ptr(void *buf, void *kext, void *ptr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct load_command_64 *after_header = kext + sizeof(struct mach_header_64);
    struct mach_header_64 *header = kext;
    struct segment_command_64 *segment = NULL;
    uint64_t ptr_addr = (uint64_t) ptr;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_SEGMENT_64) {
            segment = (struct segment_command_64 *) after_header;
            uint64_t segment_start = (uint64_t) buf + segment->fileoff;
            uint64_t segment_end = segment_start + segment->filesize;

            if (segment_start <= ptr_addr && segment_end > ptr_addr) {
                // segment's range contains the ptr
                return segment;
            }
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    printf("%s: Unable to find segment containing ptr %p!\n", __FUNCTION__, ptr);
    return NULL;
}

struct section_64 *fileset_find_section_for_ptr(void *buf, void *kext, void *ptr) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct segment_command_64 *segment = fileset_get_segment_for_ptr(buf, kext, ptr);
    if (!segment) {
        return NULL;
    }

    struct section_64 *section = macho_get_section_for_ptr(segment, buf, ptr);
    if (!section) {
        return NULL;
    }

    return section;
}


uint64_t fileset_ptr_to_va(void *buf, void *kext, void *ptr) {
    if (!macho_check(buf)) {
        return 0;
    }

    struct section_64 *section = fileset_find_section_for_ptr(buf, kext, ptr);

    uint64_t offset = ptr - buf - section->offset;
    
    return section->addr + offset;
}

struct nlist_64 *fileset_find_symbol(void *buf, void *kext, char *name) {
    if (!macho_check(buf)) {
        return NULL;
    }

    struct load_command_64 *after_header = kext + sizeof(struct mach_header_64);
    struct mach_header_64 *header = kext;
    struct symtab_command *symtab_cmd;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_SYMTAB) {
            symtab_cmd = (struct symtab_command *) after_header;

            break;
        }

        after_header = (struct load_command_64 *) ((char *) after_header + after_header->cmdsize);
    }

    if (!symtab_cmd) {
        printf("%s: Unable to find symbol table!\n", __FUNCTION__);
        return NULL;
    }

    struct nlist_64 *symtab = buf + symtab_cmd->symoff;
    char *strtab = buf + symtab_cmd->stroff;

    for (uint32_t i = 0; i < symtab_cmd->nsyms; i++) {
        struct nlist_64 *symbol_nlist = symtab + i;
        char *sym_name = strtab + symbol_nlist->un.str_index;

        if (strcmp(sym_name, name) == 0) {
            return symbol_nlist;
        }
    }

    //printf("%s: Unable to find symbol %s!\n", __FUNCTION__, name);
    return NULL;
}
