#include <jbloader.h>

bool userspace_rebooted = false, is_sysstatuscheck = false, is_jbloader = false;
bool print_info = false;

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

int main(int argc, char *argv[])
{
  int ret = 0;
  int ch = 0;
  if (getuid() != 0 || geteuid() != 0) goto out;
  if ((ret = init_info())) return ret;
  if (getpid() == 1) {
    return launchd_main(argc, argv);
  };
  while ((ch = getopt(argc, argv, "usjl")) != -1) {
    switch(ch) {
      case 'u':
        userspace_rebooted = true;
        break;
      case 's':
        is_sysstatuscheck = true;
        break;
      case 'j':
        is_jbloader = true;
        break;
      case 'l':
        print_info = true;
        break;
      case '?':
        goto out;
        break;
    }
  }
  if (is_sysstatuscheck) {
    return sysstatuscheck_main(argc, argv);
  } else if (is_jbloader) {
    return jbloader_main(argc, argv);
  } else if (print_info) {
    return print_info_main(argc, argv);
  }
out:
  puts("this is a palera1n internal utility, do not run");
  return -1;
}
