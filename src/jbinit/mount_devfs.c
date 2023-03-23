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
