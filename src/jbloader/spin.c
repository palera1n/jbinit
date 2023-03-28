#include <jbloader.h>

void spin()
{
  puts("########### ALERT: AN ERROR OCCURED ###########");
  puts("Most likely there are additional information above");
  if (access("/cores/binpack/bin/sh", F_OK) == 0) {
    putenv("PATH=/cores/binpack/usr/bin:/cores/binpack/usr/sbin:/cores/binpack/sbin:/cores/binpack/bin:/usr/bin:/usr/sbin:/bin:/sbin");
    puts("Welcome to serial=3 emergency shell!");
    run("/cores/binpack/bin/sh", (char*[]){
      "/cores/binpack/bin/sh",
      "-i",
      NULL
    });
  }
  puts("spinning...");
  while (1)
  {
    sleep(5);
  }
}
