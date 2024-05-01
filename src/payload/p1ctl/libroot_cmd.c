#include <libroot.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <libjailbreak/libjailbreak.h>
#include <xpc/xpc.h>
#include <dlfcn.h>

int libroot_main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: no command specified.\n");
        return -1;
    }
    if (strcasecmp(argv[1], "jbroot") == 0) {
        printf("%s\n", libroot_dyn_get_jbroot_prefix());
        return 0;
    } else if (strcasecmp(argv[1], "root") == 0) {
        printf("%s\n", libroot_dyn_get_root_prefix());
        return 0;
    } else if (strcasecmp(argv[1], "jbrand") == 0) {
        printf("%s\n", libroot_dyn_get_boot_uuid());
        return 0;
    }

    fprintf(stderr, "Error: unknown command %s\n", argv[1]);

    return -1;
}
