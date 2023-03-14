#include <jbloader.h>
#include <mach-o/loader.h>

unsigned long darwinMajor = -1;

int jbloader_palera1nd(int argc, char **argv)
{
  pthread_mutex_init(&safemode_mutex, NULL);
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
  char kern_osrelease[0x100];
  size_t sizeof_kern_osrelease = sizeof(kern_osrelease);
  int osrelease_ret = sysctlbyname("kern.osrelease", kern_osrelease, &sizeof_kern_osrelease, NULL, 0);
  if (osrelease_ret != 0) {
    fprintf(stderr, "cannot get kernel release: %d (%s)\n", errno, strerror(errno));
    return -1;
  }
  darwinMajor = strtoul(kern_osrelease, NULL, 0);
  if (darwinMajor == 0) {
    fprintf(stderr, "failed to convert kernel release %s to integer\n", kern_osrelease);
    return -1;
  }
  printf("Running on Darwin %lu\n", darwinMajor);
  pthread_t ssh_thread, prep_jb_launch_thread, prep_jb_ui_thread;
  pthread_create(&prep_jb_launch_thread, NULL, prep_jb_launch, NULL);
  pthread_create(&ssh_thread, NULL, enable_ssh, NULL);
  pthread_join(prep_jb_launch_thread, NULL);
  if (!checkrain_option_enabled(info.flags, checkrain_option_force_revert)
  && dyld_platform == PLATFORM_IOS)
  {
    pthread_create(&prep_jb_ui_thread, NULL, prep_jb_ui, NULL);
  }
  pthread_join(ssh_thread, NULL);
  if (!checkrain_option_enabled(info.flags, checkrain_option_force_revert) && dyld_platform == PLATFORM_IOS)
    pthread_join(prep_jb_ui_thread, NULL);
  if (dyld_platform == PLATFORM_IOS)
    uicache_loader();
  if (
    checkrain_option_enabled(info.flags, checkrain_option_safemode)
  && dyld_platform == PLATFORM_IOS
  )
  {
    void *sbservices = dlopen(
      "/System/Library/PrivateFrameworks/SpringBoardServices.framework/"
      "SpringBoardServices",
      RTLD_NOW);
      if (sbservices != NULL) {
        CFNotificationCenterAddObserver(
          CFNotificationCenterGetDarwinNotifyCenter(), NULL, &safemode_alert,
          CFSTR("SBSpringBoardDidLaunchNotification"), NULL, 0);
      void *(*SBSSpringBoardServerPort)() = dlsym(sbservices, "SBSSpringBoardServerPort");
      if (SBSSpringBoardServerPort() == NULL) {
        dispatch_main();
      }
      } else {
        printf("sbservices is NULL: %s\n", dlerror());
        set_safemode_spin(false);
      }

  }
  else
  {
    set_safemode_spin(false);
  }
  uint8_t i = 0;
  while (get_safemode_spin() && i < 180) {
    i += 3;
    sleep(3);
  }
  printf("palera1n: goodbye!\n");
  printf("========================================\n");
  pthread_mutex_destroy(&safemode_mutex);
  return 0;
}
