#include <jbinit.h>
#include <common.h>

bool darwin22 = false;

void rootwait(char** rootdev_pp) {
    struct stat statbuf;
      LOG("Checking for roots\n");
  {
    while (stat(ios15_rootdev, &statbuf) && stat(ios16_rootdev, &statbuf))
    {
      LOG("waiting for roots...\n");
      sleep(1);
    }
  }
  if (stat(ios15_rootdev, &statbuf))
  {
    *rootdev_pp = ios16_rootdev;
    darwin22 = true;
  }
  else
    *rootdev_pp = ios15_rootdev;
  LOG("Got rootfs %s\n", *rootdev_pp);
}
