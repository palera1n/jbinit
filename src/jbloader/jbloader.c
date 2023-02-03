#include <jbloader.h>

int jbloader_main(int argc, char **argv)
{
  setvbuf(stdout, NULL, _IONBF, 0);
  if (checkrain_option_enabled(pinfo.flags, palerain_option_jbinit_log_to_file))
  {
    int fd_log = open("/cores/jbinit.log", O_WRONLY | O_APPEND | O_SYNC, 0644);
    if (fd_log != -1)
    {
      dup2(fd_log, STDOUT_FILENO);
      dup2(fd_log, STDERR_FILENO);
      puts("======== jbloader (system boot) log start =========");
    }
    else
      puts("cannot open /cores/jbinit.log for logging");
  }
  printf("========================================\n");
  printf("palera1n: init!\n");
  printf("pid: %d\n", getpid());
  printf("uid: %d\n", getuid());
  pthread_t ssh_thread, prep_jb_launch_thread, prep_jb_ui_thread;
  pthread_create(&prep_jb_launch_thread, NULL, prep_jb_launch, NULL);
  pthread_create(&ssh_thread, NULL, enable_ssh, NULL);
  pthread_join(prep_jb_launch_thread, NULL);
  if (!checkrain_option_enabled(info.flags, checkrain_option_force_revert))
  {
    pthread_create(&prep_jb_ui_thread, NULL, prep_jb_ui, NULL);
  }
  pthread_join(ssh_thread, NULL);
  if (!checkrain_option_enabled(info.flags, checkrain_option_force_revert))
    pthread_join(prep_jb_ui_thread, NULL);
  uicache_loader();
  if (checkrain_option_enabled(info.flags, checkrain_option_safemode))
  {
    CFNotificationCenterAddObserver(
        CFNotificationCenterGetDarwinNotifyCenter(), NULL, &safemode_alert,
        CFSTR("SBSpringBoardDidLaunchNotification"), NULL, 0);
    void *sbservices = dlopen(
        "/System/Library/PrivateFrameworks/SpringBoardServices.framework/"
        "SpringBoardServices",
        RTLD_NOW);
    void *(*SBSSpringBoardServerPort)() = dlsym(sbservices, "SBSSpringBoardServerPort");
    if (SBSSpringBoardServerPort() == NULL) {
      dispatch_main();
    }
  }
  else
  {
    safemode_spin = false;
  }
  uint8_t i = 0;
  while (safemode_spin && i < 180) {
    i += 3;
    sleep(3);
  }
  printf("palera1n: goodbye!\n");
  printf("========================================\n");

  return 0;
}
