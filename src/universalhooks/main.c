#include <mach-o/dyld.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <universalhooks/hooks.h>
#include <paleinfo.h>
#include <inttypes.h>
#include <unistd.h>

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

bool stringEndsWith(const char* str, const char* suffix)
{
    if (!str || !suffix) {
        return false;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (str_len < suffix_len) {
        return false;
    }

    return !strcmp(str + str_len - suffix_len, suffix);
}

uint64_t pflags;
bool rootful;
__attribute__((constructor))void universalhooks_main(void) {
    pflags = strtoull(getenv("JB_PINFO_FLAGS"), NULL, 0);
    rootful = pflags & palerain_option_rootful;
    
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
    
    if (stringEndsWith(path, "/TrollStore.app/trollstorehelper")) {
        trollstorehelperInit(path);
    }
}
