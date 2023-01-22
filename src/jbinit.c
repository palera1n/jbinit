
#include <stdint.h>
#include "printf.h"
#include "kerninfo.h"

char ios15_rootdev[] = "/dev/disk0s1s1";
char ios16_rootdev[] = "/dev/disk1s1";

asm(
    ".globl __dyld_start\n"
    ".align 4\n"
    "__dyld_start:\n"
    "movn x8, #0xf\n"
    "mov x7, sp\n"
    "and x7, x7, x8\n"
    "mov sp, x7\n"
    "bl _main\n"
    "movz x16, #0x1\n"
    "svc #0x80\n");

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
      printf("cannot bind %s onto %s\n", dir, mnt);      \
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
#define O_SYNC 0x0080 /* synch I/O file integrity */

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define PROT_NONE 0x00  /* [MC2] no permissions */
#define PROT_READ 0x01  /* [MC2] pages can be read */
#define PROT_WRITE 0x02 /* [MC2] pages can be written */
#define PROT_EXEC 0x04  /* [MC2] pages can be executed */

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
char slash_fs_slash_orig[] = "/fs/orig";
char slash_fs_slash_orig_slash_private[] = "/fs/orig/private";
char slash[] = "/";
char slash_private[] = "/private";
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

__attribute__((naked)) kern_return_t thread_switch(mach_port_t new_thread, int option, mach_msg_timeout_t time)
{
  asm(
      "movn x16, #0x3c\n"
      "svc 0x80\n"
      "ret\n");
}

__attribute__((naked)) uint64_t msyscall(uint64_t syscall, ...)
{
  asm(
      "mov x16, x0\n"
      "ldp x0, x1, [sp]\n"
      "ldp x2, x3, [sp, 0x10]\n"
      "ldp x4, x5, [sp, 0x20]\n"
      "ldp x6, x7, [sp, 0x30]\n"
      "svc 0x80\n"
      "ret\n");
}

void sleep(int secs)
{
  thread_switch(0, 2, secs * 1000);
}

int sys_dup2(int from, int to)
{
  return msyscall(90, from, to);
}

int stat(void *path, void *ub)
{
  return msyscall(188, path, ub);
}

int mkdir(void *path, int mode)
{
  return msyscall(136, path, mode);
}

int chroot(void *path)
{
  return msyscall(61, path);
}

int mount(char *type, char *path, int flags, void *data)
{
  return msyscall(167, type, path, flags, data);
}

int unmount(char *path, int flags)
{
  return msyscall(159, path, flags);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, uint64_t offset)
{
  return (void *)msyscall(197, addr, length, prot, flags, fd, offset);
}

uint64_t read(int fd, void *cbuf, size_t nbyte)
{
  return msyscall(3, fd, cbuf, nbyte);
}

uint64_t write(int fd, void *cbuf, size_t nbyte)
{
  return msyscall(4, fd, cbuf, nbyte);
}

int close(int fd)
{
  return msyscall(6, fd);
}

int open(void *path, int flags, int mode)
{
  return msyscall(5, path, flags, mode);
}

int execve(char *fname, char *const argv[], char *const envp[])
{
  return msyscall(59, fname, argv, envp);
}

void _putchar(char character)
{
  static size_t chrcnt = 0;
  static char buf[0x100];
  buf[chrcnt++] = character;
  if (character == '\n' || chrcnt == sizeof(buf))
  {
    write(STDOUT_FILENO, buf, chrcnt);
    chrcnt = 0;
  }
}

uint64_t lseek(int fildes, int32_t offset, int whence)
{
  return msyscall(199, fildes, offset, whence);
}

int sys_sysctlbyname(const char *name, size_t namelen, void *old, size_t *oldlenp, void *new, size_t newlen)
{
  return msyscall(274, name, namelen, old, oldlenp, new, newlen);
}

ssize_t getdirentries64(int fd, void *buf, size_t bufsize, off_t *position)
{
  return msyscall(344, fd, buf, bufsize, position);
}

void spin()
{
  puts("jbinit DIED!");
  while (1)
  {
    sleep(5);
  }
}

void memcpy(void *dst, void *src, size_t n)
{
  uint8_t *s = (uint8_t *)src;
  uint8_t *d = (uint8_t *)dst;
  for (size_t i = 0; i < n; i++)
    *d++ = *s++;
}

void memset(void *dst, int c, size_t n)
{
  uint8_t *d = (uint8_t *)dst;
  for (size_t i = 0; i < n; i++)
    *d++ = c;
}

int get_kerninfo(struct kerninfo *info, char *rd)
{
  uint32_t size = sizeof(struct kerninfo);
  uint32_t ramdisk_size_actual;
  int fd = open(rd, O_RDONLY, 0);
  read(fd, &ramdisk_size_actual, 4);
  lseek(fd, (long)(ramdisk_size_actual), SEEK_SET);
  int64_t didread = read(fd, info, sizeof(struct kerninfo));
  if ((unsigned long)didread != sizeof(struct kerninfo) || info->size != (uint64_t)sizeof(struct kerninfo))
  {
    return -1;
  }
  close(fd);
  return 0;
}

int get_paleinfo(struct paleinfo *info, char *rd)
{
  uint32_t ramdisk_size_actual;
  int fd = open(rd, O_RDONLY, 0);
  read(fd, &ramdisk_size_actual, 4);
  lseek(fd, (long)(ramdisk_size_actual) + 0x1000L, SEEK_SET);
  int64_t didread = read(fd, info, sizeof(struct paleinfo));
  if ((unsigned long)didread != sizeof(struct paleinfo))
  {
    return -1;
  }
  if (info->magic != PALEINFO_MAGIC)
  {
    printf("Detected corrupted paleinfo!\n");
    return -1;
  }
  if (info->version != 1)
  {
    printf("Unsupported paleinfo %u (expected 1)\n", info->version);
    return -1;
  }
  close(fd);
  return 0;
}

size_t strlen(const char *str)
{
  size_t len = 0;
  while (*str++)
  {
    len++;
  }
  return len;
}

char *strcat(char *dest, char *src)
{
  char *dest_end = dest + strlen(dest);
  for (uint64_t i = 0; src[i] != '\0'; i++)
  {
    *dest_end = src[i];
    dest_end++;
  }
  dest_end = NULL;
  return dest;
}

int main()
{
  int fd_console = open("/dev/console", O_RDWR | O_SYNC, 0);
  sys_dup2(fd_console, 0);
  sys_dup2(fd_console, 1);
  sys_dup2(fd_console, 2);
  char statbuf[0x400];

  puts("================ Hello from jbinit ================");

  int fd_jbloader = 0;
  fd_jbloader = open("/sbin/launchd", O_RDONLY, 0);
  if (fd_jbloader == -1)
  {
    spin();
  }
  size_t jbloader_size = msyscall(199, fd_jbloader, 0, SEEK_END);
  msyscall(199, fd_jbloader, 0, SEEK_SET);
  void *jbloader_data = mmap(NULL, (jbloader_size & ~0x3fff) + 0x4000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (jbloader_data == (void *)-1)
  {
    spin();
  }
  int didread = read(fd_jbloader, jbloader_data, jbloader_size);
  close(fd_jbloader);

  int fd_dylib = 0;
  fd_dylib = open("/jbin/jb.dylib", O_RDONLY, 0);
  if (fd_dylib == -1)
  {
    spin();
  }
  size_t dylib_size = msyscall(199, fd_dylib, 0, SEEK_END);
  msyscall(199, fd_dylib, 0, SEEK_SET);
  void *dylib_data = mmap(NULL, (dylib_size & ~0x3fff) + 0x4000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (dylib_data == (void *)-1)
  {
    spin();
  }
  didread = read(fd_dylib, dylib_data, dylib_size);
  close(fd_jbloader);

  puts("Checking for roots");
  char *rootdev;
  {
    while (stat(ios15_rootdev, statbuf) && stat(ios16_rootdev, statbuf))
    {
      puts("waiting for roots...");
      sleep(1);
    }
  }
  if (stat(ios15_rootdev, statbuf))
  {
    rootdev = ios16_rootdev;
  }
  else
    rootdev = ios15_rootdev;
  printf("Got rootfs %s\n", rootdev);

  puts("mounting devfs");
  {
    char *path = "devfs";
    int err = mount("devfs", "/dev/", 0, path);
    if (!err)
    {
      puts("mount devfs OK");
    }
    else
    {
      printf("mount devfs FAILED with err=%d!\n", err);
      spin();
    }
  }
  struct kerninfo info;
  {
    int err = get_kerninfo(&info, RAMDISK);
    if (err)
    {
      printf("cannot get kerninfo!");
      spin();
    }
  }
  struct paleinfo pinfo;
  {
    int err = get_paleinfo(&pinfo, RAMDISK);
    if (err)
    {
      printf("cannot get paleinfo!");
      spin();
    }
  }
  uint32_t rootlivefs;
  int rootopts = MNT_RDONLY;
  if (checkrain_option_enabled(checkrain_option_bind_mount, info.flags))
  {
    printf("bind mounts are enabled\n");
    rootlivefs = 0;
  }
  else
  {
    printf("WARNING: BIND MOUNTS ARE DISABLED!");
    rootopts |= MNT_UNION;
    rootlivefs = 1;
  }
  char dev_rootdev[0x20] = "/dev/";
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
  {
    strcat(dev_rootdev, pinfo.rootdev);
    rootdev = dev_rootdev;
    rootlivefs = 1;
  }
  else
  {
    // rootopts |= MNT_RDONLY;
  }
  {
    char *path = dev_rootdev;
    int err = mount("apfs", "/", MNT_UPDATE | MNT_RDONLY, &path);
    if (!err)
    {
      puts("remount rdisk OK");
    }
    else
    {
      puts("remount rdisk FAIL");
    }
  }
  {
    char buf[0x100];
    struct apfs_mountarg
    {
      char *path;
      uint64_t _null;
      uint64_t mountAsRaw;
      uint32_t _pad;
      char snapshot[0x100];
    } arg = {
        rootdev,
        0,
        rootlivefs, // 1 mount without snapshot, 0 mount snapshot
        0,
    };
    int err = 0;
  retry_rootfs_mount:
    puts("mounting rootfs");
    err = mount("apfs", "/", rootopts, &arg);
    if (!err)
    {
      puts("mount rootfs OK");
    }
    else
    {
      printf("mount rootfs %s FAILED with err=%d!\n", rootdev, err);
      sleep(1);
      // spin();
    }

    if (stat("/sbin/fsck", statbuf))
    {
      printf("stat %s FAILED with err=%d!\n", "/sbin/fsck", err);
      sleep(1);
      goto retry_rootfs_mount;
    }
    else
    {
      printf("stat %s OK\n", "/sbin/fsck");
    }

    puts("mounting devfs again");
    {
      char *path = "devfs";
      int err = mount("devfs", "/dev/", 0, path);
      if (!err)
      {
        puts("mount devfs again OK");
      }
      else
      {
        printf("mount devfs again FAILED with err=%d!\n", err);
        spin();
      }
    }
    struct tmpfs_mountarg
    {
      uint64_t max_pages;
      uint64_t max_nodes;
      uint8_t case_insensitive;
    };
    {
      int err = 0;
      int64_t pagesize;
      unsigned long pagesize_len = sizeof(pagesize);
      err = sys_sysctlbyname("hw.pagesize", sizeof("hw.pagesize"), &pagesize, &pagesize_len, NULL, 0);
      if (err != 0)
      {
        printf("cannot get pagesize, err=%d", err);
        spin();
      }
      printf("system page size: %lld\n", pagesize);
      {
        struct tmpfs_mountarg arg = {.max_pages = (1572864 / pagesize), .max_nodes = UINT8_MAX, .case_insensitive = 0};
        err = mount("tmpfs", "/cores", 0, &arg);
        if (err != 0)
        {
          printf("cannot mount tmpfs onto /cores, err=%d", err);
          spin();
        }
      }
      puts("mounted tmpfs onto /cores");
      err = mkdir("/cores/binpack", 0755);
      if (err)
      {
        printf("mkdir(/cores/binpack) FAILED with err %d\n", err);
      }
      if (stat("/cores/binpack", statbuf))
      {
        printf("stat %s FAILED with err=%d!\n", "/cores/binpack", err);
        spin();
      }
      else
        puts("created /cores/binpack");
    }
    puts("deploying jbloader");
    fd_jbloader = open("/cores/jbloader", O_WRONLY | O_CREAT, 0755);
    printf("jbloader write fd=%d\n", fd_jbloader);
    if (fd_jbloader == -1)
    {
      puts("Failed to open /cores/jbloader for writing");
      spin();
    }
    int didwrite = write(fd_jbloader, jbloader_data, jbloader_size);
    printf("didwrite=%d\n", didwrite);
    close(fd_jbloader);

    puts("deploying jb.dylib");
    fd_dylib = open("/cores/jb.dylib", O_WRONLY | O_CREAT, 0755);
    printf("jb.dylib write fd=%d\n", fd_dylib);
    if (fd_dylib == -1)
    {
      puts("Failed to open /cores/jb.dylib for writing");
      spin();
    }
    didwrite = write(fd_dylib, dylib_data, dylib_size);
    printf("didwrite=%d\n", didwrite);
    close(fd_dylib);
  }
  {
    char **argv = (char **)jbloader_data;
    char **envp = argv + 2;
    char *strbuf = (char *)(envp + 2);
    argv[0] = strbuf;
    argv[1] = NULL;
    memcpy(strbuf, "/cores/jbloader", sizeof("/cores/jbloader"));
    strbuf += sizeof("/cores/jbloader");
    int err = execve(argv[0], argv, NULL);
    if (err)
    {
      printf("execve FAILED with err=%d!\n", err);
      spin();
    }
  }
  puts("FATAL: shouldn't get here!");
  spin();

  return 0;
}
