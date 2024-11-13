#ifndef FAKEDYLD_H
#define FAKEDYLD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <fakedyld/startup.h>
#include <fakedyld/types.h>
#include <fakedyld/syscalls.h>
#include <fakedyld/errno.h>
#include <fakedyld/param.h>
#include <fakedyld/printf.h>
#include <fakedyld/lib.h>
#include <fakedyld/string.h>
#include <fakedyld/utils.h>
#include <fakedyld/rw_file.h>
#include <fakedyld/spawn.h>
#include <paleinfo.h>

#define PLATFORM_IOS 2
#define PLATFORM_TVOS 3
#define PLATFORM_BRIDGEOS 5

extern char** environ;
extern const struct KernelArgs* gKernArgs;
extern void* gPreDyldMH;

#define PRIx64 "llx"

void mountroot(struct paleinfo* pinfo_p, struct systeminfo* sysinfo_p);
void init_cores(struct systeminfo* sysinfo_p, int platform, bool ramdisk_boot);
void patch_dyld(memory_file_handle_t* dyld_handle, int platform);
void check_dyld(const memory_file_handle_t* dyld_handle);
int get_platform(const memory_file_handle_t* dyld_handle);
void prepare_rootfs(struct systeminfo* sysinfo_p, struct paleinfo* pinfo_p, int platform);
void systeminfo(struct systeminfo* sysinfo_p);
void set_fd_console(int fd_console);
void clean_fakefs(char* rootdev);
_Noreturn void panic(char* fmt, ...);
const char* find_realfs(void);
#ifdef FAKEDYLD_ENABLE_LOGGING
#else
#define panic(...) panic("");
#undef printf
#define printf(...)
#endif
#endif
