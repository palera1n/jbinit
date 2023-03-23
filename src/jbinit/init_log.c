#include <jbinit.h>
#include <common.h>

void init_log(const char* dev_rootdev) {
    int fd_log = open("/cores/jbinit.log", O_WRONLY | O_TRUNC | O_SYNC | O_CREAT, 0644);
    if (fd_log != -1)
    {
        sys_dup2(fd_log, 1);
        sys_dup2(fd_log, 2);
        puts("======== jbinit log start =========");
    } else puts("cannot open /cores/jbinit.log for logging");

    LOG("root device: %s\n", dev_rootdev);
    LOG("kbase: 0x%llx\nkslide: 0x%llx\n", info.base, info.slide);
    LOG("kerninfo flags: 0x%08x\npaleinfo flags: 0x%08x\npaleinfo version: %u\n",info.flags, pinfo.flags, pinfo.version);
    char kver_buf[0x100];
    size_t kver_sz = sizeof(kver_buf);
    int err = sys_sysctlbyname("kern.version", sizeof("kern.version"), kver_buf, &kver_sz, NULL, 0);
    if (err)
        LOG("getting kernel version failed: %d\n", err);
    else
        LOG("kernel version: %s\n", kver_buf);
}
