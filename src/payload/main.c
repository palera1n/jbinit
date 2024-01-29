#include <stdio.h>
#include <payload/payload.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>

os_log_t palera1nd_log;

_Noreturn void spin(void) {
    setenv("PATH", "/cores/binpack/usr/bin:/cores/binpack/usr/sbin:/cores/binpack/bin:/cores/binpack/sbin:/usr/bin:/usr/sbin:/bin:/sbin",1);
    if (access("/cores/binpack/bin/sh", F_OK) == 0) {
        runCommand((char*[]){ "/cores/binpack/bin/sh", "-i", NULL });
    }
    while(1) sleep(86400);
}

int main(int argc, char* argv[]) {
    char* name = basename(argv[0]);
    if (!strcmp(name, "payload")) {
        palera1nd_log = os_log_create("com.apple.payload", "plooshInit payload");
        return loader_main(argc, argv);
    } else if (!strcmp(name, "p1ctl")) {
        return p1ctl_main(argc, argv);
    } else if (!strcmp(name, "palera1nd")) {
        palera1nd_log = os_log_create("com.apple.payload", "palera1n Daemon");
        return palera1nd_main(argc, argv);
    } else {
        return -1;
    }
}
