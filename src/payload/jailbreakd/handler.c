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
#include <sys/codesign.h>
#include <sys/mount.h>
#include <errno.h>

#define ENOTPLATFORM 154
#define RB2_USERREBOOT (0x2000000000000000llu)
int reboot3(uint64_t how, uint64_t unk);

void palera1nd_handler(xpc_object_t peer, xpc_object_t request, struct paleinfo* pinfo_p) {
    xpc_object_t xreply = xpc_dictionary_create_reply(request);
    xpc_object_t xremote = xpc_dictionary_get_remote_connection(request);
    if (!xremote || !xreply) return;

    uint64_t cmd = xpc_dictionary_get_uint64(request, "cmd");
    switch (cmd) {
        case JBD_CMD_GET_PINFO_FLAGS:
            xpc_dictionary_set_uint64(xreply, "flags", pinfo_p->flags);
            break;
        
        case JBD_CMD_GET_PREBOOTPATH: {
            if (pinfo_p->flags & palerain_option_rootless) {
                char prebootPath[150];
                int ret = jailbreak_get_prebootPath(prebootPath);
                if (ret == 0) {
                    xpc_dictionary_set_string(xreply, "path", prebootPath);
                } else {
                    xpc_dictionary_set_int64(xreply, "error", errno);
                }
            } else {
                xpc_dictionary_set_int64(xreply, "error", ENOTSUP);
            }
            break;
        }
        case JBD_CMD_GET_PINFO_KERNEL_INFO: {
            bool entitled = false;
            xpc_object_t val = xpc_connection_copy_entitlement_value(peer, KERNELINFO_ENTITLEMENT);
            if (xpc_get_type(val) == XPC_TYPE_BOOL) {
	            entitled = xpc_bool_get_value(val);
	        }
            if (!entitled) {
                xpc_dictionary_set_int64(xreply, "error", EPERM);
                break;
            }
            xpc_dictionary_set_uint64(xreply, "kbase", pinfo_p->kbase);
            xpc_dictionary_set_uint64(xreply, "kslide", pinfo_p->kslide);
            break;
        }
        case JBD_CMD_GET_PINFO_ROOTDEV: {
            xpc_dictionary_set_string(xreply, "rootdev", pinfo_p->rootdev);
            break;
        }
        case JBD_CMD_DEPLOY_BOOTSTRAP: {
            bool entitled = false;
            xpc_object_t val = xpc_connection_copy_entitlement_value(peer, BOOTSTRAPPER_ENTITLEMENT);
            if (xpc_get_type(val) == XPC_TYPE_BOOL) {
	            entitled = xpc_bool_get_value(val);
	        }
            if (!entitled) {
                xpc_dictionary_set_int64(xreply, "error", EPERM);
                xpc_dictionary_set_string(xreply, "errorDescription", "This call requires the " BOOTSTRAPPER_ENTITLEMENT ".");
                break;
            }
            bootstrap(request, xreply, pinfo_p);
            break;
        }
        case JBD_CMD_OBLITERATE_JAILBREAK: {
            bool entitled = false;
            xpc_object_t val = xpc_connection_copy_entitlement_value(peer, OBLITERATOR_ENTITLEMENT);
            if (xpc_get_type(val) == XPC_TYPE_BOOL) {
	            entitled = xpc_bool_get_value(val);
	        }
            if (!entitled) {
                xpc_dictionary_set_int64(xreply, "error", EPERM);
                xpc_dictionary_set_string(xreply, "errorDescription", "This call requires the " OBLITERATOR_ENTITLEMENT ".");
                break;
            }
            obliterate(request, xreply, pinfo_p);
            break;
        }
        case JBD_CMD_REBOOT_USERSPACE: {
            audit_token_t token;
            xpc_connection_get_audit_token(peer, &token);
            pid_t pid = xpc_connection_get_pid(peer);
            if (pid == -1) {
                xpc_dictionary_set_int64(xreply, "error", errno);
                break;
            }
            int status;
            int ret = csops_audittoken(pid, CS_OPS_STATUS, &status, 4, &token);
            if (ret) {
                xpc_dictionary_set_int64(xreply, "error", errno);
                break;
            }
            if ((status & CS_PLATFORM_BINARY) == 0) {
                xpc_dictionary_set_int64(xreply, "error", ENOTPLATFORM);
                break;
            }
            unmount("/Developer", MNT_FORCE);
            ret = reboot3(RB2_USERREBOOT, 0);
            if (ret) {
                xpc_dictionary_set_int64(xreply, "error", ret);
                break;
            }
            break;
        }
        default:
            xpc_dictionary_set_int64(xreply, "error", EINVAL);
            break;
    }
    
    char* desc = xpc_copy_description(xreply);
    // NSLog(CFSTR("sending reply: %s"), desc);
    free(desc);
    xpc_connection_send_message(xremote, xreply);
    xpc_release(xreply);
}
