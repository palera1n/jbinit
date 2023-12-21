#include <payload/payload.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <mach-o/loader.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <dlfcn.h>

mach_port_t (*SBSSpringBoardServerPort)(void);
static CFRunLoopRef loop;

void sb_launched(CFNotificationCenterRef center, void *observer,
				 CFStringRef name, const void *object, CFDictionaryRef info) {
    CFRunLoopStop(loop);
}

int launchdaemons(uint32_t payload_options, uint64_t pflags) {
    printf("plooshInit launchdaemons...\n");
    int platform = get_platform();
    if (platform == -1) {
        fprintf(stderr, "get_platform(): %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    if (platform == PLATFORM_IOS) {
        void *springboardservices = dlopen("/System/Library/PrivateFrameworks/SpringBoardServices.framework/SpringBoardServices", RTLD_NOW);
        if (springboardservices) {
            SBSSpringBoardServerPort = dlsym(springboardservices, "SBSSpringBoardServerPort");
            if (SBSSpringBoardServerPort && !MACH_PORT_VALID(SBSSpringBoardServerPort())) {
                loop = CFRunLoopGetCurrent();
                CFNotificationCenterAddObserver(CFNotificationCenterGetDarwinNotifyCenter(), NULL, &sb_launched, CFSTR("SBSpringBoardDidLaunchNotification"), NULL, 0);
                CFRunLoopRun();
            }
            dlclose(springboardservices);
        } else {
            fprintf(stderr, "failed to dlopen springboardservices\n");
        }
    }

    if (pflags & palerain_option_safemode) {
        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (dict) {
            CFDictionarySetValue(dict, kCFUserNotificationAlertHeaderKey, CFSTR("Entered Safe Mode"));
            CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, CFSTR("palera1n entered safe mode due to a user request"));

            CFUserNotificationRef notif = CFUserNotificationCreate(kCFAllocatorDefault, 0, 0, NULL, dict);
            CFRelease(dict);
            if (notif) CFRelease(notif);
        }
    }

    switch (platform) {
        case PLATFORM_IOS:
            runCommand((char*[]){ "/cores/binpack/usr/bin/uicache", "-p", "/cores/binpack/Applications/palera1nLoader.app", NULL });
            break;
        case PLATFORM_TVOS:
        case PLATFORM_BRIDGEOS:
            break;
        default:
            fprintf(stderr, "uicache_loader: unsupported platform\n");
    }

    /* just in case these commands are bad, we run them last so the user can still get the loader */
    if (access("/usr/bin/uicache", F_OK) == 0)
        runCommand((char*[]){ "/usr/bin/uicache", "-a", NULL });
    else if (access("/var/jb/usr/bin/uicache", F_OK) == 0)
        runCommand((char*[]){ "/var/jb/usr/bin/uicache", "-a", NULL });


    printf("plooshInit launchdaemons: Goodbye!\n");
    return 0;
}
