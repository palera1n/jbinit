#include <jbloader.h>
#include <xpc/xpc.h>

extern char** environ;
char* launchctl_apple[] = { NULL };

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
  xpc_object_t msg;
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
  {
    if (access("/Library/LaunchDaemons", F_OK) != 0)
      return 0;
    puts("loading /Library/LaunchDaemons");
    char* bootstrap_argv[] = { "bootstrap", "system", "/Library/LaunchDaemons", NULL };
    bootstrap_cmd(&msg, 3, bootstrap_argv, environ, launchctl_apple);

  }
  else
  {
    if (access("/var/jb/Library/LaunchDaemons", F_OK) != 0)
      return 0;
    {
      puts("loading /var/jb/Library/LaunchDaemons");
      char* bootstrap_argv[] = { "bootstrap", "system", "/var/jb/Library/LaunchDaemons", NULL };
      bootstrap_cmd(&msg, 3, bootstrap_argv, environ, launchctl_apple);
    }
  }
  return 0;
}
