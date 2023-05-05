#include <jbloader.h>

int check_and_mount_dmg()
{
  if (access("/cores/binpack/bin/sh", F_OK) != -1)
  {
    /* binpack already mounted */
    return 0;
  }
  if (access("/cores/binpack", F_OK) != 0)
  {
    fprintf(stderr, "/cores/binpack cannot be accessed! errno=%d\n", errno);
    return -1;
  }
  return mount_dmg("ramfile://checkra1n", "hfs", "/cores/binpack", MNT_RDONLY, true);
}

int check_and_mount_loader()
{
  if (access("/cores/binpack/Applications/palera1nLoader.app", F_OK) != -1)
  {
    /* loader already mounted */
    return 0;
  }
  if (access("/cores/binpack/Applications", F_OK) != 0)
  {
    fprintf(stderr, "/cores/binpack/Applications cannot be accessed! errno=%d\n", errno);
    return -1;
  }
  return mount_dmg("/cores/binpack/loader.dmg", "hfs", "/cores/binpack/Applications", MNT_RDONLY, false);
}

__attribute__((naked)) static inline uint64_t msyscall(uint64_t syscall, ...)
{
  asm(
      "mov x16, x0\n"
      "ldp x0, x1, [sp]\n"
      "ldp x2, x3, [sp, 0x10]\n"
      "ldp x4, x5, [sp, 0x20]\n"
      "ldp x6, x7, [sp, 0x30]\n"
      "svc 0x80\n"
      "ret\n");
}

int jbloader_early(int argc, char **argv)
{
  int fd_console = open("/dev/console", O_RDWR);
  if (fd_console == -1) {
    char buf[0x100];
    snprintf(buf, 0x100, "jbloader cannot open /dev/console: %d (%s)", errno, strerror(errno));
    reboot_np(RB_PANIC, buf); // crash
    return -1;
  }
  dup2(fd_console, STDIN_FILENO);
  dup2(fd_console, STDOUT_FILENO);
  dup2(fd_console, STDERR_FILENO);
  int ret = 0;
  if (checkrain_options_enabled(pinfo.flags, palerain_option_jbinit_log_to_file))
  {
    int fd_log = open("/cores/jbinit.log", O_WRONLY | O_APPEND | O_SYNC, 0644);
    if (fd_log != -1)
    {
      dup2(fd_log, STDOUT_FILENO);
      dup2(fd_log, STDERR_FILENO);
      fputs("======== jbloader (launchd) log start =========", stderr);
    }
    else
      fputs("cannot open /cores/jbinit.log for logging", stderr);
  }
  fputs(
    "#============================\n"
    "# palera1n loader (jbloader-early) \n"
    "#  (c) palera1n develope r   \n"
    "#=============================\n"
  , stderr);
  if (getenv("XPC_USERSPACE_REBOOTED") == NULL) {
    int mount_ret = 0;
    fputs("mounting overlay\n", stderr);
    mount_ret = check_and_mount_dmg();
    if (mount_ret)
      spin();
    fputs("mounting loader\n", stderr);
    mount_ret = check_and_mount_loader();
    if (mount_ret)
      spin();
    create_remove_fakefs();
  }
  puts("Closing console, goodbye!");
  /*
    Launchd doesn't like it when the console is open already!
  */
  for (size_t i = 0; i < 10; i++)
  {
    close(i);
  }
  return 0;
}
