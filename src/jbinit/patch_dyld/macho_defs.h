#ifndef MACHO_DEFS_H
#define MACHO_DEFS_H
#include <stdint.h>

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

#endif
