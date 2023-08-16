#ifndef FAKEDYLD_TYPES_H
#define FAKEDYLD_TYPES_H

#include <stdint.h>
#include <fakedyld/param.h>

typedef uint32_t kern_return_t;
typedef uint32_t mach_port_t;
typedef uint64_t mach_msg_timeout_t;
typedef int64_t user_ssize_t;
typedef int64_t off_t;
typedef uint64_t user_size_t;
typedef int64_t ssize_t;
typedef uint32_t attrgroup_t;
typedef int32_t dev_t;
typedef uint16_t mode_t;
typedef uint16_t nlink_t;
typedef int32_t pid_t;
typedef uint64_t __darwin_ino64_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef long time_t;
typedef long long blkcnt_t;
typedef int blksize_t;
typedef uint32_t uid_t;
typedef struct fsid { int32_t val[2]; } fsid_t;
typedef unsigned short u_short;

#define __DARWIN_STRUCT_DIRENTRY                                                    \
  {                                                                                 \
    uint64_t d_ino;                   /* file number of entry */                    \
    uint64_t d_seekoff;               /* seek offset (optional, used by servers) */ \
    uint16_t d_reclen;                /* length of this record */                   \
    uint16_t d_namlen;                /* length of string in d_name */              \
    uint8_t d_type;                   /* file type, see below */                    \
    char d_name[__DARWIN_MAXPATHLEN]; /* entry name (up to MAXPATHLEN bytes) */     \
  }

struct dirent __DARWIN_STRUCT_DIRENTRY;

#define __DARWIN_STRUCT_STAT64_TIMES \
        time_t          st_atime;               /* [XSI] Time of last access */ \
        long            st_atimensec;           /* nsec of last access */ \
        time_t          st_mtime;               /* [XSI] Last data modification time */ \
        long            st_mtimensec;           /* last data modification nsec */ \
        time_t          st_ctime;               /* [XSI] Time of last status change */ \
        long            st_ctimensec;           /* nsec of last status change */ \
        time_t          st_birthtime;           /*  File creation time(birth)  */ \
        long            st_birthtimensec;       /* nsec of File creation time */

struct stat64 { \
        dev_t           st_dev;                 /* [XSI] ID of device containing file */
        mode_t          st_mode;                /* [XSI] Mode of file (see below) */
        nlink_t         st_nlink;               /* [XSI] Number of hard links */
        __darwin_ino64_t st_ino;                /* [XSI] File serial number */
        uid_t           st_uid;                 /* [XSI] User ID of the file */ 
        gid_t           st_gid;                 /* [XSI] Group ID of the file */ 
        dev_t           st_rdev;                /* [XSI] Device ID */
        __DARWIN_STRUCT_STAT64_TIMES
        off_t           st_size;                /* [XSI] file size, in bytes */
        blkcnt_t        st_blocks;              /* [XSI] blocks allocated for file */
        blksize_t       st_blksize;             /* [XSI] optimal blocksize for I/O */
        uint32_t      st_flags;               /* user defined flags for file */
        uint32_t      st_gen;                 /* file generation number */
        int32_t       st_lspare;              /* RESERVED: DO NOT USE! */
        int64_t       st_qspare[2];           /* RESERVED: DO NOT USE! */
};

#define __DARWIN_STRUCT_STATFS64 { \
	uint32_t	f_bsize;        /* fundamental file system block size */ \
	int32_t		f_iosize;       /* optimal transfer block size */ \
	uint64_t	f_blocks;       /* total data blocks in file system */ \
	uint64_t	f_bfree;        /* free blocks in fs */ \
	uint64_t	f_bavail;       /* free blocks avail to non-superuser */ \
	uint64_t	f_files;        /* total file nodes in file system */ \
	uint64_t	f_ffree;        /* free file nodes in fs */ \
	fsid_t		f_fsid;         /* file system id */ \
	uid_t		f_owner;        /* user that mounted the filesystem */ \
	uint32_t	f_type;         /* type of filesystem */ \
	uint32_t	f_flags;        /* copy of mount exported flags */ \
	uint32_t	f_fssubtype;    /* fs sub-type (flavor) */ \
	char		f_fstypename[MFSTYPENAMELEN];   /* fs type name */ \
	char		f_mntonname[MAXPATHLEN];        /* directory on which mounted */ \
	char		f_mntfromname[MAXPATHLEN];      /* mounted filesystem */ \
	uint32_t    f_flags_ext;    /* extended flags */ \
	uint32_t	f_reserved[7];  /* For future use */ \
}

struct attrlist {
	u_short bitmapcount;                    /* number of attr. bit sets in list (should be 5) */
	uint16_t reserved;                     /* (to maintain 4-byte alignment) */
	attrgroup_t commonattr;                 /* common attribute group */
	attrgroup_t volattr;                    /* Volume attribute group */
	attrgroup_t dirattr;                    /* directory attribute group */
	attrgroup_t fileattr;                   /* file attribute group */
	attrgroup_t forkattr;                   /* fork attribute group */
};

typedef struct attribute_set {
	attrgroup_t commonattr;			/* common attribute group */
	attrgroup_t volattr;			/* Volume attribute group */
	attrgroup_t dirattr;			/* directory attribute group */
	attrgroup_t fileattr;			/* file attribute group */
	attrgroup_t forkattr;			/* fork attribute group */
} attribute_set_t;

typedef struct attrreference {
	int32_t     attr_dataoffset;
	uint32_t   attr_length;
} attrreference_t;

struct timezone { 
  int tz_minuteswest;
  int tz_dsttime;
};

struct statfs64 __DARWIN_STRUCT_STATFS64;

#endif
