#ifndef FAKEDYLD_PARAM_H
#define FAKEDYLD_PARAM_H

/* open */
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x00000200 /* create if nonexistant */
#define O_DIRECTORY 0x00100000
#define O_SYNC 0x0080      /* synch I/O file integrity */
#define O_TRUNC 0x00000400 /* truncate to zero length */
#define O_APPEND        0x00000008      /* set append mode */
#define O_EXCL          0x00000800      /* error if already exists */
#define O_CLOEXEC       0x01000000      /* close on exec */

/* lseek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* mmap */
#define PROT_NONE 0x00  /* [MC2] no permissions */
#define PROT_READ 0x01  /* [MC2] pages can be read */
#define PROT_WRITE 0x02 /* [MC2] pages can be written */
#define PROT_EXEC 0x04  /* [MC2] pages can be executed */

#define MAP_FAILED ((void*)-1)
#define MAP_FILE 0x0000 /* map from file (default) */
#define MAP_ANON 0x1000 /* allocated from memory, swap space */
#define MAP_ANONYMOUS MAP_ANON
#define MAP_SHARED 0x0001  /* [MF|SHM] share changes */
#define MAP_PRIVATE 0x0002 /* [MF|SHM] changes are private */

/* mount */
#define MNT_RDONLY 0x00000001
#define MNT_LOCAL 0x00001000
#define MNT_ROOTFS 0x00004000 /* identifies the root filesystem */
#define MNT_UNION 0x00000020
#define MNT_UPDATE 0x00010000  /* not a real mount, just an update */
#define MNT_NOBLOCK 0x00020000 /* don't block unmount if not responding */
#define MNT_RELOAD 0x00040000  /* reload filesystem data */
#define MNT_FORCE 0x00080000   /* force unmount or readonly change */

/* sysctl */
#define CTLTYPE 0xf                   /* Mask for the type */
#define CTLTYPE_NODE 1                /* name is a node */
#define CTLTYPE_INT 2                 /* name describes an integer */
#define CTLTYPE_STRING 3              /* name describes a string */
#define CTLTYPE_QUAD 4                /* name describes a 64-bit number */
#define CTLTYPE_OPAQUE 5              /* name describes a structure */
#define CTLTYPE_STRUCT CTLTYPE_OPAQUE /* name describes a structure */

#define CTL_UNSPEC 0  /* unused */
#define CTL_KERN 1    /* "high kernel": proc, limits */
#define CTL_VM 2      /* virtual memory */
#define CTL_VFS 3     /* file system, mount type is next */
#define CTL_NET 4     /* network, see socket.h */
#define CTL_DEBUG 5   /* debugging parameters */
#define CTL_HW 6      /* generic cpu/io */
#define CTL_MACHDEP 7 /* machine dependent */
#define CTL_USER 8    /* user-level */
#define CTL_MAXID 9   /* number of valid top-level ids */

/* limits */
#define __DARWIN_MAXPATHLEN 1024
#define GETDIRENTRIES64_EXTENDED_BUFSIZE 1024
#define MFSTYPENAMELEN  16
#define MAXPATHLEN __DARWIN_MAXPATHLEN

/* snapshot */
#define SNAPSHOT_OP_CREATE 0x01
#define SNAPSHOT_OP_DELETE 0x02
#define SNAPSHOT_OP_RENAME 0x03
#define SNAPSHOT_OP_MOUNT  0x04
#define SNAPSHOT_OP_REVERT 0x05
#define SNAPSHOT_OP_ROOT   0x06

/* fsopt */
#define FSOPT_NOFOLLOW          0x00000001
#define FSOPT_NOINMEMUPDATE     0x00000002
#define FSOPT_REPORT_FULLSIZE   0x00000004
#define FSOPT_PACK_INVAL_ATTRS  0x00000008
#define FSOPT_EXCHANGE_DATA_ONLY 0x0000010
#define FSOPT_ATTR_CMN_EXTENDED 0x00000020
#define FSOPT_LIST_SNAPSHOT     0x00000040
#define FSOPT_NOFIRMLINKPATH     0x00000080
#define FSOPT_FOLLOW_FIRMLINK    0x00000100
#define FSOPT_RETURN_REALDEV     0x00000200
#define FSOPT_ISREALFSID         FSOPT_RETURN_REALDEV
#define FSOPT_ISREALFSID         FSOPT_RETURN_REALDEV
#define FSOPT_NOFOLLOW_ANY       0x00000800

/* attrlist */
#define ATTR_CMN_NAME				0x00000001
#define ATTR_CMN_DEVID				0x00000002
#define ATTR_CMN_FSID				0x00000004
#define ATTR_CMN_OBJTYPE			0x00000008
#define ATTR_CMN_OBJTAG				0x00000010
#define ATTR_CMN_OBJID				0x00000020
#define ATTR_CMN_OBJPERMANENTID			0x00000040
#define ATTR_CMN_PAROBJID			0x00000080
#define ATTR_CMN_SCRIPT				0x00000100
#define ATTR_CMN_CRTIME				0x00000200
#define ATTR_CMN_MODTIME			0x00000400
#define ATTR_CMN_CHGTIME			0x00000800
#define ATTR_CMN_ACCTIME			0x00001000
#define ATTR_CMN_BKUPTIME			0x00002000
#define ATTR_CMN_FNDRINFO			0x00004000
#define ATTR_CMN_OWNERID			0x00008000
#define ATTR_CMN_GRPID				0x00010000
#define ATTR_CMN_ACCESSMASK			0x00020000
#define ATTR_CMN_FLAGS				0x00040000
#define ATTR_CMN_RETURNED_ATTRS 		0x80000000	
#define ATTR_BULK_REQUIRED (ATTR_CMN_NAME | ATTR_CMN_RETURNED_ATTRS)

/*
 * File types
 */
#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

#define STDOUT_FILENO 1
#endif
