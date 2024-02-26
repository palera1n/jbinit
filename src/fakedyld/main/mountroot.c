#include <fakedyld/fakedyld.h>
#include <mount_args.h>

/*
 * If the current platform has SSV, palerain_option_ssv
 * will be set by PongoOS in pinfo->flags
 * 
 * PongoOS will always put something in pinfo->rootdev.
 * 
 * The user can choose between "rootful" and "rootless",
 * The definition of rootful (fakefs or realfs) will be
 * determined by PongoOS by setting the palerain_option_ssv
 * flag.
 * 
 * If neither rootful and rootless is set, PongoOS will choose
 * one for the user based on whether the system has SSV.
 * 
 * So, rootful or rootless must be set by the time we get here
*/

void mountroot(struct paleinfo* pinfo_p, struct systeminfo* sysinfo_p) {
    /* we trust pongoOS and mount whatever is in pinfo_p->rootdev */
    /* ...unless rootful setup is requested */
    int ret;
    char dev_rootdev[32];
    char dev_realfs[32];
    if ((pinfo_p->flags & palerain_option_setup_rootful) == 0) {
        snprintf(dev_rootdev, 32, "/dev/%s", pinfo_p->rootdev);
    } else if (sysinfo_p->osrelease.darwinMajor < 22) {
        snprintf(dev_rootdev, 32, "/dev/%s", DARWIN21_ROOTDEV);
    } else {
        snprintf(dev_rootdev, 32, "/dev/%s", DARWIN22_ROOTDEV);
    }
    LOG("waiting for roots...");
    /* wait for realfs */
    if (sysinfo_p->osrelease.darwinMajor < 22) {
        snprintf(dev_realfs, 32, "/dev/%s", DARWIN21_ROOTDEV);
    } else {
        snprintf(dev_realfs, 32, "/dev/%s", DARWIN22_ROOTDEV);
    }
    struct stat64 st;
    while ((ret = stat64(dev_realfs, &st))) {
        if (ret != 2) {
            LOG("wait realfs error: %d", errno);
        }
        sleep(1);
    }
    /* since realfs had shown up, target fs should be here too if it exists */
    if ((ret = stat64(dev_rootdev, &st))) {
        panic("cannot find target fs %s: %d", dev_rootdev, errno);
    }
    int rootopts = MNT_RDONLY;
    if (!(pinfo_p->flags & palerain_option_bind_mount)) {
        rootopts |= MNT_UNION;
    }
    apfs_mount_args_t rootargs = { dev_rootdev, rootopts, APFS_MOUNT_FILESYSTEM };
retry_rootfs_mount:
    ret = mount("apfs", "/", rootopts, &rootargs);
    if (ret) {
        LOG("mount rootfs %s failed: %d");
        sleep(1);
        goto retry_rootfs_mount;
    }
    if ((ret = stat64("/private/", &st))) {
      LOG("stat %s FAILED with err=%d!", "/private/", errno);
      sleep(1);
      goto retry_rootfs_mount;
    } else {
      LOG("stat %s OK\n", "/private/");
    }
}
