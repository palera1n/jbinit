#include <jbinit.h>

void mount_devfs() {
    LOG("mounting devfs\n");
    {
      char *path = "devfs";
      int err = mount("devfs", "/dev/", 0, path);
      if (!err)
      {
        LOG("mount devfs OK\n");
      }
      else
      {
        LOG("mount devfs FAILED with err=%d!\n", err);
        spin();
      }
    }
}

void unmount_devfs() {
  LOG("unmounting devfs");
  int ret = unmount("/dev", MNT_FORCE);
  if (ret) {
    LOG("umount(/dev) failed with err=%d!\n", ret);
    spin();
  } else {
    LOG("unmounted devfs\n");
  }
}
