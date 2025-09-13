#ifndef _MOUNT_ARGS_H
#define _MOUNT_ARGS_H

#include <stdint.h>
#if !__STDC_HOSTED__
#include <fakedyld/types.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#endif

enum {
    APFS_MOUNT_AS_ROOT = 0, /* mount the default snapshot */
    APFS_MOUNT_FILESYSTEM, /* mount live fs */
    APFS_MOUNT_SNAPSHOT, /* mount custom snapshot in apfs_mountarg.snapshot */
    APFS_MOUNT_SNAPSHOT_AUTH, /* mount snapshot while suppling some representation of im4p and im4m */
    APFS_MOUNT_FOR_VERIFICATION, /* Fusion mount with tier 1 & 2, set by mount_apfs when -C is used (Conversion mount) */
    APFS_MOUNT_FOR_INVERSION, /* Fusion mount with tier 1 only, set by mount_apfs when -c is used */
    APFS_MOUNT_MODE_REPAIR,  /* repair fs */
    APFS_MOUNT_FOR_INVERT, /* mount for invert */
    APFS_MOUNT_FILESYSTEM_AUTH /* mount live fs while suppling some representation of im4p and im4m */
};

#define APFS_MOUNT_IMG4_MAXSZ               0x100000

#define APFS_AUTH_ENV_GENERIC               4
#define APFS_AUTH_ENV_SUPPLEMENTAL          5
#define APFS_AUTH_ENV_PDI_NONCE             6

#define APFS_CRYPTEX_TYPE_GENERIC           0x67746776 /* vgtg */
#define APFS_CRYPTEX_TYPE_BRAIN             0x73796162 /* bays */

#define APFS_FLAGS_SMALL_N_OPT              0x400000000 /* mount_apfs -n */
#define APFS_FLAGS_LARGE_R_OPT              0x200000000 /* mount_apfs -R */
#define APFS_FLAGS_LARGET_S_OPT             0x800000000 /* mount_apfs -S */

#define APFSFSMNT_SKIPSANITY                0x1
#define APFSFSMNT_CHECKPOINT                0x2
#define APFSFSMNT_DEMOMODE                  0x4
#define APFSFSMNT_TINYOBJCACHE              0x8
#define APFSFSMNT_CREATETMPCP               0x10
#define APFSFSMNT_LOADTMPCP                 0x20
#define APFSFSMNT_COMMITTMPCP               0x40

/* Fourth argument to mount(2) when mounting apfs */
struct apfs_mount_args {
#ifndef KERNEL
    char* fspec; /* path to device to mount from */
#endif
    uint64_t apfs_flags; /* The standard mount flags, OR'd with apfs-specific flags (APFS_FLAGS_* above) */
    uint32_t mount_mode; /* APFS_MOUNT_* */
    uint32_t pad1; /* padding */
    uint32_t unk_flags; /* yet another type some sort of flags (bitfield), possibly volume role related */
    union {
        char snapshot[256]; /* snapshot name */
        struct {
            char tier1_dev[128]; /* Tier 1 device (Fusion mount) */
            char tier2_dev[128]; /* Tier 2 device (Fusion mount) */
        };
    };
    void* im4p_ptr;
    uint32_t im4p_size;
    uint32_t pad2; /* padding */
    void* im4m_ptr;
    uint32_t im4m_size;
    uint32_t im_4cc;
    uint32_t cryptex_type; /* APFS_CRYPTEX_TYPE_* */
    int32_t auth_mode; /* APFS_AUTH_ENV_* */
    uid_t uid;
    gid_t gid;
}__attribute__((packed, aligned(4)));

typedef struct apfs_mount_args apfs_mount_args_t;

#define HFSFSMNT_NOXONFILES	    0x1	/* disable execute permissions for files */
#define HFSFSMNT_WRAPPER	    0x2	/* mount HFS wrapper (if it exists) */
#define HFSFSMNT_EXTENDED_ARGS  0x4     /* indicates new fields after "flags" are valid */


/*
 * Fourth argument to mount(2) when mounting hfs 
*/
typedef struct hfs_mount_args {
#ifndef KERNEL
	char	*fspec;			/* block special device to mount */
#endif
	uid_t	hfs_uid;		/* uid that owns hfs files (standard HFS only) */
	gid_t	hfs_gid;		/* gid that owns hfs files (standard HFS only) */
	mode_t	hfs_mask;		/* mask to be applied for hfs perms  (standard HFS only) */
	uint32_t hfs_encoding;	/* encoding for this volume (standard HFS only) */
	struct	timezone hfs_timezone;	/* user time zone info (standard HFS only) */
	int		flags;			/* mounting flags, see HFSFSMNT_* */
	int     journal_tbuffer_size;   /* size in bytes of the journal transaction buffer */
	int		journal_flags;          /* flags to pass to journal_open/create */
	int		journal_disable;        /* don't use journaling (potentially dangerous) */

} hfs_mount_args_t;

/* Fourth argument to mount(2) when mounting tmpfs */
typedef struct tmpfs_mount_args {
    uint64_t max_pages; /* maximum amount of memory pages to be used for this tmpfs*/
    uint64_t max_nodes; /* maximum amount of inodes in this tmpfs */
    uint64_t case_insensitive; /* 1 = case insensitive, 0 = case sensitive */
} tmpfs_mount_args_t;

#endif /* MOUNR_ARGS_H */
