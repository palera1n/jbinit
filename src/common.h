#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>

struct apfs_mountarg {
    char *path;
    uint64_t _null;
    uint64_t mountAsRaw;
    uint32_t _pad;
    char snapshot[0x100];
};

struct tmpfs_mountarg {
    uint64_t max_pages;
    uint64_t max_nodes;
    uint8_t case_insensitive;
};

#endif
