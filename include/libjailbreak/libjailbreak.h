#ifndef LIBJAILBREAK_LIBJAILBREAK_H
#define LIBJAILBREAK_LIBJAILBREAK_H

#include <xpc/xpc.h>

#define KERNELINFO_ENTITLEMENT "in.palera.pinfo.kernel-info"
#define BOOTSTRAPPER_ENTITLEMENT "in.palera.loader.bootstrapper"
#define OBLITERATOR_ENTITLEMENT "in.palera.loader.allow-obliterate-jailbreak"

#define HOOK_DYLIB_PATH "/cores/binpack/usr/lib/systemhook.dylib"
enum {
    JBD_CMD_GET_PINFO_FLAGS = 1,
    JBD_CMD_GET_PREBOOTPATH,
    JBD_CMD_GET_PINFO_KERNEL_INFO,
    JBD_CMD_GET_PINFO_ROOTDEV,
    JBD_CMD_DEPLOY_BOOTSTRAP,
    JBD_CMD_OBLITERATE_JAILBREAK,
    JBD_CMD_REBOOT_USERSPACE
};

int jailbreak_get_bmhash(char* hash);
int jailbreak_get_prebootPath(char jbPath[150]);
const char* jailbreak_str_pinfo_flag(uint64_t flag);
xpc_object_t jailbreak_send_jailbreakd_message_with_reply_sync(xpc_object_t xdict);
xpc_object_t jailbreak_send_jailbreakd_command_with_reply_sync(uint64_t cmd);

#endif
