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
#include <sys/stat.h>
#include <sys/mount.h>
#include <mount_args.h>

int remount_rootfs(struct utsname* name_p) {
    struct statfs fs;
    int ret = statfs("/", &fs);
    if (ret) return ret;
    int mntflags = MNT_UPDATE;
    if (atoi(name_p->release) < 21) mntflags |= MNT_UNION;
    apfs_mount_args_t arg = { fs.f_mntfromname, 0, APFS_MOUNT_FILESYSTEM, 0, 0, { "" }, NULL, 0, 0, NULL, 0, 0, 0, 0, 0, 0 };
    return mount(fs.f_fstypename, fs.f_mntonname, mntflags, &arg);
}

int remount_preboot(struct utsname* __unused name_p) {
    struct statfs fs;
    int ret = statfs("/private/preboot", &fs);
    if (ret == ENOENT) return 0;
    if (ret) return ret;
    int mntflags = MNT_UPDATE;
    apfs_mount_args_t arg = { fs.f_mntfromname, 0, APFS_MOUNT_FILESYSTEM, 0, 0, { "" }, NULL, 0, 0, NULL, 0, 0, 0, 0, 0, 0 };
    return mount(fs.f_fstypename, fs.f_mntonname, mntflags, &arg);
}

int remount(void) {
    int ret;
    struct utsname name;
    ret = uname(&name);
    if (ret) {
        fprintf(stderr, "uname() failed: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    if (
        access("/.installed_palera1n", F_OK) == 0 ||
        access("/.mount_rw", F_OK) == 0 ||
        access("/.procursus_strapped", F_OK) == 0
    )
        {
            ret = remount_rootfs(&name);
            if (ret) {
                fprintf(stderr, "mount(/) failed: %d (%s)\n", errno, strerror(errno));
                return ret;
            }
        }
    ret = remount_preboot(&name);
    if (ret) {
        fprintf(stderr, "mount(/private/preboot) failed: %d (%s)\n", errno, strerror(errno));
        return ret;
    }
    return 0;
}
