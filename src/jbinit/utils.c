#include "jbinit.h"
#include "printf.h"

char slash_fs_slash_orig[] = "/fs/orig";
char slash_fs_slash_orig_slash_private[] = "/fs/orig/private";
char slash[] = "/";
char slash_private[] = "/private";

void read_directory(int fd, void (*dir_cb)(struct dirent *))
{
  off_t pos = 0;
  char buf[sizeof(struct dirent)];
  do
  {
    pos = getdirentries64(fd, buf, sizeof(buf), &pos);
    if (pos == 0)
      break;
    struct dirent *entry = (struct dirent *)buf;
    while ((char*)entry < buf + sizeof(buf))
    {
      /*if (entry->d_ino == 0 || entry->d_namlen == 0)
        break;*/
      if (entry->d_type == DT_UNKNOWN || entry->d_reclen <= 0 || entry->d_seekoff != 0)
        break;
      // should always be fine, as directory names cannot be empty and must be null terminated
      if (*(uint16_t*)entry->d_name == 0xffff)
        break;
      dir_cb(entry);
      entry = (struct dirent *)((char *)entry + entry->d_reclen);
    }
  } while (pos);
}

void spin()
{
  puts("jbinit DIED!");
  while (1)
  {
    sleep(5);
  }
}

void _putchar(char character)
{
  static size_t chrcnt = 0;
  static char buf[0x100];
  buf[chrcnt++] = character;
  if (character == '\n' || chrcnt == sizeof(buf))
  {
    write(STDOUT_FILENO, buf, chrcnt);
    chrcnt = 0;
  }
}
