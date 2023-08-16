#include <stdio.h>
#include <payload/payload.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>

_Noreturn void spin() {
    setenv("PATH", "/cores/binpack/usr/bin:/cores/binpack/usr/sbin:/cores/binpack/bin:/cores/binpack/sbin:/usr/bin:/usr/sbin:/bin:/sbin",1);
    if (access("/cores/binpack/bin/sh", F_OK) == 0) {
        runCommand((char*[]){ "/cores/binpack/bin/sh", "-i", NULL });
    }
    while(1) sleep(86400);
}

int main(int argc, char* argv[]) {
    char* name = basename(argv[0]);
    if (!strcmp(name, "payload")) {
        return loader_main(argc, argv);
    } else if (!strcmp(name, "p1ctl")) {
        return p1ctl_main(argc, argv);
    } else if (!strcmp(name, "palera1nd")) {
        return palera1nd_main(argc, argv);
    } else {
        return -1;
    }
}
