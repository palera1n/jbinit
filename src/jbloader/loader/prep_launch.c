#include <jbloader.h>
#include <xpc/xpc.h>
#include "common.h"

extern char **environ;
char *launchctl_apple[] = {NULL};


int load_daemons_from_path(char *path) {
    int ret = -1;
    xpc_object_t msg;

    CHECK(0 == access(path, F_OK));
    printf("loading %s\n", path);
    ret = bootstrap_cmd(&msg, 3, (char *[]) {"bootstrap", "system", path, NULL}, environ,
                        launchctl_apple);

end:
    printf("bootstrap_cmd returned %d\n", ret);
    return ret;
}

void load_daemons() {
    if (checkrain_options_enabled(pinfo.flags, palerain_option_rootful)) {
        load_daemons_from_path("/Library/LaunchDaemons");
    } else {
        load_daemons_from_path("/var/jb/Library/LaunchDaemons");
    }
    load_daemons_from_path("/cores/binpack/Library/LaunchDaemons");
}

void *prep_jb_launch(void *__unused _) {
    assert(info.size == sizeof(struct kerninfo));
    if (checkrain_options_enabled(info.flags, checkrain_option_force_revert)) {
        jailbreak_obliterator();
        return NULL;
    }
    if (checkrain_options_enabled(pinfo.flags, palerain_option_clean_fakefs) &&
        !checkrain_options_enabled(jbloader_flags, jbloader_userspace_rebooted)) {
        run("/cores/binpack/bin/rm", (char *[]) {
                "/cores/binpack/bin/rm",
                "-rf",
                // "/var/jb",
                "/var/lib",
                "/var/cache",
                NULL});
        /*char num_buf[20];
        pinfo.flags &= ~palerain_option_clean_fakefs;
        snprintf(num_buf, 20, "%d", pinfo.flags);
        int ret = palera1n_flags_main(2, (char*[]){ "palera1n_flags", num_buf, NULL });
        printf("palera1n_flags_main returned %d\n", ret);*/
        return NULL;
    }
    if (checkrain_options_enabled(info.flags, checkrain_option_safemode)) {
        printf("Safe mode is enabled\n");
    } else {
        load_daemons();
    }
    return NULL;
}

