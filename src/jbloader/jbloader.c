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
        jbloader_flags |= jbloader_is_palera1nd;
        break;
      case '?':
        goto out;
        break;
    }
  }
  if (checkrain_option_enabled(jbloader_flags, jbloader_is_sysstatuscheck)) {
    return jbloader_sysstatuscheck(argc, argv);
  } else if (checkrain_option_enabled(jbloader_flags, jbloader_is_palera1nd)) {
    return jbloader_palera1nd(argc, argv);
  }
out:
  puts("this is a palera1n internal utility, do not run");
  return -1;
}
