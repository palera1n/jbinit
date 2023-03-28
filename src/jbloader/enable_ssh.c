#include <jbloader.h>

void *enable_ssh(void *__unused _)
{
  xpc_object_t msg;
  if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0)
  {
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
  puts("loading /cores/binpack/Library/LaunchDaemons/dropbear.plist");
  load_cmd(&msg, 3, (char*[]){ "load", "-w", "/cores/binpack/Library/LaunchDaemons/dropbear.plist", NULL }, environ, launchctl_apple);
  return NULL;
}
