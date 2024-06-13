#include <libjailbreak/libjailbreak.h>
#include <xpc/xpc.h>

void headboardInit(void) {
    xpc_object_t xreply = jailbreak_send_jailbreakd_command_with_reply_sync(JBD_CMD_RESUME_PAYLOAD);
    xpc_release(xreply);
    return;
}
