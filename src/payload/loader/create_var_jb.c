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

/*
 * in some situations, there could be a bogus /var/jb directory
 * either by some old version of unc0ver or the cfprefsd hook
 */
int remove_bogus_var_jb(void) {
    int ret;
    struct stat st;
    if (lstat("/var/jb", &st) != 0) {
        if (errno != ENOENT) {
            NSLog(CFSTR("failed to lstat /var/jb: %d (%s)"), errno, strerror(errno));
        }
        return 0;
    }
    if (S_ISLNK(st.st_mode)) {
        char link_path[PATH_MAX];
        ret = (int)readlink("/var/jb", link_path, PATH_MAX);
        if (ret == -1) return -1;
        // unc0ver
        if (!strcmp(link_path, "/jb") || !strcmp(link_path, "/jb/")) {
            NSLog(CFSTR("Found unc0ver! /var/jb -> %s, deleting"), link_path);
            /*if (access("/jb", F_OK) == 0) {
                removefile("/jb", NULL, 0);
            }*/
            ret = unlink("/var/jb");
            return ret;
        // unc0ver read only rootfs ssh only
        } else if (!strcmp(link_path, "/private/var/containers/Bundle/jb") || !strcmp(link_path, "/private/var/containers/Bundle/jb/")) {
            /*
             * now this is a bit more complicated, because some other tools 
             * on iOS 15 also create the dir but we do not want to touch that
             */
            if (
                access("/var/jb/etc/bash.ro", F_OK) == 0 &&
                access("/var/jb/usr/bin/ldid3", F_OK) == 0 &&
                access("/var/jb/etc/profile.ro", F_OK) == 0 &&
                access("/var/jb/etc/apt/sources.list.d/saurik.list", F_OK) == 0
            ) {
                NSLog(CFSTR("Found unc0ver! /var/jb -> %s, deleting"), link_path);
                // removefile("/private/var/containers/Bundle/jb", NULL, 0);
                ret = unlink("/var/jb");
                return ret;
            }
            NSLog(CFSTR("Not removing /var/jb -> %s"), link_path);
            return 0;
        }
        return 0;
    } else if (S_ISDIR(st.st_mode)) {
        /* if any of these directories exists, do not delete /var/jb */
        if (
            access("/var/jb/System", F_OK) == 0 ||
            access("/var/jb/boot", F_OK) == 0 ||
            access("/var/jb/usr", F_OK) == 0 ||
            access("/var/jb/mnt", F_OK) == 0 ||
            access("/var/jb/lib", F_OK) == 0 ||
            access("/var/jb/procursus", F_OK) == 0 ||
            access("/var/jb/.bootstrapped", F_OK) == 0 ||
            access("/var/jb/.procursus_strapped", F_OK) == 0
            ) {
                return 0;
        }
        if (access("/var/jb/var/mobile/Library", F_OK) == 0 ||
            access("/var/jb/var/root/Library", F_OK) == 0) {
            NSLog(CFSTR("Found rootlesshooks artifact /var/jb, deleting"));
            return removefile("/var/jb", NULL, REMOVEFILE_RECURSIVE);
        }
        NSLog(CFSTR("Not deleting (probably bogus) /var/jb directory"));
    } else {
        return 0;
    }

    return 0;
}

int create_var_jb(void) {
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
