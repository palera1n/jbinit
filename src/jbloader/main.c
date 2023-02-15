#include <jbloader.h>
#include <libgen.h>

struct kerninfo info;
struct paleinfo pinfo;

int main(int argc, char *argv[]) {
  char* name = basename(argv[0]);
  if (!strcmp(name, "jbloader")) {
    return jbloader_main(argc, argv);
  } else if (!strcmp(name, "p1ctl")) {
    return p1ctl_main(argc, argv);
  } else {
    fprintf(stderr, "%s: unknown command: %s\n", name, name);
    return -1;
  }
}

