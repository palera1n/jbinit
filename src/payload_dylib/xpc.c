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

static void (*xpc_handler_orig)(uint64_t a1, uint64_t a2, xpc_object_t xdict);
static void xpc_handler_hook(uint64_t a1, uint64_t a2, xpc_object_t xdict) {
    if (!xdict || xpc_get_type(xdict) != XPC_TYPE_DICTIONARY || !xpc_dictionary_get_bool(xdict, "jailbreak")) {
        return xpc_handler_orig(a1, a2, xdict);
    }
    
#ifdef DEV_BUILD
    char* description = xpc_copy_description(xdict);
    if (description) fprintf(stderr, "received jailbreak-related dictionary: %s\n", description);
    free(description);
#endif

    audit_token_t token = {};
    xpc_dictionary_get_audit_token(xdict, &token);
    xpc_object_t isJailbreakD = xpc_copy_entitlement_for_token(LAUNCHD_CMD_ENTITLEMENT, &token);
    xpc_object_t xreply = xpc_dictionary_create_reply(xdict);
    if (xpc_get_type(isJailbreakD) != XPC_TYPE_BOOL || !xpc_bool_get_value(isJailbreakD)) {
        xpc_dictionary_set_int64(xreply, "error", EPERM);
        goto reply;
    }

    uint64_t cmd = xpc_dictionary_get_uint64(xdict, "cmd");
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

static uint32_t* find_insn_maskmatch_match(uint8_t* data, size_t size, uint32_t* matches, uint32_t* masks, int count) {
    int found = 0;
    if(sizeof(matches) != sizeof(masks))
        return NULL;
    
    uint32_t* retval = NULL;
    uint32_t* current_inst = (uint32_t*)data;
    while ((uintptr_t)current_inst < (uintptr_t)data + size - 4 - (count*4)) {
        current_inst++;
        found = 1;
        for(int i = 0; i < count; i++) {
            if((matches[i] & masks[i]) != (current_inst[i] & masks[i])) {
                found = 0;
                break;
            }
        }
        if (found) {
            if (!retval)
                retval = current_inst;
            else {
                fprintf(stderr, "found twice!");
                return NULL;
            }
        }
    }
    
    return retval;
}

static uint32_t* find_prev_insn(uint32_t* from, uint32_t num, uint32_t insn, uint32_t mask) {
    while(num) {
        if((*from & mask) == (insn & mask)) {
            return from;
        }
        from--;
        num--;
    }
    return NULL;
}

void InitXPCHooks(void) {
    uint32_t bufsize = PATH_MAX;
    char launchd_path[PATH_MAX];
    int launchd_image_index = 0;
    int ret = _NSGetExecutablePath(launchd_path, &bufsize);
    if (ret) {
        fprintf(stderr, "_NSGetExecutablePath() failed\n");
        spin();
    }
	for (int i = 0; i < _dyld_image_count(); i++) {
		if(!strcmp(_dyld_get_image_name(i), launchd_path)) {
			launchd_image_index = i;
			break;
		}
	}
    intptr_t slide = _dyld_get_image_vmaddr_slide(launchd_image_index);
    struct mach_header_64 *mach_header = (struct mach_header_64*)_dyld_get_image_header(launchd_image_index);

	uintptr_t cmd_current = (uintptr_t)(mach_header + 1);
	uintptr_t cmd_end = cmd_current + mach_header->sizeofcmds;

    uint32_t* text_start = NULL;
    size_t text_size = 0;
    for (int i = 0; i < mach_header->ncmds && cmd_current <= cmd_end; i++) {
        const struct segment_command_64 *cmd;

        cmd = (struct segment_command_64*)cmd_current;
        cmd_current += cmd->cmdsize;

		if (cmd->cmd != LC_SEGMENT_64 || strcmp(cmd->segname, "__TEXT")) {
			continue;
		}

        text_start = (uint32_t*)(cmd->vmaddr + slide);
        text_size = (size_t)cmd->vmsize;
    }

    if (!text_size) {
        fprintf(stderr, "failed to find launchd __TEXT segment");
        spin();
    }

    uint32_t matches[] = {
        0x12000028, // mov w8, #0x1 (orr w8, wzr, #1 / movz w8, #0x1)
        0x39000200, // strb w8, [x{16-31}, did_enter_server_layer@PAGEOFF]
        0x90000010, // adrp x{16-31}, did_enter_server_layer@PAGE
        0x3900021f, // strb wzr, [x{16-31}, reply_include_req_info@PAGEOFF]
        0x10000001  // adr x1, "mig-request"
    };

    uint32_t masks[] = {
        0x121f7c3f,
        0xffc00200,
        0x9f000010,
        0xffc0021f,
        0x9f00001f
    };

    uint32_t* xpc_handler = NULL;
    uint32_t* xpc_handler_mid = find_insn_maskmatch_match((uint8_t*)text_start, text_size, matches, masks, sizeof(matches)/sizeof(uint32_t));
    printf("xpc_handler_mid=%p\n", xpc_handler_mid);

    if (xpc_handler_mid) {
        xpc_handler = find_prev_insn(xpc_handler_mid, 25, 0xd10003ff, 0xffc003ff); // sub sp, sp, *
        if (xpc_handler[-1] == 0xd503237f) xpc_handler -= 1; // pacibsp

        printf("xpc_handler=%p\n", xpc_handler);
    }

    if (xpc_handler) {
        MSHookFunction_p(xpc_handler, (void*)xpc_handler_hook, (void**)&xpc_handler_orig);
    } else {
        fprintf(stderr, "patchfind launchd failed\n");
        spin();
    }
}
