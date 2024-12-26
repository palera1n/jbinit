//
//  BlueTool.c
//  plooshInit
//
//  Created by Nick Chan on 2024/12/24.
//

#include <substrate.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>

CFTypeRef MGCopyAnswer(CFStringRef);
CFTypeRef (*MGCopyAnswer_orig)(CFStringRef);

CFTypeRef MGCopyAnswer_hook(CFStringRef k) {
    if (CFStringCompare(k, CFSTR("ProductHash"), 0) == 0) {
        UInt8 data[] = {0x4d,0x01,0x3e,0x8f,0xae,0x97,0x6e,0x61,0xa7,0xb3,0xb6,0x9c,0xdb,0x5a,0x01,0x5f,0xa7,0xff,0x04,0xbb};
        CFDataRef r = CFDataCreate(kCFAllocatorDefault, data, sizeof(data));
        return r;
    }
    // required to log producthash from ipad 7
#if 0
    else if (CFStringCompare(k, CFSTR("ReleaseType"), 0) == 0) {
        return CFStringCreateWithCString(kCFAllocatorDefault, "Internal", kCFStringEncodingUTF8);
    }
#endif

    return MGCopyAnswer_orig(k);
}

#if 0
bool SecIsInternalRelease(void);
bool (*orig_SecIsInternalRelease)(void);
bool new_SecIsInternalRelease(void) {
    return true;
}
#endif


void bluetoolInit(void) {
    CFStringRef productType = MGCopyAnswer(CFSTR("ProductType"));
    if (CFStringCompare(productType, CFSTR("iPad7,5"), 0)
        && CFStringCompare(productType, CFSTR("iPad7,6"), 0))
        return;

    MSHookFunction(MGCopyAnswer, (void*)MGCopyAnswer_hook, (void**)&MGCopyAnswer_orig);
#if 0
    MSHookFunction(SecIsInternalRelease, (void*)new_SecIsInternalRelease, (void**)&orig_SecIsInternalRelease);
#endif
}
