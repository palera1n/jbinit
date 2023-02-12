#include <jbinit.h>

void mount_devfs() {
    puts("mounting devfs");
    {
      char *path = "devfs";
      int err = mount("devfs", "/dev/", 0, path);
      if (!err)
      {
        puts("mount devfs OK");
      }
      else
      {
        printf("mount devfs FAILED with err=%d!\n", err);
        spin();
      }
    }
}
