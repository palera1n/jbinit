#ifndef JBINIT_H
#define JBINIT_H

#if __STDC_HOSTED__ && !defined(__INTELLISENSE__)
#error "this file is for freestanding use"
#endif

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <iso646.h>
#include <float.h>
#include <stdalign.h>
#include <stdnoreturn.h>

#include <kerninfo.h>
#include "printf.h"

#define STDOUT_FILENO 1
#define getpid() msyscall(20)
#define exit(err) msyscall(1, err)
#define fork() msyscall(2)
#define puts(str) printf("%s\n", str)
#define fbi(mnt, dir)                                    \
  do                                                     \
  {                                                      \
    int fbi_ret = mount("bindfs", mnt, MNT_RDONLY, dir); \
    if (fbi_ret != 0)                                    \
    {                                                    \
      printf("cannot bind %s onto %s, err=%d\n", dir, mnt, fbi_ret); \
      spin();                                            \
    }                                                    \
    else                                                 \
    {                                                    \
      printf("bound %s onto %s\n", dir, mnt);            \
    }                                                    \
  } while (0)
#define RAMDISK "/dev/rmd0"

typedef uint32_t kern_return_t;
typedef uint32_t mach_port_t;
typedef uint64_t mach_msg_timeout_t;
typedef int64_t user_ssize_t;
typedef int64_t off_t;
typedef uint64_t user_size_t;
typedef int64_t ssize_t;
typedef int FILE;
typedef enum
{
  /* the __getdirentries64 returned all entries */
  GETDIRENTRIES64_EOF = 1U << 0,
} getdirentries64_flags_t;
// typedef uint64_t size_t;

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x00000200 /* create if nonexistant */
#define O_DIRECTORY 0x00100000
#define O_SYNC 0x0080      /* synch I/O file integrity */
#define O_TRUNC 0x00000400 /* truncate to zero length */
#define O_APPEND        0x00000008      /* set append mode */
#define O_EXCL          0x00000800      /* error if already exists */

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

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

#define MNT_RDONLY 0x00000001
#define MNT_LOCAL 0x00001000
#define MNT_ROOTFS 0x00004000 /* identifies the root filesystem */
#define MNT_UNION 0x00000020
#define MNT_UPDATE 0x00010000  /* not a real mount, just an update */
#define MNT_NOBLOCK 0x00020000 /* don't block unmount if not responding */
#define MNT_RELOAD 0x00040000  /* reload filesystem data */
#define MNT_FORCE 0x00080000   /* force unmount or readonly change */

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

#define __DARWIN_MAXPATHLEN 1024
#define GETDIRENTRIES64_EXTENDED_BUFSIZE 1024
#define MFSTYPENAMELEN  16
#define MAXPATHLEN __DARWIN_MAXPATHLEN

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
typedef struct fsid { int32_t val[2]; } fsid_t;

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

typedef uint32_t uid_t;
struct statfs64 __DARWIN_STRUCT_STATFS64;

extern char slash_fs_slash_orig[];
extern char slash_fs_slash_orig_slash_private[];
extern char slash[];
extern char slash_private[];
extern char ios15_rootdev[], ios16_rootdev[];

extern struct kerninfo info;
extern struct paleinfo pinfo;

extern bool darwin22;
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

/* syscalls */
__attribute__((naked)) kern_return_t thread_switch(mach_port_t new_thread, int option, mach_msg_timeout_t time);
__attribute__((naked)) uint64_t msyscall(uint64_t syscall, ...);
void sleep(int secs);
int sys_dup2(int from, int to);
int stat(void *path, void *ub);
int mkdir(void *path, int mode);
int mount(const char *type, char *path, int flags, void *data);
int unmount(char *path, int flags);
void *mmap(void *addr, size_t length, int prot, int flags, int fd, uint64_t offset);
uint64_t read(int fd, void *cbuf, size_t nbyte);
uint64_t write(int fd, void *cbuf, size_t nbyte);
int close(int fd);
int open(void *path, int flags, int mode);
int execve(char *fname, char *const argv[], char *const envp[]);
int symlink(char* path, char* link);
uint64_t lseek(int fildes, int32_t offset, int whence);
int sys_sysctlbyname(const char *name, size_t namelen, void *old, size_t *oldlenp, void *new, size_t newlen);
ssize_t getdirentries64(int fd, void *buf, size_t bufsize, off_t *position);
int statfs64(char *path, struct statfs64 *buf);
int chroot(char* path);
int chdir(char* path);
int munmap(void* addr, size_t len);
/* end syscalls */

/* libc */
void memset(void *dst, int c, size_t n);
int memcmp(const void *b1, const void *b2, size_t n);
void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
char *strstr(const char *string, char *substring);
char *strcat(char *dest, char *src);
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
/* end libc */

/* info */
int get_kerninfo(struct kerninfo *info, char *rd);
int get_paleinfo(struct paleinfo *info, char *rd);
/* end info */

/* utils */
void read_directory(int fd, void (*dir_cb)(struct dirent *));
void* read_file(char* path, size_t* size);
int write_file(char* path, void* data, size_t size);
void spin();
/* end utils */

/* actual components */
void mount_devfs();
void get_info();
void sancheck();
void pinfo_check(bool* use_fakefs_p, char* bootargs, char* dev_rootdev);
void remount_rdisk(bool use_fakefs, char* dev_rootdev);
void mountroot(char* rootdev, uint64_t rootlivefs, int rootopts);
void mount_cores();
void init_log(const char* dev_rootdev);
void init_cores();
void rootwait(char** rootdev_pp);
void select_root(uint64_t* rootlivefs_p, int* rootopts_p, char** rootdev_p, char* dev_rootdev, bool use_fakefs);
void prepare_rootfs(char* dev_rootdev, bool use_fakefs);
void patch_dyld(void);
/* end components */
#endif
