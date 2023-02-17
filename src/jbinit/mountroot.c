#include <jbinit.h>
#include <common.h>

void mountroot(char* rootdev, uint64_t rootlivefs, int rootopts) {
    char statbuf[0x400];
    char buf[0x100];
    struct apfs_mountarg arg = {
        rootdev,
        0,
        rootlivefs, // 1 mount without snapshot, 0 mount snapshot
        0,
    };
    int err = 0;
retry_rootfs_mount:
    printf("mounting rootfs %s\n", rootdev);
    err = mount("apfs", "/", rootopts | MNT_RDONLY, &arg);
    if (!err) {
      puts("mount rootfs OK");
    } else {
      printf("mount rootfs %s FAILED with err=%d!\n", rootdev, err);
      sleep(1);
      // spin();
    }
    if ((err = stat("/sbin/mount", statbuf))) {
      printf("stat %s FAILED with err=%d!\n", "/sbin/mount", err);
      sleep(1);
      goto retry_rootfs_mount;
    } else {
      printf("stat %s OK\n", "/sbin/mount");
    }
}
