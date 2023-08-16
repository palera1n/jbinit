#include <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <substrate.h>
#include <CoreServices/FSNode.h>
#include <string.h>
#include <CoreFoundation/CFURLPriv.h>

NSURL* (*orig_LSGetInboxURLForBundleIdentifier)(NSString* bundleIdentifier)=NULL;
NSURL* new_LSGetInboxURLForBundleIdentifier(NSString* bundleIdentifier)
{
	NSURL* pathURL = orig_LSGetInboxURLForBundleIdentifier(bundleIdentifier);

	if( ![bundleIdentifier hasPrefix:@"com.apple."] 
			&& [pathURL.path hasPrefix:@"/var/mobile/Library/Application Support/Containers/"])
	{
		NSLog(@"redirect Inbox %@: %@", bundleIdentifier, pathURL);
		pathURL = [NSURL fileURLWithPath:[NSString stringWithFormat:@"/var/jb/%@", pathURL.path]];
	}

	return pathURL;
}

void lsdInit(void) {
	NSLog(@"lsdInit...");
    MSImageRef coreServicesImage = MSGetImageByName("/System/Library/Frameworks/CoreServices.framework/CoreServices");
    
    void* _LSGetInboxURLForBundleIdentifier = MSFindSymbol(coreServicesImage, "__LSGetInboxURLForBundleIdentifier");
	NSLog(@"coreServicesImage=%p, _LSGetInboxURLForBundleIdentifier=%p", coreServicesImage, _LSGetInboxURLForBundleIdentifier);
	if(_LSGetInboxURLForBundleIdentifier)
		MSHookFunction(_LSGetInboxURLForBundleIdentifier, (void *)&new_LSGetInboxURLForBundleIdentifier, (void **)&orig_LSGetInboxURLForBundleIdentifier);
}

