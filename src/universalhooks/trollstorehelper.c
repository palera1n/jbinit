#include <substrate.h>
int apply_coretrust_bypass_hook(__unused char* binaryPath) {
    return 0;
}

void trollstorehelperInit(char* executablePath) {
    MSImageRef trollstorehelper = MSGetImageByName(executablePath);
    void* apply_coretrust_bypass = MSFindSymbol(trollstorehelper, "_apply_coretrust_bypass");
    if (apply_coretrust_bypass) {
        MSHookFunction(apply_coretrust_bypass, apply_coretrust_bypass_hook, NULL);
    }
}
