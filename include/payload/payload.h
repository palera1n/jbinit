#ifndef PAYLOAD_PAYLOAD_H
#define PAYLOAD_PAYLOAD_H

#include <inttypes.h>
#include <stdbool.h>
#include <paleinfo.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>
#include <os/log.h>

#ifndef __OBJC__
void NSLog(CFStringRef, ...);
#endif

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#define CHECK_ERROR(action, loop, msg, ...) do { \
 {int ___CHECK_ERROR_ret = (action); \
 if (unlikely(___CHECK_ERROR_ret)) { \
  fprintf(stderr, msg ": %d (%s)\n", ##__VA_ARGS__, errno, strerror(errno)); \
  if (loop) spin(); \
 }} \
} while (0)

#define P1CTL_UPCALL_JBD_WITH_ERR_CHECK(varname, cmd) \
    xpc_object_t varname = jailbreak_send_jailbreakd_command_with_reply_sync(cmd); \
    if (unlikely(xpc_get_type(varname) == XPC_TYPE_ERROR)) { \
        char* desc = xpc_copy_description(varname); \
        fprintf(stderr, "jailbreakd upcall failed: %s\n", desc); \
        fprintf(stderr, "%s\n", desc); \
        free(desc); \
        return -1; \
    }

extern os_log_t palera1nd_log;

enum {    // Legal level values for CFLog()
    kCFLogLevelEmergency = 0,
    kCFLogLevelAlert = 1,
    kCFLogLevelCritical = 2,
    kCFLogLevelError = 3,
    kCFLogLevelWarning = 4,
    kCFLogLevelNotice = 5,
    kCFLogLevelInfo = 6,
    kCFLogLevelDebug = 7,
};
CF_EXPORT void CFLog(int32_t level, CFStringRef format, ...);

#ifdef DEV_BUILD
#define PALERA1ND_LOG_DEBUG(a, ...) CFLog(kCFLogLevelNotice, CFSTR(a), ##__VA_ARGS__)
#else
#define PALERA1ND_LOG_DEBUG(a, ...)
#endif

#define PALERA1ND_LOG_ERROR(a, ...) CFLog(kCFLogLevelNotice, CFSTR(a), ##__VA_ARGS__)
#define PALERA1ND_LOG_FAULT(a, ...) CFLog(kCFLogLevelNotice, CFSTR(a), ##__VA_ARGS__)
#define PALERA1ND_LOG_INFO(a, ...) CFLog(kCFLogLevelNotice, CFSTR(a), ##__VA_ARGS__)
#define PALERA1ND_LOG(a, ...) CFLog(kCFLogLevelNotice, CFSTR(a), ##__VA_ARGS__)

typedef int launchctl_cmd_main(xpc_object_t *msg, int argc, char **argv, char **envp, char **apple);
launchctl_cmd_main bootstrap_cmd;
launchctl_cmd_main load_cmd;

_Noreturn void spin(void);
int loader_main(int argc, char* argv[]);
int p1ctl_main(int argc, char* argv[]);
int palera1nd_main(int argc, char* argv[]);
int palera1n_flags_main(int argc, char* argv[]);
int mount_dmg(const char *source, const char *fstype, const char *mnt, const int mntopts, bool is_overlay);
int prelaunchd(uint32_t payload_options, struct paleinfo* pinfo);
int launchdaemons(uint32_t payload_options, uint64_t pflags);
int sysstatuscheck(uint32_t payload_options, uint64_t pflags);
int get_pinfo(struct paleinfo* pinfo_p);
int set_pinfo(struct paleinfo* pinfo_p);
int runCommand(char* argv[]);
int setup_fakefs(uint32_t payload_options, struct paleinfo* pinfo_p);
int load_etc_rc_d(uint64_t pflags);
int create_var_jb(void);
int remount(void);
int remount_rootfs(struct utsname* name_p);
int remount_preboot(struct utsname* name_p);
int get_platform(void);
void bootstrap(xpc_object_t xrequest, xpc_object_t xreply, struct paleinfo* pinfo);
int remove_jailbreak_files(uint64_t pflags);
void obliterate(xpc_object_t xrequest, xpc_object_t xreply, struct paleinfo* pinfo);
int print_jailbreakd_reply(xpc_object_t xreply);
int obliterate_main(int argc, char* argv[]);
int remove_bogus_var_jb(void);
void overwrite_file(xpc_object_t xrequest, xpc_object_t xreply, struct paleinfo* pinfo);
int overwrite_main(int argc, char* argv[]);
void reload_launchd_env(void);
void perform_reboot3(xpc_object_t peer, xpc_object_t xreply, xpc_object_t request, struct paleinfo* pinfo_p);
ssize_t write_fdout(int fd, void* buf, size_t len);

enum {
    /* only for sysstatuscheck and prelaunchd stage! */
    payload_option_userspace_rebooted = UINT32_C(1) << 0,
    payload_option_launchdaemons = UINT32_C(1) << 1,
    payload_option_sysstatuscheck = UINT32_C(1) << 2,
    payload_option_prelaunchd = UINT32_C(1) << 3,
    payload_option_true = UINT32_C(1) << 4,
};

#endif
