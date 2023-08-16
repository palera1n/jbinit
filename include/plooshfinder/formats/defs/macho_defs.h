#ifndef _MACHO_DEFS_H
#define _MACHO_DEFS_H
#include <stdint.h>

#define LC_SEGMENT_64 0x19
#define LC_BUILD_VERSION 0x32
#define LC_SYMTAB 0x2
#define CPU_TYPE_ARM64 0xc000001
#define LC_FILESET_ENTRY 0x80000035

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

struct symtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t symoff;
    uint32_t nsyms;
    uint32_t stroff;
    uint32_t strsize;
};

struct nlist_64 {
    union {
        uint32_t str_index;
    } un;
    uint8_t type;
    uint8_t nsect;
    uint16_t desc;
    uint64_t offset;
};

struct kmod_info {
    struct kmod_info *next;
	int32_t info_version;
	uint32_t id;
	char name[64];
	char version[64];
	int32_t reference_count;
	struct kmod_reference *reference_list;
	uint64_t address;
	uint64_t size;
	uint64_t hdr_size;
	void *start;
	void *stop;
};

struct kmod_reference {
	struct kmod_reference *next;
	struct kmod_info *info;
};

struct fileset_entry_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint64_t vmaddr;
    uint64_t fileoff;
    uint32_t entry_id;
    uint32_t reserved;
};

#endif