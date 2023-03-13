#include "jbinit.h"

__attribute__((naked)) kern_return_t thread_switch(mach_port_t new_thread, int option, mach_msg_timeout_t time)
{
  asm(
      "movn x16, #0x3c\n"
      "svc 0x80\n"
      "ret\n");
}

__attribute__((naked)) uint64_t msyscall(uint64_t syscall, ...)
{
  asm(
      "mov x16, x0\n"
      "ldp x0, x1, [sp]\n"
      "ldp x2, x3, [sp, 0x10]\n"
      "ldp x4, x5, [sp, 0x20]\n"
      "ldp x6, x7, [sp, 0x30]\n"
      "svc 0x80\n"
      "ret\n");
}

void sleep(int secs)
{
  thread_switch(0, 2, secs * 1000);
}

int sys_dup2(int from, int to)
{
  return msyscall(90, from, to);
}

int stat(void *path, void *ub)
{
  return msyscall(188, path, ub);
}

int mkdir(void *path, int mode)
{
  return msyscall(136, path, mode);
}

int mount(const char *type, char *path, int flags, void *data)
{
  return msyscall(167, type, path, flags, data);
}

int unmount(char *path, int flags)
{
  return msyscall(159, path, flags);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, uint64_t offset)
{
  return (void *)msyscall(197, addr, length, prot, flags, fd, offset);
}

uint64_t read(int fd, void *cbuf, size_t nbyte)
{
  return msyscall(3, fd, cbuf, nbyte);
}

uint64_t write(int fd, void *cbuf, size_t nbyte)
{
  return msyscall(4, fd, cbuf, nbyte);
}

int close(int fd)
{
  return msyscall(6, fd);
}

int open(void *path, int flags, int mode)
{
  return msyscall(5, path, flags, mode);
}

int execve(char *fname, char *const argv[], char *const envp[])
{
  return msyscall(59, fname, argv, envp);
}

int symlink(char* path, char* link) {
  return msyscall(57, path, link);
}

uint64_t lseek(int fildes, int32_t offset, int whence)
{
  return msyscall(199, fildes, offset, whence);
}

int sys_sysctlbyname(const char *name, size_t namelen, void *old, size_t *oldlenp, void *new, size_t newlen)
{
  return msyscall(274, name, namelen, old, oldlenp, new, newlen);
}

int statfs64(char *path, struct statfs64 *buf) {
  return msyscall(345, path, buf);
}

int chroot(char* path) {
  return msyscall(61, path);
}

int chdir(char* path) {
  return msyscall(12, path);
}

ssize_t getdirentries64(int fd, void *buf, size_t bufsize, off_t *position)
{
  return msyscall(344, fd, buf, bufsize, position);
}

int munmap(void* addr, size_t len) {
  return msyscall(73, addr, len);
}
