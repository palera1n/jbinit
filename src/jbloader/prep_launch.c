#include <jbloader.h>

void *prep_jb_launch(void *__unused _)
{
  assert(info.size == sizeof(struct kerninfo));
  if (checkrain_option_enabled(info.flags, checkrain_option_force_revert))
  {
    jailbreak_obliterator();
    return NULL;
  }
  if (checkrain_option_enabled(info.flags, checkrain_option_safemode))
  {
    printf("Safe mode is enabled\n");
  }
  else
  {
    loadDaemons();
  }
  return NULL;
}

int loadDaemons()
{
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
  {
    if (access("/Library/LaunchDaemons", F_OK) != 0)
      return 0;
    {
      char *args[] = {
          "/bin/launchctl",
          "load",
          "/Library/LaunchDaemons",
          NULL};
      run_async(args[0], args);
    }
  }
  else
  {
    if (access("/var/jb/Library/LaunchDaemons", F_OK) != 0)
      return 0;
    {
      char *args[] = {
          "/var/jb/bin/launchctl",
          "load",
          "/var/jb/Library/LaunchDaemons",
          NULL};
      run_async(args[0], args);
    }
  }
  return 0;
}
