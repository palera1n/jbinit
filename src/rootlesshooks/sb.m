#include <Foundation/Foundation.h>
#include <substrate.h>
#include <objc/objc.h>

@interface XBSnapshotContainerIdentity : NSObject <NSCopying> 
@property (nonatomic, readonly, copy) NSString* bundleIdentifier;
@end

@class XBSnapshotContainerIdentity;
static NSString * (*orig_XBSnapshotContainerIdentity_snapshotContainerPath)(XBSnapshotContainerIdentity*, SEL);

static NSString * XBSnapshotContainer_Identity_snapshotContainerPath(XBSnapshotContainerIdentity* self, SEL _cmd) {
    NSString* path = orig_XBSnapshotContainerIdentity_snapshotContainerPath(self, _cmd);

    if([path hasPrefix:@"/var/mobile/Library/SplashBoard/Snapshots/"] && ![self.bundleIdentifier hasPrefix:@"com.apple."]) {
        NSLog(@"redirect snapshotContainerPath %@: %@", self.bundleIdentifier, path);
        path = [NSString stringWithFormat:@"/var/jb/%@", path];
    }

    return path;
}

void sbInit(void)
{
    Class class_XBSnapshotContainerIdentity = objc_getClass("XBSnapshotContainerIdentity"); 
    MSHookMessageEx(
        class_XBSnapshotContainerIdentity, 
        @selector(snapshotContainerPath), 
        (IMP)&XBSnapshotContainer_Identity_snapshotContainerPath,
        (IMP*)&orig_XBSnapshotContainerIdentity_snapshotContainerPath
    );
}
