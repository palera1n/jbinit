#include <payload/payload.h>
#include <paleinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/types.h>
#include <limits.h>
#include <spawn.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int loader_main(int argc, char* argv[]) {
    if (getuid() != 0) goto out;
    struct paleinfo pinfo;
    CHECK_ERROR(get_pinfo(&pinfo), 1, "read paleinfo failed");
    uint32_t payload_options = 0;
    int ch;
    while ((ch = getopt(argc, argv, "usjft")) != -1) {
        switch(ch) {
            case 'u':
                payload_options |= payload_option_userspace_rebooted;
                break;
            case 's':
                payload_options |= payload_option_sysstatuscheck;
                break;
            case 'j':
                payload_options |= payload_option_launchdaemons;
                break;
            case 'f':
                payload_options |= payload_option_prelaunchd;
                break;
            case 't':
                payload_options |= payload_option_true;
                break;
            default:
                goto out;
        }
    }
    if (payload_options & payload_option_true) return 0;
    if (getenv("XPC_USERSPACE_REBOOTED")) payload_options |= payload_option_userspace_rebooted;
    if (payload_options & payload_option_prelaunchd) return prelaunchd(payload_options, &pinfo);
    else if (payload_options & payload_option_sysstatuscheck) return sysstatuscheck(payload_options, pinfo.flags);
    else if (payload_options & payload_option_launchdaemons) return launchdaemons(payload_options, pinfo.flags);
out:
    fprintf(stderr, "this is a plinit internal utility, do not run\n");
    return -1;
}
