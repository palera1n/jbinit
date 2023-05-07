#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>
#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <spawn.h>
#include <kerninfo.h>

#define RB_AUTOBOOT     0
#define CHECK_ERROR(action, msg) do { \
 ret = action; \
 if (ret) { \
  dprintf(fd_console, msg ": %d (%s)\n", errno, strerror(errno)); \
  spin(); \
 } \
} while (0)


int reboot_np(int howto, const char *message);

bool do_pspawn_hook = false;
uint32_t pflags = 0;

extern char** environ;
struct dyld_interpose_tuple {
	const void* replacement;
	const void* replacee;
};
void dyld_dynamic_interpose(const struct mach_header* mh, const struct dyld_interpose_tuple array[], size_t count);

int sandbox_check_by_audit_token(audit_token_t au, const char *operation, int sandbox_filter_type, ...);

typedef  void *posix_spawnattr_t;
typedef  void *posix_spawn_file_actions_t;
int posix_spawn(pid_t *, const char *,const posix_spawn_file_actions_t *,const posix_spawnattr_t *,char *const __argv[],char *const __envp[]);

typedef void* xpc_object_t;
typedef void* xpc_type_t;
typedef void* launch_data_t;
//typedef bool (^xpc_dictionary_applier_t)(const char *key, xpc_object_t value);

xpc_object_t xpc_dictionary_create(const char * const *keys, const xpc_object_t *values, size_t count);
void xpc_dictionary_set_uint64(xpc_object_t dictionary, const char *key, uint64_t value);
void xpc_dictionary_set_string(xpc_object_t dictionary, const char *key, const char *value);
int64_t xpc_dictionary_get_int64(xpc_object_t dictionary, const char *key);
xpc_object_t xpc_dictionary_get_value(xpc_object_t dictionary, const char *key);
bool xpc_dictionary_get_bool(xpc_object_t dictionary, const char *key);
void xpc_dictionary_set_fd(xpc_object_t dictionary, const char *key, int value);
void xpc_dictionary_set_bool(xpc_object_t dictionary, const char *key, bool value);
const char *xpc_dictionary_get_string(xpc_object_t dictionary, const char *key);
void xpc_dictionary_set_value(xpc_object_t dictionary, const char *key, xpc_object_t value);
xpc_type_t xpc_get_type(xpc_object_t object);
//bool xpc_dictionary_apply(xpc_object_t xdict, xpc_dictionary_applier_t applier);
int64_t xpc_int64_get_value(xpc_object_t xint);
char *xpc_copy_description(xpc_object_t object);
void xpc_dictionary_set_int64(xpc_object_t dictionary, const char *key, int64_t value);
const char *xpc_string_get_string_ptr(xpc_object_t xstring);
xpc_object_t xpc_array_create(const xpc_object_t *objects, size_t count);
xpc_object_t xpc_string_create(const char *string);
size_t xpc_dictionary_get_count(xpc_object_t dictionary);
void xpc_array_append_value(xpc_object_t xarray, xpc_object_t value);

#define XPC_ARRAY_APPEND ((size_t)(-1))
#define XPC_ERROR_CONNECTION_INVALID XPC_GLOBAL_OBJECT(_xpc_error_connection_invalid)
#define XPC_ERROR_TERMINATION_IMMINENT XPC_GLOBAL_OBJECT(_xpc_error_termination_imminent)
#define XPC_TYPE_ARRAY (&_xpc_type_array)
#define XPC_TYPE_BOOL (&_xpc_type_bool)
#define XPC_TYPE_DICTIONARY (&_xpc_type_dictionary)
#define XPC_TYPE_ERROR (&_xpc_type_error)
#define XPC_TYPE_STRING (&_xpc_type_string)


extern const struct _xpc_dictionary_s _xpc_error_connection_invalid;
extern const struct _xpc_dictionary_s _xpc_error_termination_imminent;
extern const struct _xpc_type_s _xpc_type_array;
extern const struct _xpc_type_s _xpc_type_bool;
extern const struct _xpc_type_s _xpc_type_dictionary;
extern const struct _xpc_type_s _xpc_type_error;
extern const struct _xpc_type_s _xpc_type_string;

#define DYLD_INTERPOSE(_replacment,_replacee) \
__attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
__attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };

void spin() {
  while(1) {sleep(5);}
}

/*
  Launch our Daemon *correctly*
*/
xpc_object_t hook_xpc_dictionary_get_value(xpc_object_t dict, const char *key){
  xpc_object_t retval = xpc_dictionary_get_value(dict,key);
  if (getpid() != 1) return retval;
  if (strcmp(key,"LaunchDaemons") == 0) {
    xpc_object_t submitJob = xpc_dictionary_create(NULL, NULL, 0);
    xpc_object_t programArguments = xpc_array_create(NULL, 0);

    xpc_array_append_value(programArguments, xpc_string_create("/cores/jbloader"));
    if(getenv("XPC_USERSPACE_REBOOTED") != NULL) {
      xpc_array_append_value(programArguments, xpc_string_create("-u"));
    }
    xpc_array_append_value(programArguments, xpc_string_create("-j"));

    xpc_dictionary_set_bool(submitJob, "KeepAlive", false);
    xpc_dictionary_set_bool(submitJob, "RunAtLoad", true);
    xpc_dictionary_set_string(submitJob, "ProcessType", "Interactive");
    xpc_dictionary_set_string(submitJob, "UserName", "root");
    xpc_dictionary_set_string(submitJob, "Program", "/cores/jbloader");
    xpc_dictionary_set_string(submitJob, "StandardInPath", "/dev/console");
    xpc_dictionary_set_string(submitJob, "StandardOutPath", "/dev/console");
    xpc_dictionary_set_string(submitJob, "StandardErrorPath", "/dev/console");
    xpc_dictionary_set_string(submitJob, "Label", "in.palera.jbloader");
    xpc_dictionary_set_value(submitJob, "ProgramArguments", programArguments);
#ifdef DEV_BUILD
    xpc_object_t environmentVariables = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(environmentVariables, "DYLD_INSERT_LIBRARIES", "/cores/xpchook.dylib");
    xpc_dictionary_set_value(submitJob, "EnvironmentVariables", environmentVariables);
#endif
    xpc_dictionary_set_value(retval, "/System/Library/LaunchDaemons/in.palera.jbloader.plist", submitJob);
  } else if (strcmp(key, "sysstatuscheck") == 0) {
    xpc_object_t programArguments = xpc_array_create(NULL, 0);
    xpc_array_append_value(programArguments, xpc_string_create("/cores/jbloader"));
    if(getenv("XPC_USERSPACE_REBOOTED") != NULL) {
      xpc_array_append_value(programArguments, xpc_string_create("-u"));
    }
    xpc_array_append_value(programArguments, xpc_string_create("-s"));
    xpc_object_t newTask = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_bool(newTask, "PerformAfterUserspaceReboot", true);
    xpc_dictionary_set_bool(newTask, "RebootOnSuccess", true);
    xpc_dictionary_set_string(newTask, "Program", "/cores/jbloader");
    xpc_dictionary_set_value(newTask, "ProgramArguments", programArguments);
    return newTask;
  }
  return retval;
}
DYLD_INTERPOSE(hook_xpc_dictionary_get_value, xpc_dictionary_get_value);

bool hook_xpc_dictionary_get_bool(xpc_object_t dictionary, const char *key) {
  if (!strcmp(key, "LogPerformanceStatistics")) return true;
  else return xpc_dictionary_get_bool(dictionary, key);
}
DYLD_INTERPOSE(hook_xpc_dictionary_get_bool, xpc_dictionary_get_bool);

int hook_posix_spawnp_launchd(pid_t *pid,
                      const char *path,
                      const posix_spawn_file_actions_t *action,
                      const posix_spawnattr_t *attr,
                      char *const argv[], char *envp[]) {
  if (argv[1] == NULL || strcmp(argv[1], "com.apple.cfprefsd.xpc.daemon"))
    return posix_spawnp(pid, path, action, attr, argv, envp);
    char *inj = NULL;
    int envcnt = 0;
    while (envp[envcnt] != NULL) {
      envcnt++;
    }
    char** newenvp = malloc((envcnt + 2) * sizeof(char **));
    int j = 0;
    char* currentenv = NULL;
    for (int i = 0; i < envcnt; i++){
      if (strstr(envp[j], "DYLD_INSERT_LIBRARIES") != NULL) {
        currentenv = envp[j];
        continue;
      }
        newenvp[i] = envp[j];
        j++;
    }
            
    char *newlib = "/cores/payload.dylib";
    if(currentenv) {
      size_t inj_len = strlen(currentenv) + 1 + strlen(newlib) + 1;
      inj = malloc(inj_len);
      if (inj == NULL) {
        perror(NULL);
        abort();
      }
      snprintf(inj, inj_len, "%s:%s", currentenv, newlib);
    } else {
      size_t inj_len = strlen("DYLD_INSERT_LIBRARIES=") + strlen(newlib) + 1;
      inj = malloc(inj_len);
      if (inj == NULL) {
        perror(NULL);
        abort();
      }
      snprintf(inj, inj_len, "DYLD_INSERT_LIBRARIES=%s", newlib);
    }
    newenvp[j] = inj;
    newenvp[j + 1] = NULL;
            
    int ret = posix_spawnp(pid, path, action, attr, argv, newenvp);
    if (inj != NULL) free(inj);
    if (currentenv != NULL) free(currentenv);
    return ret;
}
int hook_posix_spawnp_xpcproxy(pid_t *pid,
                      const char *path,
                      const posix_spawn_file_actions_t *action,
                      const posix_spawnattr_t *attr,
                      char *const argv[], char *envp[])
{
    if(strcmp(argv[0], "/usr/sbin/cfprefsd")) {
        return posix_spawnp(pid, path, action, attr, argv, envp);
    }
    int envcnt = 0;
    while (envp[envcnt] != NULL)
    {
        envcnt++;
    }

    char **newenvp = malloc((envcnt + 2) * sizeof(char **));
    if (newenvp == NULL) abort();
    int j = 0;
    char *currentenv = NULL;
    for (int i = 0; i < envcnt; i++)
    {
        if (strstr(envp[j], "DYLD_INSERT_LIBRARIES") != NULL)
        {
            currentenv = envp[j];
            continue;
        }
        newenvp[i] = envp[j];
        j++;
    }

    char *newlib = "/cores/binpack/usr/lib/rootlesshooks.dylib";
    char *inj = NULL;
    if (currentenv)
    {
        size_t inj_len = strlen(currentenv) + 1 + strlen(newlib) + 1;
        inj = malloc(inj_len);
        if (inj == NULL) abort();
        snprintf(inj, inj_len, "%s:%s", currentenv, newlib);
    }
    else
    {
        size_t inj_len = strlen("DYLD_INSERT_LIBRARIES=") + strlen(newlib) + 1;
        inj = malloc(inj_len);
        if (inj == NULL) abort();
        snprintf(inj, inj_len, "DYLD_INSERT_LIBRARIES=%s", newlib);
    }
    newenvp[j] = inj;
    newenvp[j + 1] = NULL;

    int ret = posix_spawnp(pid, path, action, attr, argv, newenvp);
    free(inj);
    free(newenvp);
    return ret;
}
int hook_posix_spawnp(pid_t *pid,
                      const char *path,
                      const posix_spawn_file_actions_t *action,
                      const posix_spawnattr_t *attr,
                      char *const argv[], char *envp[]) {
  /* do_pspawn_hook only works in launchd */
  if (do_pspawn_hook && getpid() == 1) {
    return hook_posix_spawnp_launchd(pid, path, action, attr, argv, envp);
  }
  char exe_path[PATH_MAX];
  uint32_t bufsize = PATH_MAX;
  int ret = _NSGetExecutablePath(exe_path, &bufsize);
  if (ret) abort();
  if (!strcmp("/usr/sbin/cfprefsd", path) && getppid() == 1 && !strcmp("/usr/libexec/xpcproxy", exe_path)) {
    return hook_posix_spawnp_xpcproxy(pid, path, action, attr, argv, envp);
  } else {
    return posix_spawnp(pid, path, action, attr, argv, envp);
  }
}
DYLD_INTERPOSE(hook_posix_spawnp, posix_spawnp);

void DoNothingHandler(int __unused _) {}

uint32_t get_flags_from_p1ctl(int fd_console) {
  if (access("/cores/binpack/usr/sbin/p1ctl", F_OK) != 0) {
    dprintf(fd_console, "could not access p1ctl: %d (%s)\n", errno, strerror(errno));
    spin();
  }
  int flides[2];
  int ret;
  CHECK_ERROR(pipe(flides), "pipe failed");
  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);
  posix_spawn_file_actions_adddup2(&actions, flides[1], STDOUT_FILENO);
  posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/console", O_WRONLY, 0);
  /* spawn p1ctl */
  pid_t pid;
  CHECK_ERROR(posix_spawnp(&pid, "/cores/binpack/usr/sbin/p1ctl", &actions, NULL, (char*[]){"p1ctl","palera1n_flags",NULL}, NULL), "could not spawn p1ctl");
  ssize_t didRead;
  int status;
  char p1flags_buf[16];
  /* in principle the child may block indefinitely without read(), so we read() then waitpid() */
  didRead = read(flides[0], p1flags_buf, 15);
  if (didRead < 0) {
    dprintf(fd_console, "read failed: %d (%s)\n", errno, strerror(errno));
    spin();
  }
  waitpid(pid, &status, 0);
  close(flides[0]);
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    dprintf(fd_console, "p1ctl waitpid status: %d\n", status);
    spin();
  }
  p1flags_buf[15] = '\0';
  pflags = (uint32_t)strtoul(p1flags_buf, NULL, 16);
  dprintf(fd_console, "pflags: %u\n", pflags);
  return pflags;
}

__attribute__((constructor))
static void customConstructor(int argc, const char **argv){
  if (getpid() != 1) return;
  int fd_console = open("/dev/console",O_RDWR,0);
  dprintf(fd_console,"================ Hello from payload.dylib ================ \n");
  signal(SIGBUS, DoNothingHandler);
  /* make binpack available */
  pid_t pid;
  int ret;
  CHECK_ERROR(posix_spawn(&pid, "/cores/jbloader", NULL, NULL, (char*[]){"/cores/jbloader","-f",NULL},environ), "could not spawn jbloader");
  int status;
  waitpid(pid, &status, 0);
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    dprintf(fd_console, "jbloader quit unexpectedly\n");
    spin();
  }
  get_flags_from_p1ctl(fd_console);
  if ((pflags & palerain_option_setup_rootful)) {
    int32_t initproc_started = 1;
    CHECK_ERROR(sysctlbyname("kern.initproc_spawned", NULL, NULL, &initproc_started, 4), "sysctl kern.initproc_spawned=1");
    CHECK_ERROR(unmount("/cores/binpack/Applications", MNT_FORCE), "unmount(/cores/binpack/Applications)");
    CHECK_ERROR(unmount("/cores/binpack", MNT_FORCE), "unmount(/cores/binpack)");
    dprintf(fd_console, "Rebooting\n");
    reboot_np(RB_AUTOBOOT, NULL);
    sleep(5);
    dprintf(fd_console, "reboot timed out\n");
    spin();
  }
  if ((pflags & palerain_option_rootful) == 0) do_pspawn_hook = true;
  dprintf(fd_console, "do_pspawn_hook: %d\n", do_pspawn_hook);
  dprintf(fd_console,"========= Goodbye from payload.dylib constructor ========= \n");
  close(fd_console);
}
