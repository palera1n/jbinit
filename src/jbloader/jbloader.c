#include <jbloader.h>
#include <libgen.h>

uint32_t jbloader_flags = 0;
int dyld_platform = -1;

int init_info() {
  int ret = get_kerninfo(&info, RAMDISK);
  if (ret != 0)
  {
    fprintf(stderr, "cannot get kerninfo: ret: %d, errno: %d (%s)\n", ret, errno, strerror(errno));
    return -1;
  }
  ret = get_paleinfo(&pinfo, RAMDISK);
  if (ret != 0)
  {
    fprintf(stderr, "cannot get paleinfo: ret: %d, errno: %d (%s)\n", ret, errno, strerror(errno));
    return -1;
  }
  return 0;
}

int jbloader_main(int argc, char *argv[])
{
#ifdef DEV_BUILD
  if (getenv("DYLD_INSERT_LIBRARIES") != NULL)
    printf("DYLD_INSERT_LIBRARIES: %s\n", getenv("DYLD_INSERT_LIBRARIES"));
  else {
    setenv("DYLD_INSERT_LIBRARIES", "/cores/xpchook.dylib", 1);
    execv("/cores/jbloader", argv);
  } 
#endif
  int ret = 0;
  int ch = 0;
  if (getuid() != 0 || geteuid() != 0) goto out;
  if ((ret = init_info())) return ret;
  if ((dyld_platform = get_dyld_platform()) == -1) {
    return -1;
  }
  if (getpid() == 1) {
    return jbloader_launchd(argc, argv);
  };
  
  while ((ch = getopt(argc, argv, "usj")) != -1) {
    switch(ch) {
      case 'u':
        jbloader_flags |= jbloader_userspace_rebooted;
        break;
      case 's':
        jbloader_flags |= jbloader_is_sysstatuscheck;
        break;
      case 'j':
        jbloader_flags |= jbloader_is_bakera1nd;
        break;
      case '?':
        goto out;
        break;
    }
  }
  if (checkrain_option_enabled(jbloader_flags, jbloader_is_sysstatuscheck)) {
    return jbloader_sysstatuscheck(argc, argv);
  } else if (checkrain_option_enabled(jbloader_flags, jbloader_is_bakera1nd)) {
    return jbloader_bakera1nd(argc, argv);
  }
out:
  puts("this is a palera1n internal utility, do not run");
  return -1;
}
