
#ifndef _PLOOSHFINDER_H
#define _PLOOSHFINDER_H

#include <stdint.h>
#include <jbinit.h>
#include <stdbool.h>

// util macros
#define nop 0xd503201f
#define ret 0xd65f03c0

struct pf_patch_t {
    void *matches;
    void *masks;
    uint32_t count;
    bool disabled;
    bool (*callback)(struct pf_patch_t patch, void *stream);
};
struct pf_patch32_t {
    uint32_t *matches;
    uint32_t *masks;
    uint32_t count;
    bool disabled;
    bool (*callback)(struct pf_patch32_t patch, void *stream);
};

struct pf_patch64_t {
    uint64_t *matches;
    uint64_t *masks;
    uint32_t count;
    bool disabled;
    bool (*callback)(struct pf_patch64_t patch, void *stream);
};

struct pf_patchset_t {
    struct pf_patch_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset_t patch);
};

struct pf_patchset32_t {
    struct pf_patch32_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset32_t patch);
};
struct pf_patchset64_t {
    struct pf_patch64_t *patches;
    uint32_t count;
    void (*handler)(void *buf, size_t size, struct pf_patchset64_t patch);
};

#define LC_SEGMENT_64 0x19
#define LC_BUILD_VERSION 0x32
#define CPU_TYPE_ARM64 0xc000001

struct mach_header_64 {
    uint32_t magic; 
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
};

struct fat_header {
    uint32_t magic;
    uint32_t nfat_arch;
};

struct fat_arch {
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t offset;
    uint32_t size;
    uint32_t align;
};

struct section_64 {
    char sectname[16];
    char segname[16];
    
    uint64_t addr;
    uint64_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
};

struct segment_command_64 {
    uint32_t cmd;
    uint32_t cmdsize;

    char segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    uint32_t maxprot;
    uint32_t initprot;
    uint32_t nsects;
    uint32_t flags;
};

struct load_command_64 {
    uint32_t cmd;
    uint32_t cmdsize;
};

struct build_version_command {
    uint32_t cmd;
    uint32_t cmdsize;

    uint32_t platform;
    uint32_t minos;
    uint32_t sdk;
    uint32_t ntools;
};

// patch utils
struct pf_patch32_t pf_construct_patch32(uint32_t matches[], uint32_t masks[], uint32_t count, bool (*callback)(struct pf_patch32_t patch, void *stream));
struct pf_patch64_t pf_construct_patch64(uint64_t matches[], uint64_t masks[], uint32_t count, bool (*callback)(struct pf_patch64_t patch, void *stream));
struct pf_patchset32_t pf_construct_patchset32(struct pf_patch32_t *patches, uint32_t count, void (*handler)(void *buf, size_t size, struct pf_patchset32_t patchset));
struct pf_patchset64_t pf_construct_patchset64(struct pf_patch64_t *patches, uint32_t count, void (*handler)(void *buf, size_t size, struct pf_patchset64_t patchset));
void pf_patchset_emit32(void *buf, size_t size, struct pf_patchset32_t patchset);
void pf_patchset_emit64(void *buf, size_t size, struct pf_patchset64_t patchset);
void pf_disable_patch32(struct pf_patch32_t patch);
void pf_disable_patch64(struct pf_patch64_t patch);

// utils for finding
void pf_find_maskmatch32(void *buf, size_t size, struct pf_patchset32_t patchset);
void pf_find_maskmatch64(void *buf, size_t size, struct pf_patchset64_t patchset);
uint32_t *pf_find_next(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask);
uint32_t *pf_find_prev(uint32_t *stream, uint32_t count, uint32_t match, uint32_t mask);
int32_t pf_signextend_32(int32_t val, uint8_t bits);
int64_t pf_signextend_64(int64_t val, uint8_t bits);
uint32_t *pf_follow_branch(uint32_t *insn);
int64_t pf_adrp_offset(uint32_t adrp);
void *pf_follow_xref(uint32_t *stream);

uint32_t macho_get_magic(void *buf);
void *macho_find_arch(void *buf, uint32_t arch);
uint32_t macho_get_platform(void *buf);
struct segment_command_64 *macho_get_segment(void *buf, char *name);
struct section_64 *macho_get_section(void *buf, struct segment_command_64 *segment, char *name);
struct section_64 *macho_find_section(void *buf, char *segment_name, char *section_name);

uint32_t convert_endianness32(uint32_t val);

void patch_platform_check15(void *dyld_buf, size_t dyld_len, uint32_t platform);
void patch_platform_check16(void *dyld_buf, size_t dyld_len, uint32_t platform);
void patch_platform_check_old(void *dyld_buf, size_t dyld_len, uint32_t platform);

#endif
