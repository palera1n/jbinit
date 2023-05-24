#include <jbloader.h>
#include <mach-o/loader.h>

void *enable_ssh(void *__unused _)
{
  xpc_object_t msg;
  if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0)
  {
    puts("generating /private/var/dropbear_rsa_host_key, this will take some time...");
    run("/cores/binpack/usr/bin/dropbearkey",
     (char*[]){"/cores/binpack/usr/bin/dropbearkey", 
     "-f", 
     "/private/var/dropbear_rsa_host_key", 
     "-t", 
     "rsa", 
     "-s", 
     "4096",
     NULL});
  }
  /* WTF */
  char* dropbear_plist = NULL;
  switch (dyld_platform) {
    case PLATFORM_IOS:
      dropbear_plist = "/cores/binpack/Library/LaunchDaemons/DropBear/dropbear.plist";
      break;
    case PLATFORM_BRIDGEOS:
      dropbear_plist = "/cores/binpack/Library/LaunchDaemons/DropBear/dropbear-bridgeos-ncm.plist";
      break;
    case PLATFORM_TVOS:
    default:
      dropbear_plist = "/cores/binpack/Library/LaunchDaemons/DropBear/dropbear-tv.plist";
      break;
  }
  printf("loading %s\n", dropbear_plist);
  int ret;
  if (checkrain_options_enabled(jbloader_flags, jbloader_userspace_rebooted)) {
    ret = load_cmd(&msg, 4, (char*[]){ 
      "load",
      "-w",
      dropbear_plist,
      dropbear_plist, 
      NULL
    }, environ, launchctl_apple);
  } else {
    ret = load_cmd(&msg, 3, (char*[]){ 
        "load",
        "-w",
        dropbear_plist,
        NULL
      }, environ, launchctl_apple);
  }
  printf("load_cmd returned %d\n", ret);
  return NULL;
}
