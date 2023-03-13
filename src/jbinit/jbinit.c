#include "jbinit.h"
#include "printf.h"
#include <kerninfo.h>
#include <common.h>

asm(
    ".globl __dyld_start\n"
    ".align 4\n"
    "__dyld_start:\n"
    "movn x8, #0xf\n"
    "mov x7, sp\n"
    "and x7, x7, x8\n"
    "mov sp, x7\n"
    "bl _main\n"
    "movz x16, #0x1\n"
    "svc #0x80\n");

struct kerninfo info;
struct paleinfo pinfo;

char ios15_rootdev[] = "/dev/disk0s1s1";
char ios16_rootdev[] = "/dev/disk1s1";

int main()
{
  int fd_console = open("/dev/console", O_RDWR | O_SYNC, 0);
  sys_dup2(fd_console, 0);
  sys_dup2(fd_console, 1);
  sys_dup2(fd_console, 2);
  char statbuf[0x400];
  char bootargs[0x270]; 

  puts("================ Hello from jbinit ================");
  {
    unsigned long bootargs_len = sizeof(bootargs);
    int err = sys_sysctlbyname("kern.bootargs", sizeof("kern.bootargs"), &bootargs, &bootargs_len, NULL, 0);
    if (err) {
      printf("cannot get bootargs: %d\n", err);
      spin();
    }
    printf("boot-args = %s\n", bootargs);
  }

  size_t jbloader_size;
  void* jbloader_data = read_file("/sbin/launchd", &jbloader_size);

  size_t dylib_size;
  void* dylib_data = read_file("/jbin/jb.dylib", &dylib_size);

#ifdef ASAN
  size_t asan_size;
  void* asan_data = read_file("/jbin/libclang_rt.asan_ios_dynamic.dylib", &asan_size);

  size_t ubsan_size;
  void* ubsan_data = read_file("/jbin/libclang_rt.ubsan_ios_dynamic.dylib", &ubsan_size);
#endif

  char* rootdev;

  rootwait(&rootdev);
  mount_devfs();
  get_info();

  char dev_rootdev[0x20] = "/dev/";
  bool use_fakefs = false;
  pinfo_check(&use_fakefs, bootargs, dev_rootdev);

  uint64_t rootlivefs;
  int rootopts = MNT_RDONLY;

  select_root(&rootlivefs, &rootopts, &rootdev, dev_rootdev, use_fakefs);
  remount_rdisk(use_fakefs, dev_rootdev);
  mountroot(rootdev, rootlivefs, rootopts);
  mount_devfs();
  mount_cores();

  if (checkrain_option_enabled(pinfo.flags, palerain_option_jbinit_log_to_file))
    init_log(dev_rootdev);

  init_cores();
  patch_dyld();
  write_file("/cores/jbloader", jbloader_data, jbloader_size);
  write_file("/cores/jb.dylib", dylib_data, dylib_size);
#ifdef ASAN
  write_file("/cores/libclang_rt.asan_ios_dynamic.dylib", asan_data, asan_size);
  write_file("/cores/libclang_rt.ubsan_ios_dynamic.dylib", ubsan_data, ubsan_size);
#endif

  prepare_rootfs(dev_rootdev, use_fakefs);

  puts("Closing console, goodbye!");
  /*
    Launchd doesn't like it when the console is open already!
  */
  for (size_t i = 0; i < 10; i++)
  {
    close(i);
  }
  {
    char **argv = (char **)jbloader_data;
    char **envp = argv + 2;
    char *strbuf = (char *)(envp + 2);
    argv[0] = strbuf;
    argv[1] = NULL;
    memcpy(strbuf, "/cores/jbloader", sizeof("/cores/jbloader"));
    strbuf += sizeof("/cores/jbloader");
    int err = execve(argv[0], argv, NULL);
    if (err)
    {
      printf("execve FAILED with err=%d!\n", err);
      spin();
    }
  }
  puts("FATAL: shouldn't get here!");
  spin();

  return 0;
}
