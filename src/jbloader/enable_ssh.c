#include <jbloader.h>

void *enable_ssh(void *__unused _)
{
  xpc_object_t msg;
  if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0)
  {
    char *dropbearkey_argv[] = {"/cores/binpack/usr/bin/dropbearkey", "-f", "/private/var/dropbear_rsa_host_key", "-t", "rsa", "-s", "4096", NULL};
    run(dropbearkey_argv[0], dropbearkey_argv);
  }
  puts("loading /cores/binpack/Library/LaunchDaemons/dropbear.plist");
  char* load_argv[] = { "load", "-w", "/cores/binpack/Library/LaunchDaemons/dropbear.plist", NULL };
  load_cmd(&msg, 3, load_argv, environ, launchctl_apple);
  return NULL;
}
