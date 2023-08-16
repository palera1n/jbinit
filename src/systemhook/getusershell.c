#include <systemhook/libiosexec.h>
#include <systemhook/common.h>
#include <dyld-interpose.h>

#ifdef SYSTEMWIDE_IOSEXEC
char* getusershell_hook(void) {
    RET_TWC(getusershell);
}

void endusershell_hook(void) {
    RET_TWC(endusershell);
}

void setusershell_hook(void) {
    RET_TWC(setusershell);
}

DYLD_INTERPOSE(getusershell_hook, getusershell);
DYLD_INTERPOSE(endusershell_hook, endusershell);
DYLD_INTERPOSE(setusershell_hook, setusershell);
#endif
