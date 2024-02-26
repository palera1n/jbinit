#include <fakedyld/fakedyld.h>

__attribute__((naked)) kern_return_t thread_switch(mach_port_t new_thread, int option, mach_msg_timeout_t time)
{
  __asm__(
      "movn x16, #0x3c\n"
      "svc 0x80\n"
      "ret\n");
}

static int __errno = 0;
int* __error() {
  return &__errno;
}

void sleep(int secs){
  thread_switch(0, 2, secs * 1000);
}

void exit(int rval) {
  msyscall(SYS_exit, rval);
  return;
}

ssize_t read(int fd, void *cbuf, size_t nbyte) {
  return msyscall(SYS_read, fd, cbuf, nbyte);
}

ssize_t write(int fd, void *cbuf, size_t nbyte) {
  return msyscall(SYS_write, fd, cbuf, nbyte);
}

int open(char* path, int flags, int mode) {
  return msyscall(SYS_open, path, flags, mode);
}

int close(int fd) {
  return msyscall(SYS_close, fd);
}

int link(char* path, char* link) {
  return msyscall(SYS_link, path, link);
}

int unlink(char* path) {
  return msyscall(SYS_unlink, path);
}

int chdir(char* path) {
  return msyscall(SYS_chdir, path);
}

int fchdir(int fd) {
  return msyscall(SYS_fchdir, fd);
}

int mknod(char* path, int mode, int dev) {
  return msyscall(SYS_mknod, path, mode, dev);
}

int chmod(char* path, int mode) {
  return msyscall(SYS_chmod, path, mode);
}

int chown(char* path, int uid, int gid) {
  return msyscall(SYS_chown, path, uid, gid);
}

int reboot(int opt, char* msg) {
  return msyscall(SYS_reboot, opt, msg);
}

int symlink(char *path, char *link) {
  return msyscall(SYS_symlink, path, link);
}

int execve(char* fname, char** argp, char** envp) {
  return msyscall(SYS_execve, fname, argp, envp);
}

int chroot(char* path) {
  return msyscall(SYS_chroot, path);
}

int munmap(void* addr, size_t len) {
  return msyscall(SYS_munmap, addr, len);
}

int dup2(int from, int to) {
  return msyscall(SYS_dup2, from, to);
}

int mkdir(char* path, int mode) {
  return msyscall(SYS_mkdir, path, mode);
}

int rmdir(char* path) {
  return msyscall(SYS_rmdir, path);
}

int unmount(char *path, int flags) {
  return msyscall(SYS_unmount, path, flags);
}

int mount(char* type, char* path, int flags, void* data) {
  return msyscall(SYS_mount, type, path, flags, data);
}

/*int stat(void *path, struct stat *ub) {
  return msyscall(SYS_stat, path, ub);
}*/

int stat64(void *path, struct stat64 *ub) {
  return msyscall(SYS_stat64, path, ub);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, uint64_t offset) {
  return (void*)msyscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}

uint64_t lseek(int fildes, int32_t offset, int whence) {
  return msyscall(SYS_lseek, fildes, offset, whence);
}

int sys_sysctlbyname(const char *name, size_t namelen, void *old, size_t *oldlenp, void *new, size_t newlen) {
  return msyscall(SYS_sys_sysctlbyname, name, namelen, old, oldlenp, new, newlen);
}

ssize_t getdirentries64(int fd, void *buf, size_t bufsize, off_t *position) {
  return msyscall(SYS_getdirentries64, fd, buf, bufsize, position);
}

int statfs64(char *path, struct statfs64 *buf) {
  return msyscall(SYS_statfs64, path,buf);
}

int fs_snapshot(uint32_t op, int dirfd, const char* name1, const char* name2, void* data, uint32_t flags) {
  return msyscall(SYS_fs_snapshot, op, dirfd, name1, name2, data, flags);
}

int posix_spawn(pid_t *pid, const char *path, const void* adesc, char **argv, char **envp) {
  return msyscall(SYS_posix_spawn, pid, path, adesc, argv, envp);
}

int wait4(int pid, int* status, int options, void* rusage) {
  return msyscall(SYS_wait4, pid, status, options, rusage);
}

_Noreturn void abort_with_payload(uint32_t reason_namespace, uint64_t reason_code, void *payload, uint32_t payload_size, const char *reason_string, uint64_t reason_flags) {
  msyscall(SYS_abort_with_payload, reason_namespace, reason_code, payload, payload_size, reason_string, reason_flags);
  __asm__("b .");
  __builtin_unreachable();
}
