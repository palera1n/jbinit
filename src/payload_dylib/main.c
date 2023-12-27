#define __APPLE_API_PRIVATE

#include <stdio.h>
#include <paleinfo.h>
#include <xpc/xpc.h>
#include <stdint.h>
#include <spawn.h>
#include <errno.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <payload_dylib/common.h>
#include <payload_dylib/crashreporter.h>
#include <sandbox/private.h>
#include <dlfcn.h>

#define HOOK_DYLIB_PATH "/cores/binpack/usr/lib/systemhook.dylib"
#define ELLEKIT_PATH "/cores/binpack/usr/lib/libellekit.dylib"

uint64_t pflags;
int (*spawn_hook_common_p)(pid_t *restrict pid, const char *restrict path,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict],
					   void *pspawn_org) = NULL;

void _spin() {
  fprintf(stderr, "An error occured");
  while (1) {
    sleep(5);
  }
}

char* generate_sandbox_extensions(void) {
  return sandbox_extension_issue_mach("com.apple.security.exception.mach-lookup.global-name", "in.palera.palera1nd.systemwide", 0);
}

uint64_t load_pflags(void) {
  pflags = strtoull(getenv("JB_PINFO_FLAGS"), NULL, 0);
  return pflags;
}

__attribute__((constructor))void launchd_hook_main(void) {
  if (getpid() != 1) return;
  int fd_console = open("/dev/console",O_RDWR|O_SYNC,0);
  if (fd_console == -1) {
    char errMsg[1024];
    snprintf(errMsg, 1024, "payload.dylib cannot open /dev/console: %d (%s)", errno, strerror(errno));
    reboot_np(RB_PANIC, errMsg);
  }
  
  dup2(fd_console, STDIN_FILENO);
  dup2(fd_console, STDOUT_FILENO);
  dup2(fd_console, STDERR_FILENO);

  printf("=========== hello from payload.dylib ===========\n");
  crashreporter_start();
  setenv("JB_SANDBOX_EXTENSIONS", generate_sandbox_extensions(), 1);
  load_pflags();
  pid_t pid;
  int ret, status;
  CHECK_ERROR(posix_spawn(&pid, "/cores/payload", NULL, NULL, (char*[]){"/cores/payload","-f",NULL},environ), "could not spawn payload");
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    if (WEXITSTATUS(status) != 0) {
      fprintf(stderr, "/cores/payload exited with status code %d\n", WEXITSTATUS(status));
      spin();
    }
  } else if (WIFSIGNALED(status)) {
    fprintf(stderr, "/cores/payload exited abnormally: signal %d\n", WTERMSIG(status));
    spin();
  } else {
    spin();
  }

  bootscreend_main();
  void* systemhook_handle = dlopen(HOOK_DYLIB_PATH, RTLD_NOW);
  if (!systemhook_handle) {
    fprintf(stderr, "dlopen systemhook failed: %s\n", dlerror());
    spin();
  }
  spawn_hook_common_p = dlsym(systemhook_handle, "spawn_hook_common");
  if (!spawn_hook_common_p) {
    fprintf(stderr, "symbol spawn_hook_common not found in " HOOK_DYLIB_PATH ": %s\n", dlerror());
    spin();
  }

  void* ellekit_handle = dlopen(ELLEKIT_PATH, RTLD_NOW);
  if (!ellekit_handle) {
    fprintf(stderr, "dlopen ellekit failed: %s\n", dlerror());
    spin();
  }

  MSHookFunction_p = dlsym(ellekit_handle, "MSHookFunction");
  if (!MSHookFunction_p) {
    fprintf(stderr, "symbol MSHookFunction not found in " ELLEKIT_PATH ": %s\n", dlerror());
    spin();
  }
  
  initSpawnHooks();
  InitDaemonHooks();

  if ((pflags & palerain_option_setup_rootful)) {
    int32_t initproc_started = 1;
    CHECK_ERROR(sysctlbyname("kern.initproc_spawned", NULL, NULL, &initproc_started, 4), "sysctl kern.initproc_spawned=1");
    CHECK_ERROR(unmount("/cores/binpack/Applications", MNT_FORCE), "unmount(/cores/binpack/Applications)");
    CHECK_ERROR(unmount("/cores/binpack", MNT_FORCE), "unmount(/cores/binpack)");
    printf("Rebooting\n");
    kern_return_t failed = host_reboot(mach_host_self(), 0x1000);
    fprintf(stderr, "reboot failed: %d (%s)\n", failed, mach_error_string(failed));
    spin();
  }
  printf("=========== bye from payload.dylib ===========\n");

  close(fd_console);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}
