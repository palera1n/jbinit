#include <payload/payload.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <libjailbreak/libjailbreak.h>
#include <unistd.h>

int obliterate_main(int argc, char* argv[]) {
    P1CTL_UPCALL_JBD_WITH_ERR_CHECK(xreply, JBD_CMD_OBLITERATE_JAILBREAK);

    int retval = print_jailbreakd_reply(xreply);
    xpc_release(xreply);
    return retval;
}
