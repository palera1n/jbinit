#include <jbloader.h>

void *enable_ssh(void *__unused _)
{
  if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0)
  {
    char *dropbearkey_argv[] = {"/cores/binpack/usr/bin/dropbearkey", "-f", "/private/var/dropbear_rsa_host_key", "-t", "rsa", "-s", "4096", NULL};
    run(dropbearkey_argv[0], dropbearkey_argv);
  }
  char *launchctl_argv[] = {"/cores/binpack/bin/launchctl", "load", "-w", "/cores/binpack/Library/LaunchDaemons/dropbear.plist", NULL};
  run(launchctl_argv[0], launchctl_argv);
  return NULL;
}
