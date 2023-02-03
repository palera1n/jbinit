#include <jbloader.h>

int run(const char *cmd, char *const *args)
{
  int pid = 0;
  int retval = 0;
  char printbuf[0x1000] = {};
  for (char *const *a = args; *a; a++)
  {
    size_t csize = strlen(printbuf);
    if (csize >= sizeof(printbuf))
      break;
    snprintf(printbuf + csize, sizeof(printbuf) - csize, "%s ", *a);
  }

  retval = posix_spawn(&pid, cmd, NULL, NULL, args, environ);
  printf("Executing: %s (posix_spawn returned: %d)\n", printbuf, retval);
  {
    int pidret = 0;
    printf("waiting for '%s' to finish...\n", printbuf);
    retval = waitpid(pid, &pidret, 0);
    printf("waitpid for '%s' returned: %d\n", printbuf, retval);
    return pidret;
  }
  return retval;
}


int run_async(const char *cmd, char *const *args)
{
  int pid = 0;
  int retval = 0;
  char printbuf[0x1000] = {};
  for (char *const *a = args; *a; a++)
  {
    size_t csize = strlen(printbuf);
    if (csize >= sizeof(printbuf))
      break;
    snprintf(printbuf + csize, sizeof(printbuf) - csize, "%s ", *a);
  }
  retval = posix_spawn(&pid, cmd, NULL, NULL, args, NULL);
  printf("Asynchronous execution: %s (posix_spawn returned: %d)\n", printbuf, retval);
  return retval;
}

