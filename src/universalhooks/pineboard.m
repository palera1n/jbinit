#include <Foundation/Foundation.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <substrate.h>

@interface LSApplicationWorkspace: NSObject
- (id)allApplications;
- (void)openApplicationWithBundleID:(NSString *)string;
- (id)defaultWorkspace;
@end

@interface NSDistributedNotificationCenter : NSNotificationCenter

+ (id)defaultCenter;

- (void)addObserver:(id)arg1 selector:(SEL)arg2 name:(id)arg3 object:(id)arg4;
- (void)postNotificationName:(id)arg1 object:(id)arg2 userInfo:(id)arg3;

@end

@interface PBSSystemService : NSObject
+(id)sharedInstance;
-(void)deactivateScreenSaver;
@end

@interface PBSPowerManager : NSObject

+(id)sharedInstance;
+(void)load;
+(void)setupPowerManagement;
-(void)_performUserEventWakeDevice;
-(void)wakeDeviceWithOptions:(id)arg1;
-(void)setNeedsDisplayWakeOnPowerOn:(BOOL)arg1;
- (void)sleepDeviceWithOptions:(id)arg1;
-(void)_registerForPowerNotifications;
-(void)_registerForThermalNotifications;
-(void)_enableIdleSleepAndWatchdog;
-(void)_registerForBackBoardNotifications;
-(void)_updateIdleTimer;
@end

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
