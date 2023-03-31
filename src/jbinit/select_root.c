#include <jbinit.h>
#include <common.h>

void select_root(uint64_t* rootlivefs_p, int* rootopts_p, char** rootdev_p, char* dev_rootdev, bool use_fakefs) {
  if (checkrain_option_enabled(info.flags, checkrain_option_bind_mount)) {
    LOG("bind mounts are enabled\n");
    *rootlivefs_p = 0;
  } else {
    LOG("WARNING: BIND MOUNTS ARE DISABLED!");
    *rootopts_p |= MNT_UNION;
    *rootlivefs_p = 1;
  }
  if (use_fakefs)
  {
    *rootdev_p = dev_rootdev;
    *rootlivefs_p = 1;
  }
  if (checkrain_option_enabled(pinfo.flags, palerain_option_setup_rootful))
  {
    *rootlivefs_p = 1;
  }
  if (!checkrain_option_enabled(pinfo.flags, palerain_option_rootful) &&
    checkrain_option_enabled(pinfo.flags,palerain_option_rootless_livefs)) {
      *rootlivefs_p = 1;
  }
  LOG("will be using %s as rootfs\n", *rootdev_p);
}
