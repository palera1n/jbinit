#include <substrate.h>
#include <limits.h>
#include <stdint.h>

int apply_coretrust_bypass_hook(__unused char* binaryPath) {
    return 0;
}

void trollstorehelperInit(char* executablePath) {
    MSImageRef trollstorehelper = MSGetImageByName(executablePath);
    void* apply_coretrust_bypass = MSFindSymbol(trollstorehelper, "_apply_coretrust_bypass");
    bool (*isPlatformVulnerableToExploitType)(uint32_t) = MSFindSymbol(trollstorehelper, "_isPlatformVulnerableToExploitType");

    if (isPlatformVulnerableToExploitType && apply_coretrust_bypass) {
        if (!isPlatformVulnerableToExploitType(UINT32_MAX)) {
            MSHookFunction(apply_coretrust_bypass, apply_coretrust_bypass_hook, NULL);
        }
    }
}
