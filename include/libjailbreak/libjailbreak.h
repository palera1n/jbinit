#ifndef LIBJAILBREAK_LIBJAILBREAK_H
#define LIBJAILBREAK_LIBJAILBREAK_H

#include <xpc/xpc.h>
#include <os/alloc_once_private.h>

#define KERNELINFO_ENTITLEMENT "in.palera.pinfo.kernel-info"
#define BOOTSTRAPPER_ENTITLEMENT "in.palera.loader.bootstrapper"
#define OBLITERATOR_ENTITLEMENT "in.palera.loader.allow-obliterate-jailbreak"
#define LAUNCHD_CMD_ENTITLEMENT "in.palera.private.launchd-commands.client"


#define HOOK_DYLIB_PATH "/cores/binpack/usr/lib/systemhook.dylib"
enum {
    JBD_CMD_GET_PINFO_FLAGS = 1,
    JBD_CMD_GET_PREBOOTPATH,
    JBD_CMD_GET_PINFO_KERNEL_INFO,
    JBD_CMD_GET_PINFO_ROOTDEV,
    JBD_CMD_DEPLOY_BOOTSTRAP,
    JBD_CMD_OBLITERATE_JAILBREAK,
    JBD_CMD_PERFORM_REBOOT3,
    JBD_CMD_OVERWRITE_FILE_WITH_CONTENT,
    JBD_CMD_INTERCEPT_USERSPACE_PANIC,
    JBD_CMD_EXIT_SAFE_MODE,
    JBD_CMD_RUN_AS_ROOT,
    JBD_CMD_REGISTER_PAYLOAD_PID,
    JBD_CMD_RESUME_PAYLOAD
};

enum {
    LAUNCHD_CMD_RELOAD_JB_ENV = 1,
    LAUNCHD_CMD_SET_TWEAKLOADER_PATH,
    LAUNCHD_CMD_SET_PINFO_FLAGS,
    LAUNCHD_CMD_DRAW_IMAGE,
    LAUNCHD_CMD_CRASH,
    LAUNCHD_CMD_RUN_BOOTSCREEND,
    LAUNCHD_CMD_GET_BOOT_UUID
};

struct xpc_global_data {
	uint64_t a;
	uint64_t xpc_flags;
	mach_port_t task_bootstrap_port; /* 0x10 */
#ifndef _64
	uint32_t padding;
#endif
	xpc_object_t xpc_bootstrap_pipe; /* 0x18 */
};

int jailbreak_get_bmhash(char* hash);
int jailbreak_get_prebootPath(char jbPath[150]);
const char* jailbreak_str_pinfo_flag(uint64_t flag);
xpc_object_t jailbreak_send_jailbreakd_message_with_reply_sync(xpc_object_t xdict);
xpc_object_t jailbreak_send_jailbreakd_command_with_reply_sync(uint64_t cmd);
int jailbreak_send_launchd_message(xpc_object_t xdict, xpc_object_t *xreply);

#endif
