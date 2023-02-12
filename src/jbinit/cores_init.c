#include <jbinit.h>
#include <common.h>

void mount_cores() {
    int err = 0;
    int64_t pagesize;
    unsigned long pagesize_len = sizeof(pagesize);
    err = sys_sysctlbyname("hw.pagesize", sizeof("hw.pagesize"), &pagesize, &pagesize_len, NULL, 0);
    if (err != 0)
    {
        printf("cannot get pagesize, err=%d\n", err);
        spin();
    }
    printf("system page size: %lld\n", pagesize);
    {
        struct tmpfs_mountarg arg = {.max_pages = (2097152 / pagesize), .max_nodes = UINT8_MAX, .case_insensitive = 0};
        err = mount("tmpfs", "/cores", 0, &arg);
        if (err != 0)
        {
          printf("cannot mount tmpfs onto /cores, err=%d", err);
          spin();
        }
        puts("mounted tmpfs onto /cores");
    }
}

void init_cores() {
    char statbuf[0x400];
    int err = mkdir("/cores/binpack", 0755);
      if (err)
      {
        printf("mkdir(/cores/binpack) FAILED with err %d\n", err);
      }
      if (stat("/cores/binpack", statbuf))
      {
        printf("stat %s FAILED with err=%d!\n", "/cores/binpack", err);
        spin();
      }
      else
        puts("created /cores/binpack");
}
