#include <jbloader.h>

void spin()
{
  puts("jbinit DIED!");
  if (access("/cores/binpack/bin/sh", F_OK) == 0) {
    putenv("PATH=/cores/binpack/usr/bin:/cores/binpack/usr/sbin:/cores/binpack/sbin:/cores/binpack/bin:/usr/bin:/usr/sbin:/bin:/sbin");
    puts("Welcome to jbinit DIED emergency shell!");
    char* shell_argv[] = {
      "/cores/binpack/bin/sh",
      "-i",
      NULL
    };
    run(shell_argv[0], shell_argv);
  }
  while (1)
  {
    sleep(5);
  }
}
