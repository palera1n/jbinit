#include <fakedyld/fakedyld.h>
#include <mount_args.h>

#ifdef ASAN
#define CORES_SIZE 8388608
#else
#define CORES_SIZE 2097152
#endif

void mount_tmpfs_cores() {
    int err = 0;
    int64_t pagesize;
    unsigned long pagesize_len = sizeof(pagesize);
    err = sys_sysctlbyname("hw.pagesize", sizeof("hw.pagesize"), &pagesize, &pagesize_len, NULL, 0);
    if (err != 0)
    {
        LOG("cannot get pagesize, err=%d", errno);
        spin();
    }
    LOG("system page size: %lld", pagesize);
    {
        struct tmpfs_mount_args arg = {.max_pages = (CORES_SIZE / pagesize), .max_nodes = UINT8_MAX, .case_insensitive = 0};
        err = mount("tmpfs", "/cores", 0, &arg);
        if (err != 0)
        {
          LOG("cannot mount tmpfs onto /cores, err=%d", errno);
          spin();
        }
        LOG("mounted tmpfs onto /cores");
    }
}

void cores_mkdir(char* path) {
    struct stat64 statbuf;
    int err = mkdir(path, 0755);
    if (err) {
        LOG("mkdir(%s) FAILED with err %d", path, errno);
    }
    if (stat64(path, &statbuf)) {
        LOG("stat %s FAILED with err=%d!", path, errno);
        spin();
    }
    else
        LOG("created %s", path);
}

void mount_ramdisk_cores(int platform) {
  char executable[30];
  switch (platform) {
    case PLATFORM_IOS:
    case PLATFORM_TVOS:
    case PLATFORM_BRIDGEOS:
      snprintf(executable, 30, "/mount_cores.%d", platform);
      break;
    default:
      LOG("mount_cores: unsupported platform");
      spin();
  }
  pid_t pid;
  int ret = posix_spawn(&pid, executable, NULL, NULL, NULL);
  printf("spawned pid %d\n", pid);
  if (ret != 0) {
    LOG("failed to spawn %s: %d", executable, errno);
    spin();
  }
  ret = wait4(-1, NULL, 0, NULL);
  printf("wait4 retval: %d\n", ret);
  if (ret == -1) {
    printf("wait4 errno: %d\n", errno);
  }
}

void init_cores(struct systeminfo* sysinfo_p, int platform) {
  if (sysinfo_p->osrelease.darwinMajor > 20) {
    tmpfs_mount_args_t args = { .case_insensitive = 0, .max_pages = 1, .max_nodes = 1 };
    int ret = mount("tmpfs", "/System/Library/PrivateFrameworks/ProgressUI.framework", MNT_DONTBROWSE, &args);
    if (ret) {
      printf("mount ProgressUI tmpfs error: %d\n", errno);
    } else {
      printf("Blanked ProgressUI\n");
      /* for some reason tmpfs does not support readonly first mounts */
      ret = mount("tmpfs", "/System/Library/PrivateFrameworks/ProgressUI.framework", MNT_RDONLY | MNT_DONTBROWSE | MNT_UPDATE , &args);
      if (ret) {
        printf("remount ProgressUI tmpfs read-only failed with error: %d\n", errno);
      } else {
        printf("Blank ProgressUI made read-only\n");
      }
    }
  }

  if (sysinfo_p->osrelease.darwinMajor < 20) {
    return;
  } else if (sysinfo_p->osrelease.darwinMajor < 21) {
    mount_tmpfs_cores();
    cores_mkdir("/cores/binpack");
    cores_mkdir("/cores/fs");
    cores_mkdir("/cores/fs/real");
    cores_mkdir("/cores/fs/fake");
    cores_mkdir("/cores/usr");
    cores_mkdir("/cores/usr/lib");
    symlink("/payload", "/cores/payload");
    symlink("/payload.dylib", "/cores/payload.dylib");
    symlink("/mount_cores.2", "/cores/mount_cores.2");
    symlink("/mount_cores.3", "/cores/mount_cores.3");
    symlink("/mount_cores.5", "/cores/mount_cores.5");
  } else {
    char device_path[] = "/dev/md0";
    struct hfs_mount_args cores_mountarg = { device_path, 0, 0, 0, 0, { 0, 0 }, HFSFSMNT_EXTENDED_ARGS, 0, 0, 1 };
    if (sysinfo_p->xnuMajor < 7938) {
      return;
    }
    CHECK_ERROR(mount("hfs", "/cores", MNT_UPDATE, &cores_mountarg), "remount /cores failed");
    CHECK_ERROR(unlink("/cores/usr/lib/dyld"), "delete fakedyld failed");
  }
}
