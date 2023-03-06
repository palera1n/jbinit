#include <jbinit.h>

void remount_rdisk(bool use_fakefs, char* dev_rootdev) {
    char *path = "/dev/md0";
    int err = mount("hfs", "/", MNT_UPDATE | MNT_RDONLY, &path);
    if (!err)
    {
      puts("remount rdisk OK");
    }
    else
    {
      puts("remount rdisk FAIL");
    }
}
