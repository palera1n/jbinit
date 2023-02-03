#include <jbloader.h>

int uicache_apps()
{
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
  {
    if (access("/usr/bin/uicache", F_OK) == 0)
    {
      {
        char *uicache_argv[] = {
            "/usr/bin/uicache",
            "-a",
            NULL};
        run_async(uicache_argv[0], uicache_argv);
        return 0;
      };
    }
    return 0;
  }
  else
  {
    if (access("/var/jb/usr/bin/uicache", F_OK) == 0)
    {
      {
        char *uicache_argv[] = {
            "/var/jb/usr/bin/uicache",
            "-a",
            NULL};
        run_async(uicache_argv[0], uicache_argv);
        return 0;
      };
    }
    return 0;
  }
}

void *prep_jb_ui(void *__unused _)
{
  uicache_apps();
  return NULL;
}

int uicache_loader()
{
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful) && (access("/jbin/loader.app", F_OK) == 0))
  {
    char *loader_uicache_argv[] = {
        "/cores/binpack/usr/bin/uicache",
        "-p",
        "/jbin/loader.app",
        NULL};
    run(loader_uicache_argv[0], loader_uicache_argv);
  }
  else
  {
    char *loader_uicache_argv[] = {
        "/cores/binpack/usr/bin/uicache",
        "-p",
        "/cores/binpack/Applications/palera1nLoader.app",
        NULL};
    run(loader_uicache_argv[0], loader_uicache_argv);
  }
  return 0;
}