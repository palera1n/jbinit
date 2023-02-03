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
