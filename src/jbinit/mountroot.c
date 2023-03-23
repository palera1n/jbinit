#include <jbinit.h>
#include <common.h>

void mountroot(char* rootdev, uint64_t rootlivefs, int rootopts) {
    char statbuf[0x400];
    struct statfs64 fst;
    char buf[0x100];
    int err = statfs64("/", &fst);
    if (err) {
      LOG("statfs64(/) failed with err=%d\n", err);
      spin();
    }
    LOG("%s on %s type %s flags=%u\n", fst.f_mntfromname, fst.f_mntonname, fst.f_fstypename, fst.f_flags);
    struct apfs_mountarg arg = {
        rootdev,
        0,
        rootlivefs, // 1 mount without snapshot, 0 mount snapshot
        0,
    };
retry_rootfs_mount:
    LOG("mounting rootfs %s\n", rootdev);
    err = mount("apfs", "/", rootopts | MNT_RDONLY, &arg);
    if (!err) {
      LOG("mount rootfs OK\n");
    } else {
      LOG("mount rootfs %s FAILED with err=%d!\n", rootdev, err);
      sleep(1);
      goto retry_rootfs_mount;
      // spin();
    }
    err = statfs64("/", &fst);
    if (err) {
      LOG("statfs64(/) failed with err=%d\n", err);
      spin();
    }
    if ((err = stat("/private/", statbuf))) {
      LOG("stat %s FAILED with err=%d!\n", "/private/", err);
      sleep(1);
      goto retry_rootfs_mount;
    } else {
      LOG("stat %s OK\n", "/private/");
    }
    LOG("%s on %s type %s flags=%u\n", fst.f_mntfromname, fst.f_mntonname, fst.f_fstypename, fst.f_flags);
}

