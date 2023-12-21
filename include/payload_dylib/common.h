#ifndef PAYLOAD_DYLIB_CRASHREPORTER_H
#define PAYLOAD_DYLIB_CRASHREPORTER_H

#include <stdint.h>
#include <dyld-interpose.h>
#include <payload_dylib/crashreporter.h>
#include <paleinfo.h>
#include <spawn.h>
#include <xpc/xpc.h>

#define spin() _spin(fd_console)

#define CHECK_ERROR(action, msg) do { \
 ret = action; \
 if (ret) { \
  dprintf(fd_console, msg ": %d (%s)\n", errno, strerror(errno)); \
  spin(); \
 } \
} while (0)

extern uint64_t pflags;
extern char** environ;
void _spin(int fd_console);
int get_platform();

int (*spawn_hook_common_p)(pid_t *restrict pid, const char *restrict path,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict],
					   void *pspawn_org);

void (*MSHookFunction_p)(void *symbol, void *replace, void **result);
void initSpawnHooks(void);
void InitDaemonHooks(void);
int bootscreend_main(void);

#endif
