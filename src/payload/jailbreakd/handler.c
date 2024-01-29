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
#include <Security/Security.h>
#include <Security/SecTask.h>
#include <errno.h>

#define ENOENTITLEMENT 144
#define ENOTPLATFORM 154
#define RB2_USERREBOOT (0x2000000000000000llu)

#ifdef HAVE_DEBUG_JBD_MSG
#define STR_FORMAT "%{public}s"
#else
#define STR_FORMAT "%s"
#endif

//typedef struct CF_BRIDGED_TYPE(id) __SecTask *SecTaskRef;
//SecTaskRef SecTaskCreateWithAuditToken(CFAllocatorRef allocator, audit_token_t token);

void palera1nd_handler(xpc_object_t peer, xpc_object_t request, struct paleinfo* pinfo_p) {
    xpc_object_t xreply = xpc_dictionary_create_reply(request);
    xpc_object_t xremote = xpc_dictionary_get_remote_connection(request);
#ifdef HAVE_DEBUG_JBD_MSG
    char* xrequeststr = xpc_copy_description(request);
    pid_t pid = xpc_connection_get_pid(peer);
    audit_token_t token = {};
    xpc_connection_get_audit_token(peer, &token);
    SecTaskRef task = SecTaskCreateWithAuditToken(kCFAllocatorDefault, token);
    if (task) {
        CFStringRef signingIdentifier = SecTaskCopySigningIdentifier(task, NULL);
        PALERA1ND_LOG("received dictionary from client %{public}@(%d): " STR_FORMAT, signingIdentifier ? signingIdentifier : CFSTR("unknown"), pid, xrequeststr);
        if (signingIdentifier) CFRelease(signingIdentifier);
        if (task) CFRelease(task);
    }
    free(xrequeststr);
#endif

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
            if (val && xpc_get_type(val) == XPC_TYPE_BOOL) {
	            entitled = xpc_bool_get_value(val);
	        }
            if (val) xpc_release(val);
            if (!entitled) {
                xpc_dictionary_set_int64(xreply, "error", ENOENTITLEMENT);
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
            if (val && xpc_get_type(val) == XPC_TYPE_BOOL) {
	            entitled = xpc_bool_get_value(val);
	        }
            if (val) xpc_release(val);
            if (!entitled) {
                xpc_dictionary_set_int64(xreply, "error", ENOENTITLEMENT);
                xpc_dictionary_set_string(xreply, "errorDescription", "This call requires the " BOOTSTRAPPER_ENTITLEMENT ".");
                break;
            }
            bootstrap(request, xreply, pinfo_p);
            break;
        }
        case JBD_CMD_OBLITERATE_JAILBREAK: {
            bool entitled = false;
            xpc_object_t val = xpc_connection_copy_entitlement_value(peer, OBLITERATOR_ENTITLEMENT);
            if (val && xpc_get_type(val) == XPC_TYPE_BOOL) {
	            entitled = xpc_bool_get_value(val);
	        }
            if (val) xpc_release(val);
            if (!entitled) {
                xpc_dictionary_set_int64(xreply, "error", ENOENTITLEMENT);
                xpc_dictionary_set_string(xreply, "errorDescription", "This call requires the " OBLITERATOR_ENTITLEMENT ".");
                break;
            }
            obliterate(request, xreply, pinfo_p);
            break;
        }
        case JBD_CMD_PERFORM_REBOOT3: {
            perform_reboot3(peer, xreply, request, pinfo_p);
            break;
        }
        case JBD_CMD_OVERWRITE_FILE_WITH_CONTENT: {
            bool entitled = false;
            xpc_object_t val = xpc_connection_copy_entitlement_value(peer, BOOTSTRAPPER_ENTITLEMENT);
            if (val && xpc_get_type(val) == XPC_TYPE_BOOL) {
	            entitled = xpc_bool_get_value(val);
	        }
            if (val) xpc_release(val);
            if (!entitled) {
                xpc_dictionary_set_int64(xreply, "error", ENOENTITLEMENT);
                xpc_dictionary_set_string(xreply, "errorDescription", "This call requires the " BOOTSTRAPPER_ENTITLEMENT ".");
                break;
            }
            overwrite_file(request, xreply, pinfo_p);
            break;
        }
        default:
            xpc_dictionary_set_int64(xreply, "error", EINVAL);
            break;
    }

#ifdef HAVE_DEBUG_JBD_MSG
    char* xreplystr = xpc_copy_description(xreply);
    PALERA1ND_LOG("sending reply: " STR_FORMAT, xreplystr);
    free(xreplystr);
#endif
    xpc_connection_send_message(xremote, xreply);
    xpc_release(xreply);
}
