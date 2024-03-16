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
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/codesign.h>
#include <copyfile.h>

#include <sys/stat.h>

#define ENOENTITLEMENT 144
#define ENOTPLATFORM 154
#define RB2_USERREBOOT (0x2000000000000000llu)
#define ITHINK_PURPOSE (0x0100000000000000llu)
int reboot3(uint64_t howto, ...);

void perform_reboot3(xpc_object_t peer, xpc_object_t xreply, xpc_object_t request, struct paleinfo* __unused pinfo_p) {
    audit_token_t token;
    xpc_connection_get_audit_token(peer, &token);
    pid_t pid = xpc_connection_get_pid(peer);
    if (pid == -1) {
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }
    int status;
    int ret = csops_audittoken(pid, CS_OPS_STATUS, &status, 4, &token);
    if (ret) {
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }
    if ((status & CS_PLATFORM_BINARY) == 0) {
        xpc_dictionary_set_int64(xreply, "error", ENOTPLATFORM);
        return;
    }
    xpc_object_t xpc_howto = xpc_dictionary_get_value(request, "howto");
    xpc_object_t xpc_purpose = xpc_dictionary_get_value(request, "purpose");
    if (!xpc_howto) {
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        xpc_dictionary_set_string(xreply, "errorDescription", "howto not supplied");
        return;
    }
    uint64_t howto = xpc_uint64_get_value(xpc_howto);
    if (howto & RB2_USERREBOOT) {
        bool entitled = false;
        xpc_object_t value = xpc_connection_copy_entitlement_value(peer, "com.apple.private.xpc.launchd.userspace-reboot");
        if (value && xpc_get_type(value) == XPC_TYPE_BOOL) {
            entitled = true;
        }
        if (value) xpc_release(value);
        if (!entitled) {
            xpc_dictionary_set_int64(xreply, "error", ENOENTITLEMENT);
            return;
        }
    }
    if ((howto & ITHINK_PURPOSE) && !xpc_purpose) {
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        xpc_dictionary_set_string(xreply, "errorDescription", "reboot flags included the purpose flag, but no purpose supplied in request");
        return;
    }

    unmount("/Developer", MNT_FORCE);
    if (howto & ITHINK_PURPOSE) {
        ret = reboot3(howto, (uint32_t)xpc_uint64_get_value(xpc_purpose));
    } else ret = reboot3(howto);

    if (ret) {
        xpc_dictionary_set_int64(xreply, "error", ret);
        return;
    }
}
