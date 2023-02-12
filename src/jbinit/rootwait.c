#include <jbinit.h>
#include <common.h>

void rootwait(char** rootdev_pp) {
    char statbuf[0x400];
      puts("Checking for roots");
  {
    while (stat(ios15_rootdev, statbuf) && stat(ios16_rootdev, statbuf))
    {
      puts("waiting for roots...");
      sleep(1);
    }
  }
  if (stat(ios15_rootdev, statbuf))
  {
    *rootdev_pp = ios16_rootdev;
  }
  else
    *rootdev_pp = ios15_rootdev;
  printf("Got rootfs %s\n", *rootdev_pp);
}
