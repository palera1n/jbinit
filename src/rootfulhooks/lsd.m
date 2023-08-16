#include <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <substrate.h>
#include <CoreServices/FSNode.h>
#include <string.h>
#include <CoreFoundation/CFURLPriv.h>

BOOL (*orig__ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy)(FSNode *const node, const NSURLResourceKey key, CFURLResourcePropertyFlags URLflags, CFURLVolumePropertyFlags VolumeFlags);
BOOL new__ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy(FSNode *const node, const NSURLResourceKey key, CFURLResourcePropertyFlags URLflags, CFURLVolumePropertyFlags VolumeFlags) {
	NSLog(@"new__ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy(node=%p, key=%@, URLFlags=0x%llx, CFURLVolumePropertyFlags=0x%llx);\n[node URL].fileSystemRepresentation=%s", node, key, URLflags, VolumeFlags, [node URL].fileSystemRepresentation);
	
	BOOL retval = orig__ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy(node, key, URLflags, VolumeFlags);
	if (
		strncmp([node URL].fileSystemRepresentation, "/Applications", sizeof("/Applications")-1) 
		&& strncmp([node URL].fileSystemRepresentation, "/System", sizeof("/System")-1)
		&& strncmp([node URL].fileSystemRepresentation, "/cores", sizeof("/cores")-1) /* test */
		) {
			return retval;

	}
	if ([key isEqualToString: NSURLVolumeIsLocalKey] ) {
		NSLog(@"key %@ for %s forced YES", key, [node URL].fileSystemRepresentation);
		return YES;

	} else if ([key isEqualToString:NSURLIsVolumeKey] 
	|| [key isEqualToString:NSURLIsMountTriggerKey]
	|| [key isEqualToString:(__bridge NSString*)_kCFURLVolumeIsDiskImageKey]) {
		NSLog(@"key %@ for %s forced NO", key, [node URL].fileSystemRepresentation);
		return NO;
	}

	return retval;
}

void lsdInit(void) {
	NSLog(@"lsdInit...");
    MSImageRef coreServicesImage = MSGetImageByName("/System/Library/Frameworks/CoreServices.framework/CoreServices");

	void* _ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy = MSFindSymbol(coreServicesImage, "__ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy");
	NSLog(@"coreServicesImage=%p, _ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy=%p", coreServicesImage, _ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy);

	if (_ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy)
		MSHookFunction(_ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy, &new__ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy, (void**)&orig__ZL25_FSNodeGetSimpleBoolValueP6FSNodeP8NSStringyy);
}
