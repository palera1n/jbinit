#include <jbloader.h>

int create_remove_fakefs() {
  if (checkrain_option_enabled(info.flags, checkrain_option_force_revert) && checkrain_option_enabled(pinfo.flags, palerain_option_rootful)) {
    if (pinfo.rootdev[strlen(pinfo.rootdev) - 1] == '1') {
      printf("avoiding self destruction by user error\n");
      return 0;
    } 
    kern_return_t delete_ret = APFSVolumeDelete(pinfo.rootdev);
    if (delete_ret != KERN_SUCCESS) {
      fprintf(stderr, "cannot delete fakefs %s: %d %s\n", pinfo.rootdev, delete_ret, mach_error_string(delete_ret));
    } else {
      printf("deleted %s\n", pinfo.rootdev);
    }
  }
  if (!checkrain_option_enabled(pinfo.flags, palerain_option_setup_rootful)) return 0;
  char dev_rootdev[0x20];
  snprintf(dev_rootdev, 0x20, "/dev/%s", pinfo.rootdev);
  if (access(dev_rootdev, F_OK) == 0) {
    if (!checkrain_option_enabled(pinfo.flags, palerain_option_setup_rootful_forced)) {
      // should be unreachable because jbinit checked it
      assert(0);
    }
    kern_return_t delete_ret = APFSVolumeDelete(pinfo.rootdev);
    if (delete_ret != KERN_SUCCESS) {
      fprintf(stderr, "cannot delete existing fakefs: %d %s", delete_ret, mach_error_string(delete_ret));
      spin();
    } else {
      printf("deleted %s\n", pinfo.rootdev);
    }
  }
  assert(!mkdir("/cores/fs", 0755));
  assert(!mkdir("/cores/fs/real", 0755));
  assert(!mkdir("/cores/fs/fake", 0755));
  putenv("XPC_SERVICES_UNAVAILABLE=1");
  int fd_script = open("/cores/create_fakefs.sh", O_WRONLY | O_CREAT , 0);
  if (fd_script == -1) {
    fprintf(stderr, "cannot create /cores/create_fakefs.sh: %d (%s)\n", errno, strerror(errno));
    spin();
  }
  chmod("/cores/create_fakefs.sh", 0755);
  ssize_t didWrite = write(fd_script, create_fakefs_sh, create_fakefs_sh_len);
  if (didWrite != create_fakefs_sh_len) {
    fprintf(stderr, "wrote %zd bytes, expected %u bytes\n", didWrite, create_fakefs_sh_len);
    spin();
  }
  close(fd_script);
  char* setup_fakefs_argv[] = {
    "/cores/binpack/bin/sh",
    "/cores/create_fakefs.sh",
    dev_rootdev,
    NULL
  };
  int pidret = run(setup_fakefs_argv[0], setup_fakefs_argv);
  if (!WIFEXITED(pidret)) {
    int termsig = 0;
    if (WIFSIGNALED(pidret)) {
      termsig = WTERMSIG(pidret);
      fprintf(stderr, "/cores/create_fakefs.sh exited due to signal %d\n", termsig);
    } else {
      fprintf(stderr, "/cores/create_fakefs.sh exited abnormally\n");
    }
    spin();
  }
  if (WEXITSTATUS(pidret) != 0) {
    fprintf(stderr, "/cores/create_fakefs.sh exited with code %d\n", WEXITSTATUS(pidret));
    spin();
  }
  puts("Rebooting in 5 seconds");
  sleep(5);
  reboot_np(RB_AUTOBOOT, NULL);
  sleep(5);
  puts("reboot timed out");
  spin();
  return -1;
}
