#include <payload/payload.h>
#include <removefile.h>
#include <sys/mount.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/utsname.h>
#include <CoreFoundation/CoreFoundation.h>
#include <libjailbreak/libjailbreak.h>
#include <sys/stat.h>

int create_var_jb() {
    if (access("/var/jb", F_OK) == 0) return 0;
    char prebootPath[150];
    int ret = jailbreak_get_prebootPath(prebootPath);
    switch (ret) {
        case ENOTSUP:
        case ENOENT:
            return 0;
        case EEXIST:
            fprintf(stderr, "duplicate preboot jailbreak path\n");
            return -1;
        case ENOTDIR:
            fprintf(stderr, "jailbreak path is not a directory\n");
            return -1;
        case KERN_SUCCESS:
            printf("/var/jb -> %s\n", prebootPath);
            break;
        default:
            fprintf(stderr, "cannot create /var/jb\n");
            return -1;
    }
    return symlink(prebootPath, "/var/jb");
}
