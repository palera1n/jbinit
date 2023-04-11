#ifndef JBLOADER_H
#define JBLOADER_H
#ifdef JBINIT_H
#error "this header is not to be used in jbinit"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <sys/clonefile.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <mach/mach.h>
#include <stdbool.h>
#include <spawn.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <CommonCrypto/CommonDigest.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <APFS/APFS.h>
#include <APFS/APFSConstants.h>
#include <xpc/xpc.h>
#include "kerninfo.h"

#ifndef RAMDISK
#define RAMDISK "/dev/rmd0"
#endif

#define fakefs_is_in_use checkrain_option_enabled(pinfo.flags, palerain_option_rootful) && !checkrain_option_enabled(pinfo.flags, checkrain_option_force_revert)

#define RB_AUTOBOOT     0
#define RB_PANIC    0x800
#define RB_PANIC_FORCERESET 0x2000
int reboot_np(int howto, const char *message);

#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i) \
  (((i)&0x80ll) ? '1' : '0'),         \
      (((i)&0x40ll) ? '1' : '0'),     \
      (((i)&0x20ll) ? '1' : '0'),     \
      (((i)&0x10ll) ? '1' : '0'),     \
      (((i)&0x08ll) ? '1' : '0'),     \
      (((i)&0x04ll) ? '1' : '0'),     \
      (((i)&0x02ll) ? '1' : '0'),     \
      (((i)&0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
  PRINTF_BINARY_PATTERN_INT8 PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
  PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
  PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
  PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64 \
  PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
  PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)

extern char **environ;

#define jbloader_userspace_rebooted (1 << 0)
#define jbloader_is_sysstatuscheck  (1 << 1)
#define jbloader_is_bakera1nd       (1 << 2)

#define p1ctl_print_info            (1 << 0)

#define HDI_MAGIC 0x1beeffeed
struct HDIImageCreateBlock64
{
  uint64_t magic;
  const void *props;
  uint64_t props_size;
  char padding[0x100 - 24];
};
extern struct kerninfo info;
extern struct paleinfo pinfo;
extern pthread_mutex_t safemode_mutex;

extern unsigned char create_fakefs_sh[];
extern unsigned int create_fakefs_sh_len;
extern char* launchctl_apple[];

typedef int launchctl_cmd_main(xpc_object_t *msg, int argc, char **argv, char **envp, char **apple);

extern char** environ;
extern uint32_t jbloader_flags;
extern uint32_t p1ctl_flags;
extern int dyld_platform;

launchctl_cmd_main bootstrap_cmd;
launchctl_cmd_main load_cmd;
int run(const char *cmd, char *const *args);
int run_async(const char *cmd, char *const *args);
void spin();
int get_kerninfo(struct kerninfo *info, char *rd);
int get_paleinfo(struct paleinfo *info, char *rd);
int mount_dmg(const char *device, const char *fstype, const char *mnt, const int mntopts, bool is_overlay);
int get_boot_manifest_hash(char hash[97]);
int jailbreak_obliterator();
int check_and_mount_dmg();
int check_and_mount_loader();
int load_etc_rc_d();
int init_info();
int create_remove_fakefs();
int sbreload();
void *enable_ssh(void *__unused _);
void *prep_jb_launch(void *__unused _);
int loadDaemons();
void safemode_alert(CFNotificationCenterRef center, void *observer,
                    CFStringRef name, const void *object, CFDictionaryRef userInfo);
int uicache_apps();
void *prep_jb_ui(void *__unused _);
int uicache_loader();
int remount(char *rootdev);
bool get_safemode_spin();
bool set_safemode_spin(bool val);
int enable_non_default_system_apps();
int get_dyld_platform();

const char* str_checkrain_flags(checkrain_option_t opt);
const char* str_palerain_flags(checkrain_option_t opt);
void print_flag_text(uint32_t flags, const char* prefix, const char* (strflags)(checkrain_option_t opt));

int jbloader_launchd(int argc, char* argv[]);
int jbloader_bakera1nd(int argc, char* argv[]);
int jbloader_sysstatuscheck(int argc, char* argv[]);
int jbloader_main(int argc, char *argv[]);
int p1ctl_main(int argc, char *argv[]);
int print_info(int argc, char *argv[]);
int palera1n_flags_main(int argc, char* argv[]);
int checkra1n_flags_main(int argc, char* argv[]);

kern_return_t DeleteAPFSVolumeWithRole(const char* volpath);

#endif
