#include <payload/payload.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <mach-o/loader.h>

int launchdaemons(uint32_t payload_options, uint64_t pflags) {
    printf("plooshInit launchdaemons...\n");
    int platform = get_platform();
    if (platform == -1) {
        fprintf(stderr, "get_platform(): %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    switch (platform) {
        case PLATFORM_IOS:
            runCommand((char*[]){ "/cores/binpack/usr/bin/uicache", "-p", "/cores/binpack/Applications/palera1nLoader.app", NULL });
            break;
        case PLATFORM_TVOS:
        case PLATFORM_BRIDGEOS:
            break;
        default:
            fprintf(stderr, "unsupported platform\n");
    }

    if (access("/usr/bin/uicache", F_OK) == 0)
        runCommand((char*[]){ "/usr/bin/uicache", "-a", NULL });
    else if (access("/var/jb/usr/bin/uicache", F_OK) == 0)
        runCommand((char*[]){ "/var/jb/usr/bin/uicache", "-a", NULL });
    return 0;
}
