#include <jbinit.h>

void remount_rdisk(bool use_fakefs, char* dev_rootdev) {
    char *path = "/dev/md0";
    int err = mount("hfs", "/", MNT_UPDATE | MNT_RDONLY, &path);
    if (!err)
    {
      LOG("remount rdisk OK\n");
    }
    else
    {
      LOG("remount rdisk FAIL\n");
    }
}
