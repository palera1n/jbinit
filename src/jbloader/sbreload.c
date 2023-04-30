#include <jbloader.h>

int sbreload()
{
  if (checkrain_options_enabled(pinfo.flags, palerain_option_rootful))
  {
    if (access("/usr/bin/sbreload", F_OK) != 0)
      return 0;
    char *args[] = {
        "/usr/bin/sbreload",
        NULL};
    return run(args[0], args);
  }
  else
  {
    if (access("/var/jb/usr/bin/sbreload", F_OK) != 0)
      return 0;
    char *args[] = {
        "/var/jb/usr/bin/sbreload",
        NULL};
    return run(args[0], args);
  }
}
