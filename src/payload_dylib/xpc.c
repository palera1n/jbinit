#include <payload_dylib/common.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/kern_memorystatus.h>
#include <xpc/private.h>
#include <sys/sysctl.h>
#include <sys/reboot.h>
#include <libjailbreak/libjailbreak.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <mach-o/loader.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#define ENOTDEVELOPMENT 142

static uuid_t boot_uuid;

static void xpc_handler(xpc_object_t xdict) {
#ifdef DEV_BUILD
    char* description = xpc_copy_description(xdict);
    if (description) fprintf(stderr, "received jailbreak related dictionary: %s\n", description);
    free(description);
#endif

    audit_token_t token = {};
    xpc_dictionary_get_audit_token(xdict, &token);
    xpc_object_t isJailbreakD = xpc_copy_entitlement_for_token(LAUNCHD_CMD_ENTITLEMENT, &token);
    xpc_object_t xreply = xpc_dictionary_create_reply(xdict);

    uint64_t cmd = xpc_dictionary_get_uint64(xdict, "cmd");
    if (cmd != LAUNCHD_CMD_GET_BOOT_UUID) {
        if (xpc_get_type(isJailbreakD) != XPC_TYPE_BOOL || !xpc_bool_get_value(isJailbreakD)) {
            xpc_dictionary_set_int64(xreply, "error", EPERM);
            goto reply;
        }
    }
    
    switch (cmd) {
        case LAUNCHD_CMD_RELOAD_JB_ENV: {
            if ((pflags & palerain_option_rootful) == 0) {
                load_bootstrapped_jailbreak_env();
            } else {
                xpc_dictionary_set_string(xdict, "errorDescription", "Operation only supported on rootless");
                xpc_dictionary_set_int64(xreply, "error", ENOTSUP);
            }
            break;
        }
        case LAUNCHD_CMD_SET_TWEAKLOADER_PATH: {
#ifdef DEV_BUILD
            const char* path = xpc_dictionary_get_string(xdict, "path");
            if (!path) {
                xpc_dictionary_set_string(xdict, "errorDescription", "no tweakloader path supplied");
                xpc_dictionary_set_int64(xreply, "error", EINVAL);
                break;
            }
            const char* desc;
            if ((desc = set_tweakloader_path(path))) {
                xpc_dictionary_set_string(xdict, "errorDescription", desc);
            }
#else
            xpc_dictionary_set_int64(xreply, "error", ENOTDEVELOPMENT);
#endif
            break;
        }
        case LAUNCHD_CMD_SET_PINFO_FLAGS: {
            xpc_object_t xpc_flags = xpc_dictionary_get_value(xdict, "flags");
            if (xpc_get_type(xpc_flags) != XPC_TYPE_UINT64) {
                xpc_dictionary_set_int64(xreply, "error", EINVAL);
                break;
            }
            uint64_t flags = xpc_uint64_get_value(xpc_flags);
            pflags = flags;
            void *systemhook_handle = dlopen(HOOK_DYLIB_PATH, RTLD_NOW);
            if (systemhook_handle) {
                char** pJB_PinfoFlags = dlsym(systemhook_handle, "JB_PinfoFlags");
                uint64_t* ppflags = dlsym(systemhook_handle, "pflags");
                if (ppflags) {
                    *ppflags = flags;
                }
                if (pJB_PinfoFlags) {
                    char* new_JB_PinfoFlags = malloc(30);
                    if (new_JB_PinfoFlags) {
                        char* old_JB_PinfoFlags = *pJB_PinfoFlags;
                        snprintf(new_JB_PinfoFlags, 30, "0x%" PRIx64, pflags);
                        *pJB_PinfoFlags = new_JB_PinfoFlags;
                        free(old_JB_PinfoFlags);
                    }
                }
                dlclose(systemhook_handle);
            }
            break;
        }
#if 0
        case LAUNCHD_CMD_DRAW_IMAGE: {
            const char* path = xpc_dictionary_get_string(xdict, "path");
            if (!path) {
                xpc_dictionary_set_int64(xreply, "error", EINVAL);
                break;
            }
            bootscreend_draw_image(path);
            break;
        }
#endif
        case LAUNCHD_CMD_CRASH:
#ifdef DEV_BUILD
            ((void(*)(void))0x4141414141414141)();
#else
            xpc_dictionary_set_int64(xreply, "error", ENOTDEVELOPMENT);
#endif
            break;
        case LAUNCHD_CMD_RUN_BOOTSCREEND:
            break;
        case LAUNCHD_CMD_GET_BOOT_UUID:
            xpc_dictionary_set_uuid(xreply, "uuid", boot_uuid);
            break;
        default: {
            xpc_dictionary_set_int64(xreply, "error", EINVAL);
            break;
        }
    }
reply:
#ifdef DEV_BUILD
    description = xpc_copy_description(xreply);
    if (description) fprintf(stderr, "replying to jailbreak related dictionary: %s\n", description);
    free(description);
#endif
    xpc_pipe_routine_reply(xreply);
    if (isJailbreakD) xpc_release(isJailbreakD);
    xpc_release(xreply);
    return;

}

int (*xpc_receive_mach_msg_orig)(void *a1, void *a2, void *a3, void *a4, xpc_object_t *xdictp);
int xpc_receive_mach_msg_hook(void *a1, void *a2, void *a3, void *a4, xpc_object_t *xdictp)
{
    int r = xpc_receive_mach_msg_orig(a1, a2, a3, a4, xdictp);
    if (r == 0) {
        if (!xdictp || xpc_get_type(*xdictp) != XPC_TYPE_DICTIONARY || !xpc_dictionary_get_bool(*xdictp, "jailbreak")) {
            return r;
        } else {
            xpc_handler(*xdictp);
            xpc_release(*xdictp);
            return 22;
        }
    }
    return r;
}


void InitXPCHooks(void) {
    uuid_generate(boot_uuid);
    MSHookFunction_p(xpc_receive_mach_msg, (void*)&xpc_receive_mach_msg_hook, (void**)&xpc_receive_mach_msg_orig);
}
