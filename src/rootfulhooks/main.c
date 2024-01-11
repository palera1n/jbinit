#include <mach-o/dyld.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <rootfulhooks/hooks.h>
#include <inttypes.h>

__attribute__((constructor))void rootfulhooks_main(void) {
    char path[PATH_MAX];
    uint32_t pathmax = PATH_MAX;
    if (_NSGetExecutablePath(path, &pathmax)) {
        return;
    }

    if (!strcmp(path, "/usr/libexec/lsd")) lsdInit();
    else if (strcmp(path, "/usr/libexec/securityd") == 0
     || strcmp(path, "/usr/libexec/trustd") == 0) securitydInit();
}
