#include <payload_dylib/common.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/kern_memorystatus.h>
#include <xpc/private.h>
#include <sys/sysctl.h>
#include <sys/reboot.h>
#include <libjailbreak/libjailbreak.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <pthread.h>
#include <errno.h>

void (*xpc_handler_orig)(uint64_t a1, uint64_t a2, xpc_object_t xdict);
void xpc_handler_hook(uint64_t a1, uint64_t a2, xpc_object_t xdict) {
    if (!xdict || xpc_get_type(xdict) != XPC_TYPE_DICTIONARY || !xpc_dictionary_get_bool(xdict, "jailbreak")) {
        return xpc_handler_orig(a1, a2, xdict);
    }

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
                break;
            }
        }
        case LAUNCHD_CMD_SET_TWEAKLOADER_PATH: {
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
            break;
        }
        
        default: {
            xpc_dictionary_set_int64(xreply, "error", EINVAL);
            return;
        }
    }
reply:
    xpc_pipe_routine_reply(xreply);
    return;

}

uint32_t* find_insn_maskmatch_match(uint8_t* data, size_t size, uint32_t* matches, uint32_t* masks, int count) {
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

uint32_t* find_prev_insn(uint32_t* from, uint32_t num, uint32_t insn, uint32_t mask) {
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
        0x00000008, // mov w8, #0x1 (orr w8, wzr, #1 / movz w8, #0x1)
        0x39000000, // strb *
        0x90000000, // adrp *
        0x39000000, // strb *
        0x10000001  // adr x1, *
    };

    uint32_t masks[] = {
        0x001f7c1f,
        0xffc00000,
        0x9f000000,
        0xffc00000,
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
