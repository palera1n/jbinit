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

  int fd_jbloader = 0;
  fd_jbloader = open("/sbin/launchd", O_RDONLY, 0);
  if (fd_jbloader == -1)
  {
    spin();
  }
  size_t jbloader_size = lseek(fd_jbloader, 0, SEEK_END);
  lseek(fd_jbloader, 0, SEEK_SET);
  void *jbloader_data = mmap(NULL, (jbloader_size & ~0x3fff) + 0x4000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (jbloader_data == (void *)-1)
  {
    spin();
  }
  int didread = read(fd_jbloader, jbloader_data, jbloader_size);
  close(fd_jbloader);

  int fd_dylib = 0;
  fd_dylib = open("/jbin/jb.dylib", O_RDONLY, 0);
  if (fd_dylib == -1)
  {
    spin();
  }
  size_t dylib_size = lseek(fd_dylib, 0, SEEK_END);
  lseek(fd_dylib, 0, SEEK_SET);
  void *dylib_data = mmap(NULL, (dylib_size & ~0x3fff) + 0x4000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (dylib_data == (void *)-1)
  {
    spin();
  }
  didread = read(fd_dylib, dylib_data, dylib_size);
  close(fd_jbloader);

  puts("Checking for roots");
  char *rootdev;
  {
    while (stat(ios15_rootdev, statbuf) && stat(ios16_rootdev, statbuf))
    {
      puts("waiting for roots...");
      sleep(1);
    }
  }
  if (stat(ios15_rootdev, statbuf))
  {
    rootdev = ios16_rootdev;
  }
  else
    rootdev = ios15_rootdev;
  printf("Got rootfs %s\n", rootdev);

  puts("mounting devfs");
  {
    char *path = "devfs";
    int err = mount("devfs", "/dev/", 0, path);
    if (!err)
    {
      puts("mount devfs OK");
    }
    else
    {
      printf("mount devfs FAILED with err=%d!\n", err);
      spin();
    }
  }
  struct kerninfo info;
  {
    int err = get_kerninfo(&info, RAMDISK);
    if (err)
    {
      printf("cannot get kerninfo!");
      spin();
    }
  }
  struct paleinfo pinfo;
  {
    int err = get_paleinfo(&pinfo, RAMDISK);
    if (err)
    {
      printf("cannot get paleinfo!");
      spin();
    }
  }
  char dev_rootdev[0x20] = "/dev/";
  bool use_fakefs = false;
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful)) {
    snprintf(dev_rootdev, 0x20, "/dev/%s", pinfo.rootdev);
    use_fakefs = true;
  }

  if (checkrain_option_enabled(info.flags, checkrain_option_force_revert)) {
    use_fakefs = false;
  }

  if (checkrain_option_enabled(pinfo.flags, palerain_option_setup_rootful)) {
    use_fakefs = false;
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
  }

  uint32_t rootlivefs;
  int rootopts = MNT_RDONLY;
  if (checkrain_option_enabled(info.flags, checkrain_option_bind_mount))
  {
    printf("bind mounts are enabled\n");
    rootlivefs = 0;
  }
  else
  {
    printf("WARNING: BIND MOUNTS ARE DISABLED!");
    rootopts |= MNT_UNION;
    rootlivefs = 1;
  }
  if (use_fakefs)
  {
    rootdev = dev_rootdev;
    rootlivefs = 1;
  }
  {
    char *path = NULL;
    if (use_fakefs)
      path = dev_rootdev;
    else
      path = "/dev/md0";
    int err = mount("apfs", "/", MNT_UPDATE | MNT_RDONLY, &path);
    if (!err)
    {
      puts("remount rdisk OK");
    }
    else
    {
      puts("remount rdisk FAIL");
    }
  }
  {
    char buf[0x100];
    struct apfs_mountarg arg = {
        rootdev,
        0,
        rootlivefs, // 1 mount without snapshot, 0 mount snapshot
        0,
    };
    int err = 0;
  retry_rootfs_mount:
    puts("mounting rootfs");
    err = mount("apfs", "/", rootopts | MNT_RDONLY, &arg);
    if (!err)
    {
      puts("mount rootfs OK");
    }
    else
    {
      printf("mount rootfs %s FAILED with err=%d!\n", rootdev, err);
      sleep(1);
      // spin();
    }

    if (stat("/sbin/fsck", statbuf))
    {
      printf("stat %s FAILED with err=%d!\n", "/sbin/fsck", err);
      sleep(1);
      goto retry_rootfs_mount;
    }
    else
    {
      printf("stat %s OK\n", "/sbin/fsck");
    }

    puts("mounting devfs again");
    {
      char *path = "devfs";
      int err = mount("devfs", "/dev/", 0, path);
      if (!err)
      {
        puts("mount devfs again OK");
      }
      else
      {
        printf("mount devfs again FAILED with err=%d!\n", err);
        spin();
      }
    }
    {
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
        struct tmpfs_mountarg arg = {.max_pages = (1572864 / pagesize), .max_nodes = UINT8_MAX, .case_insensitive = 0};
        err = mount("tmpfs", "/cores", 0, &arg);
        if (err != 0)
        {
          printf("cannot mount tmpfs onto /cores, err=%d", err);
          spin();
        }
        puts("mounted tmpfs onto /cores");
      }
      {
        if (checkrain_option_enabled(pinfo.flags, palerain_option_jbinit_log_to_file))
        {
          int fd_log = open("/cores/jbinit.log", O_WRONLY | O_TRUNC | O_SYNC | O_CREAT, 0644);
          if (fd_log != -1)
          {
            sys_dup2(fd_log, 1);
            sys_dup2(fd_log, 2);
            puts("======== jbinit log start =========");
          }
          else
            puts("cannot open /cores/jbinit.log for logging");
          printf("root device: %s\n", dev_rootdev);
          printf("kbase: 0x%llx\nkslide: 0x%llx\n", info.base, info.slide);
          printf("kerninfo flags: 0x%08x\npaleinfo flags: 0x%08x\npaleinfo version: %u\n",info.flags, pinfo.flags, pinfo.version);
          char kver_buf[0x100];
          size_t kver_sz = sizeof(kver_buf);
          err = sys_sysctlbyname("kern.version", sizeof("hw.pagesize"), kver_buf, &kver_sz, NULL, 0);
          if (err)
            printf("getting kernel version failed: %d\n", err);
          else
            printf("kernel version: %s\n", kver_buf);

        }
      }

      err = mkdir("/cores/binpack", 0755);
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
    puts("deploying jbloader");
    fd_jbloader = open("/cores/jbloader", O_WRONLY | O_CREAT, 0755);
    printf("jbloader write fd=%d\n", fd_jbloader);
    if (fd_jbloader == -1)
    {
      puts("Failed to open /cores/jbloader for writing");
      spin();
    }
    int didwrite = write(fd_jbloader, jbloader_data, jbloader_size);
    printf("didwrite=%d\n", didwrite);
    close(fd_jbloader);

    puts("deploying jb.dylib");
    fd_dylib = open("/cores/jb.dylib", O_WRONLY | O_CREAT, 0755);
    printf("jb.dylib write fd=%d\n", fd_dylib);
    if (fd_dylib == -1)
    {
      puts("Failed to open /cores/jb.dylib for writing");
      spin();
    }
    didwrite = write(fd_dylib, dylib_data, dylib_size);
    printf("didwrite=%d\n", didwrite);
    close(fd_dylib);
  }

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
