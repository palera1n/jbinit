#include <mach-o/dyld.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <rootlesshooks/hooks.h>
#include <inttypes.h>

__attribute__((constructor))void rootlesshooks_main(void) {
    char path[PATH_MAX];
    uint32_t pathmax = PATH_MAX;
    if (_NSGetExecutablePath(path, &pathmax)) {
        return;
    }

    if (!strcmp(path, "/usr/sbin/cfprefsd")) cfprefsdInit();
    else if (!strcmp(path, "/System/Library/CoreServices/SpringBoard.app/SpringBoard")) sbInit();
    /*else if (!strcmp(path, "/usr/libexec/lsd")) lsdInit(); */
}
