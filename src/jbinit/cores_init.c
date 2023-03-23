#include <jbinit.h>
#include <common.h>

#ifdef ASAN
#define CORES_SIZE 8388608
#else
#define CORES_SIZE 2097152
#endif

void mount_cores() {
    int err = 0;
    int64_t pagesize;
    unsigned long pagesize_len = sizeof(pagesize);
    err = sys_sysctlbyname("hw.pagesize", sizeof("hw.pagesize"), &pagesize, &pagesize_len, NULL, 0);
    if (err != 0)
    {
        LOG("cannot get pagesize, err=%d\n", err);
        spin();
    }
    LOG("system page size: %lld\n", pagesize);
    {
        struct tmpfs_mountarg arg = {.max_pages = (CORES_SIZE / pagesize), .max_nodes = UINT8_MAX, .case_insensitive = 0};
        err = mount("tmpfs", "/cores", 0, &arg);
        if (err != 0)
        {
          LOG("cannot mount tmpfs onto /cores, err=%d", err);
          spin();
        }
        LOG("mounted tmpfs onto /cores");
    }
}

void cores_mkdir(char* path) {
    char statbuf[0x400];
    int err = mkdir(path, 0755);
    if (err) {
        LOG("mkdir(%s) FAILED with err %d\n", path, err);
    }
    if (stat(path, statbuf)) {
        LOG("stat %s FAILED with err=%d!\n", path, err);
        spin();
    }
    else
        LOG("created %s\n", path);
}

void init_cores() {
  cores_mkdir("/cores/binpack");
  cores_mkdir("/cores/fs");
  cores_mkdir("/cores/fs/real");
  cores_mkdir("/cores/fs/fake");
}
