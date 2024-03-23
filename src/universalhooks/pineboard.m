#include <Foundation/Foundation.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <substrate.h>

void* PBAppInfo_isEnabled_orig;
void* PBAppState_isEnabledForApplicationWithIdentifier_orig;
void* PBSMutableAppState_isEnabled_orig;
bool returns_true(void) {
    return true;
}

void pineboardInit(void) {
    Class PBAppInfo = objc_getClass("PBAppInfo");
    if (PBAppInfo) {
        MSHookMessageEx(PBAppInfo, @selector(isEnabled), (IMP)&returns_true, (IMP*)&PBAppInfo_isEnabled_orig);
    }
    
    Class PBAppState = objc_getClass("PBAppState");
    if (PBAppState) {
        MSHookMessageEx(PBAppState, @selector(isEnabledForApplicationWithIdentifier), (IMP)&returns_true, (IMP*)&PBAppState_isEnabledForApplicationWithIdentifier_orig);

    }
    
    Class PBSMutableAppState = objc_getClass("PBSMutableAppState");
    if (PBSMutableAppState) {
        MSHookMessageEx(PBSMutableAppState, @selector(isEnabled), (IMP)&returns_true, (IMP*)&PBSMutableAppState_isEnabled_orig);
    }
}
