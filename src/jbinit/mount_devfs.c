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

void unmount_devfs(char* rootdev, int* fd_console_p) {
  struct stat rootdev_st;
  struct stat console_st;
  int err = stat(rootdev, &rootdev_st);
  if (err) {
    LOG("failed to stat %s, err=%d\n",rootdev, err);
    spin();
  }
  err = stat("/dev/console", &console_st);
  if (err) {
    LOG("failed to stat %s, err=%d\n","/dev/console", err);
    spin();
  }
  for (size_t i = 0; i < 10; i++)
  {
    close(i);
  }
  LOG("unmounting devfs\n");
  int ret = unmount("/dev", MNT_FORCE);
  if (ret) {
    LOG("umount(/dev) failed with err=%d!\n", ret);
    spin();
  } else {
    LOG("unmounted devfs\n");
    exit(1);
  }
  err = mknod(rootdev, rootdev_st.st_mode, rootdev_st.st_rdev);
  if (err) {
    LOG("failed to mknod %s, err=%d\n", rootdev, err);
    spin();
  }
  err = mknod(rootdev, console_st.st_mode, console_st.st_rdev);
  if (err) {
    LOG("failed to mknod %s, err=%d\n", "/dev/console", err);
    spin();
  }
  *fd_console_p = open("/dev/console", O_RDWR | O_SYNC, 0);
  sys_dup2(*fd_console_p , 0);
  sys_dup2(*fd_console_p , 1);
  sys_dup2(*fd_console_p , 2);
}
