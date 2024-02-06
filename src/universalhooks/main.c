#include <mach-o/dyld.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <universalhooks/hooks.h>
#include <inttypes.h>

__attribute__((constructor))void universalhooks_main(void) {
    char path[PATH_MAX];
    uint32_t pathmax = PATH_MAX;
    if (_NSGetExecutablePath(path, &pathmax)) {
        return;
    }

    if (strcmp(path, "/usr/libexec/securityd") == 0
     || strcmp(path, "/usr/libexec/trustd") == 0) securitydInit();
    
    else if (strcmp(path, "/usr/libexec/watchdogd") == 0) watchdogdInit();
}
