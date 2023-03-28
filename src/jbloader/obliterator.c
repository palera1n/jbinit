#include <jbloader.h>

int jailbreak_obliterator()
{

  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
  {
  run("/cores/binpack/bin/rm", (char*[]){
      "/cores/binpack/bin/rm",
      "-rf",
      "/var/jb",
      "/var/lib",
      "/var/cache",
    NULL});
  }
  else
  {
    printf("Obliterating jailbraek\n");
    char hash[97];
    char prebootPath[150] = "/private/preboot/";
    memset(hash, '\0', sizeof(hash));
    int ret = get_boot_manifest_hash(hash);
    if (ret != 0)
    {
      fprintf(stderr, "cannot get boot manifest hash\n");
      return ret;
    }
    printf("boot manifest hash: %s\n", hash);
    if (access("/var/jb/Applications", F_OK) == 0)
    {
      printf("unregistering applications\n");
      DIR *d = NULL;
      struct dirent *dir = NULL;
      if (!(d = opendir("/var/jb/Applications")))
      {
        fprintf(stderr, "Failed to open dir with err=%d (%s)\n", errno, strerror(errno));
        return -1;
      }
      while ((dir = readdir(d)))
      { // remove all subdirs and files
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
        {
          continue;
        }
        char *pp = NULL;
        asprintf(&pp, "/var/jb/Applications/%s", dir->d_name);
        {
          char *args[] = {
              "/cores/binpack/usr/bin/uicache",
              "-u",
              pp,
              NULL};
          run(args[0], args);
        }
        free(pp);
      }
      closedir(d);
    }

    printf("Apps now unregistered\n");
    strncat(prebootPath, hash, 150 - sizeof("/procursus") - sizeof("/private/preboot"));
    strncat(prebootPath, "/procursus", 150 - 97 - sizeof("/private/preboot/"));
    printf("prebootPath: %s\n", prebootPath);
    printf("%lu\n", strlen(hash));
    printf("%lu\n", strlen("/private/preboot/") + strlen(hash) + strlen("/procursus"));
    // yeah we don't want rm -rf /private/preboot
    assert(strlen(prebootPath) == strlen("/private/preboot/") + strlen(hash) + strlen("/procursus"));
    run("/cores/binpack/bin/rm", (char*[]){
        "/cores/binpack/bin/rm",
        "-rf",
        "/var/jb",
        prebootPath,
        "/var/lib",
        "/var/cache",
        "/var/LIB",
        "/var/Liy",
        "/var/ulb",
        "/var/bin",
        "/var/sbin",
        "/var/ubi",
        "/var/local",
        NULL
      });
    run("/cores/binpack/usr/bin/uicache", (char*[]){
        "/cores/binpack/usr/bin/uicache",
        "-af",
        NULL});
    printf("Jailbreak obliterated\n");
  }
  return 0;
}
