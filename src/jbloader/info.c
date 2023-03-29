#include <jbloader.h>

int get_kerninfo(struct kerninfo *info, char *rd)
{
  uint32_t ramdisk_size_actual;
  errno = 0;
  int fd = open(rd, O_RDONLY);
  if (fd == -1)
    return errno;
  read(fd, &ramdisk_size_actual, 4);
  lseek(fd, (long)(ramdisk_size_actual), SEEK_SET);
  if (errno != 0)
    return errno;
  ssize_t didread = read(fd, info, sizeof(struct kerninfo));
  if ((unsigned long)didread != sizeof(struct kerninfo) || info->size != (uint64_t)sizeof(struct kerninfo))
  {
    return EINVAL;
  }
  close(fd);
  return 0;
}

int get_paleinfo(struct paleinfo *info, char *rd)
{
  uint32_t ramdisk_size_actual;
  int fd = open(rd, O_RDONLY, 0);
  read(fd, &ramdisk_size_actual, 4);
  lseek(fd, (long)(ramdisk_size_actual) + 0x1000L, SEEK_SET);
  int64_t didread = read(fd, info, sizeof(struct paleinfo));
  if ((unsigned long)didread != sizeof(struct paleinfo))
  {
    return -1;
  }
  if (info->magic != PALEINFO_MAGIC)
  {
    printf("Detected corrupted paleinfo!\n");
    return -1;
  }
  if (info->version != 1)
  {
    printf("Unsupported paleinfo %u (expected 1)\n", info->version);
    return -1;
  }
  close(fd);
  return 0;
}

const char* str_checkrain_flags(checkrain_option_t opt) {
  const char* ret = NULL;
  switch(opt) {
    case checkrain_option_safemode:
      ret = "checkrain_option_safemode";
      break;
    case checkrain_option_bind_mount:
      ret = "checkrain_option_bind_mount";
      break;
    case checkrain_option_overlay:
      ret = "checkrain_option_overlay";
      break;
    case checkrain_option_force_revert:
      ret = "checkrain_option_force_revert";
      break;
    default:
      ret = NULL;
      break;
  }
  return ret;
}

const char* str_palerain_flags(checkrain_option_t opt) {
  const char* ret = NULL;
  switch(opt) {
    case palerain_option_rootful:
      ret = "palerain_option_rootful";
      break;
    case palerain_option_jbinit_log_to_file:
      ret = "palerain_option_jbinit_log_to_file";
      break;
    case palerain_option_setup_rootful:
      ret = "palerain_option_setup_rootful";
      break;
    case palerain_option_setup_rootful_forced:
      ret = "palerain_option_setup_rootful_forced";
      break;
    case palerain_option_setup_partial_root:
      ret = "palerain_option_setup_partial_root";
      break;
    case palerain_option_test1:
      ret = "palerain_option_test1";
      break;
    case palerain_option_test2:
      ret = "palerain_option_test2";
      break;
    case palerain_option_checkrain_is_clone:
      ret = "palerain_option_checkrain_is_clone";
      break;
    case palerain_option_rootless_livefs:
      ret = "palerain_option_rootless_livefs";
      break;
    default:
      ret = NULL;
      break;
  }
  return ret;
}
