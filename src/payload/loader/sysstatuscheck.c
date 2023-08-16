#include <payload/payload.h>
#include <libjailbreak/libjailbreak.h>
#include <removefile.h>
#include <sys/mount.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <mount_args.h>

int remove_jailbreak_files(uint64_t pflags) {
    removefile_state_t state = removefile_state_alloc();
    if (pflags & palerain_option_rootful) {
        printf("delete /var/lib\n");
        removefile("/var/lib", state, REMOVEFILE_RECURSIVE);
        printf("delete /var/cache\n");
        removefile("/var/cache", state, REMOVEFILE_RECURSIVE);
    } else {
        char bmhash[97], prebootPath[150], jbPaths[150];
        int ret = jailbreak_get_bmhash(bmhash);
        if (ret) {
            fprintf(stderr, "get bmhash failed\n");
            return -1;
        }
        snprintf(prebootPath, 150, "/private/preboot/%s", bmhash);
        DIR* dir = opendir(prebootPath);
        if (!dir) return 0;
        struct dirent* d;
        while ((d = readdir(dir)) != NULL) {
            if (strncmp(d->d_name, "jb-", 3)) continue;
            snprintf(jbPaths, 150, "%s/%s", prebootPath, d->d_name);
            if ((strcmp(jbPaths, prebootPath) == 0)) {
                fprintf(stderr, "disaster averted\n");
                return -1;
            }
            printf("delete %s\n", jbPaths);
            removefile(jbPaths, state, REMOVEFILE_RECURSIVE);
        }
        printf("delete /var/jb\n");
        removefile("/var/jb", state, REMOVEFILE_RECURSIVE);
    }
    removefile_state_free(state);
    return 0;
}

int fixup_databases(void);
int sysstatuscheck(uint32_t payload_options, uint64_t pflags) {
    printf("plooshInit sysstatuscheck...\n");
    remount(0);
    if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0) {
        printf("generating ssh host key...\n");
        runCommand((char*[]){ "/cores/binpack/usr/bin/dropbearkey", "-f",  "/private/var/dropbear_rsa_host_key", "-t", "rsa", "-s", "4096", });
    }
    if ((pflags & palerain_option_force_revert)) remove_jailbreak_files(pflags);
    if (pflags & palerain_option_rootful) {
        unlink("/var/jb");
    } else {
        create_var_jb();
#ifdef SYSTEMWIDE_IOSEXEC
        if (access("/var/jb", F_OK) == 0) {
            fixup_databases();
        }
#endif
    }
    if (
        (pflags & palerain_option_safemode) == 0 &&
        (pflags & palerain_option_force_revert) == 0
        ) {
            load_etc_rc_d(pflags);
    }

    return execv("/usr/libexec/sysstatuscheck", (char*[]){ "/usr/libexec/sysstatuscheck", NULL });
}
