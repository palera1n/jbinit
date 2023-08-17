#ifndef FAKEDYLD_SYSCALLS_H
#define FAKEDYLD_SYSCALLS_H

#include <fakedyld/types.h>
#include <stddef.h>

#define SYS_exit                1
#define SYS_fork                2
#define SYS_read                3
#define SYS_write               4
#define SYS_open                5
#define SYS_close               6
#define SYS_wait4               7
#define SYS_link                9
#define SYS_unlink              10
#define SYS_chdir               12
#define SYS_fchdir              13
#define SYS_mknod               14
#define SYS_chmod               15
#define SYS_chown               16
#define SYS_symlink             57
#define SYS_execve              59
#define SYS_chroot              61
#define SYS_munmap              73
#define SYS_dup2                90
#define SYS_mkdir               136
#define SYS_rmdir               137
#define SYS_unmount             159
#define SYS_mount               167
#define SYS_stat                188
#define SYS_mmap                197
#define SYS_lseek               199
#define SYS_posix_spawn         244
#define SYS_sys_sysctlbyname    274
#define SYS_getdirentries64     344
#define SYS_statfs64            345
#define SYS_fs_snapshot         518

uint64_t msyscall(uint64_t syscall, ...);

void sleep(int secs);
/* keep this list in order of syscall numbers */
void exit(int rval);
int fork();
ssize_t read(int fd, void *cbuf, size_t nbyte);
ssize_t write(int fd, void *cbuf, size_t nbyte);
int open(char* path, int flags, int mode);
int close(int fd);
int link(char* path, char* link);
int unlink(char* path);
int chdir(char* path);
int fchdir(int fd);
int mknod(char* path, int mode, int dev);
int chmod(char* path, int mode);
int chown(char* path, int uid, int gid);
int symlink(char *path, char *link);
int execve(char* fname, char** argp, char** envp);
int chroot(char* path);
int munmap(void* addr, size_t len);
int dup2(int from, int to);
int mkdir(char* path, int mode);
int rmdir(char* path);
int unmount(char *path, int flags);
int mount(char* type, char* path, int flags, void* data);
int stat(void *path, struct stat *ub);
void *mmap(void *addr, size_t length, int prot, int flags, int fd, uint64_t offset);
uint64_t lseek(int fildes, int32_t offset, int whence);
int sys_sysctlbyname(const char *name, size_t namelen, void *old, size_t *oldlenp, void *new_, size_t newlen);
ssize_t getdirentries64(int fd, void *buf, size_t bufsize, off_t *position);
int statfs64(char *path, struct statfs64 *buf);
int fs_snapshot(uint32_t op, int dirfd, const char* name1, const char* name2, void* data, uint32_t flags);
int wait4(int pid, int* status, int options, void* rusage);

#endif
