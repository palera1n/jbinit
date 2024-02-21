#include <payload_dylib/common.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/kern_memorystatus.h>
#include <xpc/private.h>
#include <sys/sysctl.h>
#include <sys/reboot.h>
#include <pthread.h>
#include <libgen.h>
#include <sys/types.h>

static xpc_object_t sysstatuscheck_task;
static int platform = 0;

#define PAYLOAD_PLIST "/cores/binpack/Library/LaunchDaemons/payload.plist"
#define fakePath(x) ({ \
  char basePath[PATH_MAX]; \
  char* outPath = alloca(PATH_MAX); \
  snprintf(outPath, PATH_MAX, "/System/Library/LaunchDaemons/%s", basename_r(x, basePath)); \
  (outPath);\
  })

void append_daemon_from_plist(xpc_object_t daemonsDict, const char* path) {
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    printf("failed to open plist at %s\n", path);
    return;
  }
  struct stat st;
  int retval = fstat(fd, &st);
  if (retval == -1) {
    printf("failed to fstat %d\n", fd);
    return;
  }
  void* buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE | MAP_FILE, fd, 0);
  if (buf == MAP_FAILED) {
    printf("mmap failed\n");
    return;
  }
  xpc_object_t plist = xpc_create_from_plist(buf, st.st_size);
  if (plist) {
    xpc_dictionary_set_value(daemonsDict, fakePath(path), plist);
    printf("file %s added to launch daemons\n", path);
    xpc_release(plist);
  } else {
    printf("failed to create daemon xpc object\n");
  }

  munmap(buf, st.st_size);
  close(fd);
  return;
}

xpc_object_t (*xpc_dictionary_get_value_orig)(xpc_object_t xdict, const char *key);
xpc_object_t hook_xpc_dictionary_get_value(xpc_object_t dict, const char *key){
  xpc_object_t retval = xpc_dictionary_get_value_orig(dict,key);
  if (!retval) return retval;

  if (strcmp(key,"LaunchDaemons") == 0 && xpc_get_type(retval) == XPC_TYPE_DICTIONARY) {
    append_daemon_from_plist(retval, PAYLOAD_PLIST);
    append_daemon_from_plist(retval, "/cores/binpack/Library/LaunchDaemons/palera1nd.plist");
#ifdef HAVE_DEBUG_SHELL
    append_daemon_from_plist(retval, "/cores/binpack/Library/LaunchDaemons/debug-shell.plist");
#endif
    char dropbearBuf[150];
    snprintf(dropbearBuf, 150, "/cores/binpack/Library/LaunchDaemons/dropbear-%d.plist", platform);
    append_daemon_from_plist(retval, dropbearBuf);
    if (getenv("XPC_USERSPACE_REBOOTED") != NULL) {
      xpc_object_t payloadDict = xpc_dictionary_get_dictionary(retval, fakePath(PAYLOAD_PLIST));
      xpc_object_t payloadArgs = xpc_dictionary_get_array(payloadDict, "ProgramArguments");
      xpc_array_set_string(payloadArgs, XPC_ARRAY_APPEND, "-u");
    }
  } else if (strcmp(key, "sysstatuscheck") == 0 && xpc_get_type(retval) == XPC_TYPE_DICTIONARY) {
    return sysstatuscheck_task;
  } else if (strcmp(key, "Paths") == 0 && xpc_get_type(retval) == XPC_TYPE_ARRAY) {
    if ((pflags & palerain_option_safemode) == 0) {
      if (pflags & palerain_option_rootful) {
        xpc_array_set_string(retval, XPC_ARRAY_APPEND, "/Library/LaunchDaemons");
      } else {
        xpc_array_set_string(retval, XPC_ARRAY_APPEND, "/var/jb/Library/LaunchDaemons");
        xpc_array_set_string(retval, XPC_ARRAY_APPEND, "/var/jb/System/Library/LaunchDaemons");
      }
    }
  }
  return retval;
}

bool (*xpc_dictionary_get_bool_orig)(xpc_object_t dictionary, const char *key);
bool hook_xpc_dictionary_get_bool(xpc_object_t dictionary, const char *key) {
  if (!strcmp(key, "LogPerformanceStatistics")) {
    static int console_fd = -1;
    if (console_fd != -1) return true;
    console_fd = open("/dev/console", O_RDWR | O_CLOEXEC);
    if (console_fd != -1) {
      dup2(console_fd, STDIN_FILENO);
      dup2(console_fd, STDOUT_FILENO);
      dup2(console_fd, STDERR_FILENO);
    }
    return true;
  }
  else return xpc_dictionary_get_bool_orig(dictionary, key);
}

bool (*memorystatus_control_orig)(uint32_t command, int32_t pid, uint32_t flags, void *buffer, size_t buffersize);
bool hook_memorystatus_control(uint32_t command, int32_t pid, uint32_t flags, void *buffer, size_t buffersize) {
  if (command == MEMORYSTATUS_CMD_SET_JETSAM_TASK_LIMIT && pid == 1) {
    flags = 32768;
  }
  return memorystatus_control_orig(command, pid, flags, buffer, buffersize);
}

void InitDaemonHooks(void) {
  sysstatuscheck_task = xpc_dictionary_create(NULL, NULL, 0);
  xpc_object_t programArguments = xpc_array_create(NULL, 0);
  xpc_array_set_string(programArguments, XPC_ARRAY_APPEND, "/cores/payload");
  if(getenv("XPC_USERSPACE_REBOOTED") != NULL) {
    xpc_array_set_string(programArguments, XPC_ARRAY_APPEND, "-u");
  }
  xpc_dictionary_set_bool(sysstatuscheck_task, "PerformAfterUserspaceReboot", true);
  xpc_dictionary_set_bool(sysstatuscheck_task, "RebootOnSuccess", true);
  xpc_dictionary_set_string(sysstatuscheck_task, "Program", "/cores/payload");
  xpc_dictionary_set_value(sysstatuscheck_task, "ProgramArguments", programArguments);
  xpc_release(programArguments);

  platform = get_platform();
#define fd_console STDOUT_FILENO
  if (platform == -1) spin();
#undef fd_console

  MSHookFunction_p(&xpc_dictionary_get_value, (void *)hook_xpc_dictionary_get_value, (void **)&xpc_dictionary_get_value_orig);
  MSHookFunction_p(&xpc_dictionary_get_bool, (void *)hook_xpc_dictionary_get_bool, (void **)&xpc_dictionary_get_bool_orig);
  MSHookFunction_p(&memorystatus_control, (void*)hook_memorystatus_control, (void**)&memorystatus_control_orig);
}
