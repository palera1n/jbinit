#include <jbinit.h>
#include <common.h>

void mountroot(char* rootdev, uint64_t rootlivefs, int rootopts) {
    char statbuf[0x400];
    struct statfs64 fst;
    char buf[0x100];
    int err = statfs64("/", &fst);
    if (err) {
      printf("statfs64(/) failed with err=%d\n", err);
      spin();
    }
    printf("%s on %s type %s flags=%u\n", fst.f_mntfromname, fst.f_mntonname, fst.f_fstypename, fst.f_flags);
    struct apfs_mountarg arg = {
        rootdev,
        0,
        rootlivefs, // 1 mount without snapshot, 0 mount snapshot
        0,
    };
retry_rootfs_mount:
    printf("mounting rootfs %s\n", rootdev);
    err = mount("apfs", "/", rootopts | MNT_RDONLY, &arg);
    if (!err) {
      puts("mount rootfs OK");
    } else {
      printf("mount rootfs %s FAILED with err=%d!\n", rootdev, err);
      sleep(1);
      goto retry_rootfs_mount;
      // spin();
    }
    err = statfs64("/", &fst);
    if (err) {
      printf("statfs64(/) failed with err=%d\n", err);
      spin();
    }
    if (stat("/sbin/fsck", statbuf)) {
      printf("stat %s FAILED with err=%d!\n", "/sbin/fsck", err);
      sleep(1);
      goto retry_rootfs_mount;
    } else {
      printf("stat %s OK\n", "/sbin/fsck");
    }
    printf("%s on %s type %s flags=%u\n", fst.f_mntfromname, fst.f_mntonname, fst.f_fstypename, fst.f_flags);
}

