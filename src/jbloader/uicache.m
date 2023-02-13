#include <jbloader.h>
#include <Foundation/Foundation.h>

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
  NSMutableDictionary* md = [[NSMutableDictionary alloc] initWithContentsOfFile:@"/var/mobile/Library/Preferences/com.apple.springboard.plist"];
  if([md objectForKey:@"SBShowNonDefaultSystemApps"] == nil)
  {
    char *arg1[] = { "/cores/binpack/usr/bin/killall", "-SIGSTOP", "cfprefsd", NULL };
    run(arg1[0], arg1);

    // add SBShowNonDefaultSystemApps key
    [md setObject:[NSNumber numberWithBool:YES] forKey:@"SBShowNonDefaultSystemApps"];
    [md writeToFile:@"/var/mobile/Library/Preferences/com.apple.springboard.plist" atomically:YES];

    char *arg2[] = { "/cores/binpack/usr/bin/killall", "-SIGKILL", "cfprefsd", NULL };
    run(arg2[0], arg2);

    char *arg3[] = { "/cores/binpack/usr/sbin/chown", "501:501", "/var/mobile/Library/Preferences/com.apple.springboard.plist", NULL };
    run(arg3[0], arg3);
  }

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
