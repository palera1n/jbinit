#include <Security/Security.h>
#include <substrate.h>

bool SecIsInternalRelease(void);

bool (*orig_SecIsInternalRelease)(void);
bool new_SecIsInternalRelease(void) {
    return true;
}

void securitydInit(void) {
    MSHookFunction(SecIsInternalRelease, (void*)new_SecIsInternalRelease, (void**)&orig_SecIsInternalRelease);
}
