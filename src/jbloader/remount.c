#include <jbloader.h>
#include <common.h>
int remount(char *rootdev)
{
  int ret = 0;
  if (fakefs_is_in_use)
  {
    char dev_rootdev[0x20];
    snprintf(dev_rootdev, 0x20, "/dev/%s", rootdev);
    struct apfs_mountarg arg = {
      .path = dev_rootdev,
      ._null = 0,
      .apfs_flags = 1,
      ._pad = 0,
    };
    ret = mount("apfs", "/", MNT_UPDATE, &arg);
    if (ret) {
      fprintf(stderr, "could not remount /: %d (%s)\n", errno, strerror(errno));
      return ret;
    }
  }
  struct statfs fs;
  if ((ret = statfs("/private/preboot", &fs))) {
    fprintf(stderr, "could not statfs /private/preboot: %d (%s)\n", errno, strerror(errno));
    return ret;
  }
  struct apfs_mountarg preboot_arg = {
    .path = fs.f_mntfromname,
    ._null = 0,
    .apfs_flags = 1,
    ._pad = 0,
  };
  ret = mount("apfs", "/private/preboot", MNT_UPDATE, &preboot_arg);
  if (ret) {
    fprintf(stderr, "could not remount /private/preboot: %d (%s)\n", errno, strerror(errno));
    return ret;
  }
  return ret;
}
