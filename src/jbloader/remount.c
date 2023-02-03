#include <jbloader.h>

int remount(char *rootdev)
{
  if (fakefs_is_in_use)
  {
    char dev_rootdev[0x20];
    snprintf(dev_rootdev, 0x20, "/dev/%s", rootdev);
    char *args[] = {
        "/sbin/mount_apfs",
        "-o",
        "rw,update",
        dev_rootdev,
        "/",
        NULL};
    run(args[0], args);
  }
  char *args[] = {
      "/sbin/mount",
      "-uw",
      "/private/preboot",
      NULL};
  return run(args[0], args);
}
