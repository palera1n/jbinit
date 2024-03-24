#include <stdio.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <xpc/connection.h>
#include <libjailbreak/libjailbreak.h>
#include <CoreFoundation/CoreFoundation.h>
#include <payload/payload.h>
#include <paleinfo.h>
#include <errno.h>
#include <spawn.h>
#include <pthread.h>
#define TARGET_OS_IPHONE 1
#include <spawn_private.h>
#include <sys/spawn_internal.h>
#include <sys/kern_memorystatus.h>
#include <CoreFoundation/CoreFoundation.h>
#include <removefile.h>
#include <copyfile.h>
#include <sys/reboot.h>

#include <sys/stat.h>

extern char** environ;

void revert_snapshot(void);
int reboot3(uint64_t howto, ...);

int revert_rootful(xpc_object_t xreply, struct paleinfo* pinfo) {
    int unload_cmd_ret, remove_ret;
    xpc_object_t msg;
    unload_cmd_ret = load_cmd(&msg, 2, (char*[]){ "unload", "/Library/LaunchDaemons", NULL }, environ, (char*[]){ NULL });
    remove_ret = remove_jailbreak_files(pinfo->flags);
    
    revert_snapshot();
    
    xpc_dictionary_set_int64(xreply, "unload_cmd_ret", (int64_t)unload_cmd_ret);
    xpc_dictionary_set_int64(xreply, "remove_ret", (int64_t)remove_ret);
    return remove_ret;
}

int revert_rootless(xpc_object_t xreply, struct paleinfo* pinfo) {
    int unload_cmd_ret, remove_ret;
    xpc_object_t msg;
    unload_cmd_ret = load_cmd(&msg, 2, (char*[]){ "unload", "/var/jb/Library/LaunchDaemons", NULL }, environ, (char*[]){ NULL });
    remove_ret = remove_jailbreak_files(pinfo->flags);

    xpc_dictionary_set_int64(xreply, "unload_cmd_ret", (int64_t)unload_cmd_ret);
    xpc_dictionary_set_int64(xreply, "remove_ret", (int64_t)remove_ret);
    return remove_ret;
}

void obliterate(xpc_object_t __unused xrequest, xpc_object_t xreply, struct paleinfo* pinfo) {
    if ((pinfo->flags & (palerain_option_rootful | palerain_option_ssv)) == (palerain_option_rootful | palerain_option_ssv)) {
        xpc_dictionary_set_string(xreply, "errorDescription", "oblierating jailbreak while booted is not supported on rootful fakefs");
        xpc_dictionary_set_int64(xreply, "error", ENOTSUP);
        return;
    }
    int remove_ret;
    if (pinfo->flags & palerain_option_rootful)
        remove_ret = revert_rootful(xreply, pinfo);
    else
        remove_ret = revert_rootless(xreply, pinfo);

    if (remove_ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "remove_jailbreak_files returned non-zero code");
    } else {
        if (pinfo->flags & palerain_option_rootful) {
            xpc_dictionary_set_string(xreply, "message", "Success. Now rebooting.");
        } else {
            xpc_dictionary_set_string(xreply, "message", "Success");
        }
    }

    reload_launchd_env();
    
    if (pinfo->flags & palerain_option_rootful) reboot3(RB_AUTOBOOT);
    return;
}
