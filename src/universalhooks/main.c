#include <mach-o/dyld.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <universalhooks/hooks.h>
#include <paleinfo.h>
#include <inttypes.h>

struct hook_info {
    const char* executablePath;
    void (*rootlessInit)(void);
    void (*rootfulInit)(void);
    void (*universalInit)(void);
};

struct hook_info info[] = {
    { "/usr/libexec/securityd", NULL, NULL, securitydInit },
    { "/usr/libexec/trustd", NULL, NULL, securitydInit },
    { "/usr/libexec/watchdogd", NULL, NULL, watchdogdInit },
    { "/System/Library/CoreServices/SpringBoard.app/SpringBoard", springboardInit, NULL, NULL },
    { "/usr/libexec/lsd", lsdRootlessInit, NULL, lsdUniversalInit },
    { "/usr/sbin/cfprefsd", cfprefsdInit, NULL, NULL },
    { "/Applications/PineBoard.app/PineBoard", NULL, NULL, pineboardInit },
    { "/Applications/HeadBoard.app/HeadBoard", NULL, NULL, headboardInit },
};

__attribute__((constructor))void universalhooks_main(void) {
    uint64_t pflags = strtoull(getenv("JB_PINFO_FLAGS"), NULL, 0);
    bool rootful = pflags & palerain_option_rootful;
    
    char path[PATH_MAX];
    uint32_t pathmax = PATH_MAX;
    if (_NSGetExecutablePath(path, &pathmax)) {
        return;
    }
    
    for (size_t i = 0; i < (sizeof(info) / sizeof(struct hook_info)); i++) {
        if (strcmp(path, info[i].executablePath)) continue;
        if (rootful && info[i].rootfulInit) info[i].rootfulInit();
        else if (info[i].rootlessInit && !rootful) info[i].rootlessInit();
        
        if (info[i].universalInit) info[i].universalInit();
    }
}
