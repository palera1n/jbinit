#include <Security/Security.h>
#include <substrate.h>

#ifdef DEV_BUILD
bool SecIsInternalRelease(void);

bool (*orig_SecIsInternalRelease)(void);
bool new_SecIsInternalRelease(void) {
    return true;
}

#endif
void securitydInit(void) {
#ifdef DEV_BUILD
    MSHookFunction(SecIsInternalRelease, (void*)new_SecIsInternalRelease, (void**)&orig_SecIsInternalRelease);
#endif
}
