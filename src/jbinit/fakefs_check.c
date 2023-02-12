#include <jbinit.h>

void pinfo_check(bool* use_fakefs_p, char* bootargs, char* dev_rootdev) {
  char statbuf[0x400];
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful)) {
    snprintf(dev_rootdev, 0x20, "/dev/%s", pinfo.rootdev);
    *use_fakefs_p = true;
  }

  if (checkrain_option_enabled(info.flags, checkrain_option_force_revert)) {
    *use_fakefs_p = false;
  }

  if (checkrain_option_enabled(pinfo.flags, palerain_option_setup_rootful)) {
    *use_fakefs_p = false;
    if (!checkrain_option_enabled(pinfo.flags, palerain_option_rootful)) {
      printf("cannot have palerain_option_setup_rootful when palerain_option_rootful is unset\n");
      spin();
    }
    if (strstr(bootargs, "wdt=-1") == NULL) {
      printf("cannot have palerain_option_setup_rootful without wdt=-1 in boot-args\n");
      spin();
    }
    if (stat(dev_rootdev, statbuf) == 0) {
      if (!checkrain_option_enabled(pinfo.flags, palerain_option_setup_rootful_forced)) {
        printf("cannot create fakefs over an existing one without palerain_option_setup_rootful_forced\n");
        spin();
      }
    }
  } else if (checkrain_option_enabled(pinfo.flags, palerain_option_setup_partial_root)) {
    printf("cannot have palerain_option_setup_partial_root without palerain_option_setup_rootful\n");
    spin();
  }
}
