#include "jbinit.h"
#include "printf.h"
#include <kerninfo.h>

int get_kerninfo(struct kerninfo *info, char *rd)
{
  uint32_t size = sizeof(struct kerninfo);
  uint32_t ramdisk_size_actual;
  int fd = open(rd, O_RDONLY, 0);
  read(fd, &ramdisk_size_actual, 4);
  lseek(fd, (long)(ramdisk_size_actual), SEEK_SET);
  int64_t didread = read(fd, info, sizeof(struct kerninfo));
  if ((unsigned long)didread != sizeof(struct kerninfo) || info->size < (uint64_t)sizeof(struct kerninfo))
  {
    return -1;
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
    LOG("Detected corrupted paleinfo!\n");
    return -1;
  }
  if (info->version != 1)
  {
    LOG("Unsupported paleinfo %u (expected 1)\n", info->version);
    return -1;
  }
  close(fd);
  return 0;
}

void get_info() {
  {
    int err = get_kerninfo(&info, RAMDISK);
    if (err)
    {
      LOG("cannot get kerninfo!");
      spin();
    }
  }

  {
    int err = get_paleinfo(&pinfo, RAMDISK);
    if (err)
    {
      LOG("cannot get paleinfo!");
      spin();
    }
  }
}
