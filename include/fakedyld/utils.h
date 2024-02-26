#ifndef FAKEDYLD_UTILS_H
#define FAKEDYLD_UTILS_H

#include <paleinfo.h>
#include <stdarg.h>

void spin();
void get_pinfo(struct paleinfo* pinfo_p);
void pinfo_check(struct paleinfo* pinfo_p);
/* declared in assembly */
uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz, uint64_t dstsz);

#ifndef RAW_RAMDISK
#define RAW_RAMDISK "/dev/rmd0"
#endif

#ifndef RAMDISK
#define RAMDISK "/dev/md0"
#endif

#define DARWIN21_ROOTDEV "disk0s1s1"
#define DARWIN22_ROOTDEV "disk1s1"

#define MAX_BOOTARGS_LEN  0x270
#define MAX_KVERSION_LEN  256
#define MAX_OSRELEASE_LEN 16

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

struct systeminfo {
    char bootargs[MAX_BOOTARGS_LEN];
    char kversion[MAX_KVERSION_LEN];
    struct {
        int darwinMajor; /* ex. 22 */
        int darwinMinor; /* ex. 5 */
        int darwinRevision; /* most likely 0 */
    } osrelease;
    int xnuMajor; /* ex. 8796 */
};

static inline int p1_log(const char* format, ...) {
    printf("fakedyld: ");
    va_list va;
    va_start(va, format);
    int ret = vprintf(format, va);
    va_end(va);
    printf("\n");
    return ret;
}
#define LOG(...) p1_log(__VA_ARGS__)
#define CHECK_ERROR(action, msg, ...) do { \
 int check_error_ret = action; \
 if (unlikely(check_error_ret)) { \
  panic(msg ": %d", ##__VA_ARGS__, errno); \
 } \
} while (0)

#define fbi(mnt, dir)                                    \
  do                                                     \
  {                                                      \
    int fbi_ret = mount("bindfs", mnt, MNT_RDONLY, dir); \
    if (fbi_ret != 0)                                    \
    {                                                    \
      panic("cannot bind %s onto %s, err=%d", dir, mnt, errno); \
    }                                                    \
    else                                                 \
    {                                                    \
      panic("bound %s onto %s", dir, mnt);            \
    }                                                    \
  } while (0)

#endif
