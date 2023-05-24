#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>


#define CHECK(expression)            \
    if (!(expression)){              \
        goto end;                    \
    }

#define APFS_MOUNT_DEFAULT_SNAPSHOT         0x0
#define APFS_MOUNT_LIVEFS                   0x1
#define APFS_MOUNT_CUSTOM_SNAPSHOT          0x2
#define APFS_IMG4_SNAPSHOT                  0x3
/* fusion ? */
#define APFS_MOUNT_MNTAPFS_LARGE_C_OPT      0x4
#define APFS_MOUNT_MNTAPFS_SMALL_C_OPT      0x5

#define APFS_MOUNT_IMG4                     0x8

struct apfs_mountarg {
    char *path;
    uint64_t _null;
    uint64_t apfs_flags;
    uint32_t _pad;
    char snapshot[0x100];
    char im4p[16];
    char im4m[16];
};

struct tmpfs_mountarg {
    uint64_t max_pages;
    uint64_t max_nodes;
    uint8_t case_insensitive;
};

#endif
