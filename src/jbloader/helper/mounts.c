// 
//  mounts.c
//  src/jbloader/helper/mounts.c
//  
//  Created 04/05/2023
//  jbloader (helper)
//

#include <jbloader.h>

struct statfs64 __DARWIN_STRUCT_STATFS64;

void fstatfs64(int fd, struct statfs64 *buf) {
    register int x0 asm("x0") = fd; // int fd
    register struct statfs64 *x1 asm("x1") = buf; // struct statfs64 *buf
    register long x16 asm("x16") = 346; // FSTATFS64 syscall number

    asm volatile("svc #0x80"
                :"=r"(x0), "=r"(x1) 
                :"r"(x0), "r"(x1), "r"(x16): 
                "memory",
                "cc");
}

void statfs64(char *path, struct statfs64 *buf) {
    register char* x0 asm("x0") = path; // char *path
    register struct statfs64 *x1 asm("x1") = buf; // struct statfs64 *buf
    register long x16 asm("x16") = 345; // STATFS64 syscall number

    asm volatile("svc #0x80"
                :"=r"(x0), "=r"(x1) 
                :"r"(x0), "r"(x1), "r"(x16): 
                "memory",
                "cc");
}

int mount_check(const char *mountpoint) {
    int mnt_fd = open(mountpoint, O_RDONLY, 0);
     if (mnt_fd < 0) {
        fprintf(stderr, "%s %s\n", "Failed to open", mountpoint);
        return -1;
    }

    struct statfs64 fs_stat;
    fstatfs64(mnt_fd, &fs_stat);
    close(mnt_fd);

    if (strcmp(fs_stat.f_mntonname, mountpoint) != 0) {
        fprintf(stderr, "%s %s %s\n", fs_stat.f_mntfromname, "is not mounted on", mountpoint);
        return -1;
    }

    return 0;
}
