#include <payload/payload.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <mach-o/loader.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <libjailbreak/libjailbreak.h>
#include <dlfcn.h>

#define RB2_FULLREBOOT (0x8000000000000000llu)
int reboot3(uint64_t flags, ...);
mach_port_t (*SBSSpringBoardServerPort)(void);
static CFRunLoopRef loop;

void sb_launched(CFNotificationCenterRef __unused center, void __unused *observer,
				 CFStringRef __unused name, const void __unused *object, CFDictionaryRef __unused info) {
    CFRunLoopStop(loop);
}

void* force_revert_notif_thread(__unused void* arg) {
    CFUserNotificationRef force_revert_notif = NULL;
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (dict) {
        CFDictionarySetValue(dict, kCFUserNotificationAlertHeaderKey, CFSTR("Reboot required"));
        CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, CFSTR(
            "A reboot is required to complete the force revert.\n\n"
            "If you wish to jailbreak again, you can choose to enter recovery mode instead.\n"
        ));
        CFDictionarySetValue(dict, kCFUserNotificationDefaultButtonTitleKey, CFSTR("Reboot now"));
        CFDictionarySetValue(dict, kCFUserNotificationAlternateButtonTitleKey, CFSTR("Enter recovery mode"));
        CFDictionarySetValue(dict, kCFUserNotificationOtherButtonTitleKey, CFSTR("Reboot later"));
        
        force_revert_notif = CFUserNotificationCreate(kCFAllocatorDefault, 0, 0, NULL, dict);
        CFRelease(dict);
        
        CFOptionFlags cfres;
        CFUserNotificationReceiveResponse(force_revert_notif, 0, &cfres);
        if ((cfres & 0x3) == kCFUserNotificationDefaultResponse) {
            reboot3(RB2_FULLREBOOT);
        } else if ((cfres & 0x3) == kCFUserNotificationAlternateResponse) {
            nvram("auto-boot", "false");
            reboot3(RB2_FULLREBOOT);
        }
        CFRelease(force_revert_notif);
    }
    return NULL;
}

int launchdaemons(uint32_t payload_options, uint64_t pflags) {
    printf("plooshInit launchdaemons...\n");
    int platform = jailbreak_get_platform();
    if (platform == -1) {
        fprintf(stderr, "jailbreak_get_platform(): %d (%s)\n", errno, strerror(errno));
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
    } else if (platform == PLATFORM_TVOS) {
        xpc_object_t xreply = jailbreak_send_jailbreakd_command_with_reply_sync(JBD_CMD_REGISTER_PAYLOAD_PID);
        xpc_release(xreply);
        kill(getpid(), SIGSTOP);
    }

    if (pflags & palerain_option_safemode) {
        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (dict) {
            CFDictionarySetValue(dict, kCFUserNotificationAlertHeaderKey, CFSTR("Entered Safe Mode"));
            if (pflags & palerain_option_failure)
                CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, CFSTR(
                    "palera1n entered safe mode due to a failure\n\n"
                    "palera1n /did not/ cause this problem, rather, it has protected you from it\n\n"
                    "You can exit safe mode by clicking on exit safe mode in the palera1n app\n\n"
                    "If this issue persists, then most likely you have a bad tweak installed, and you should uninstall it from your package manager."
                ));
                else
            CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, CFSTR("palera1n entered safe mode due to a user request"));

            CFUserNotificationRef notif = CFUserNotificationCreate(kCFAllocatorDefault, 0, 0, NULL, dict);
            CFRelease(dict);
            if (notif) CFRelease(notif);
        }
    }
    
    if (access("/usr/bin/uicache", F_OK) == 0)
        runCommand((char*[]){ "/usr/bin/uicache", "-a", NULL });
    else if (access("/var/jb/usr/bin/uicache", F_OK) == 0)
        runCommand((char*[]){ "/var/jb/usr/bin/uicache", "-a", NULL });
    else if (platform != PLATFORM_BRIDGEOS)
        runCommand((char*[]){ "/cores/binpack/usr/bin/uicache", "-a", NULL });

    pthread_t fr_notif_thread = NULL;
    if (pflags & palerain_option_force_revert) {
        pthread_create(&fr_notif_thread, NULL, &force_revert_notif_thread, NULL);
    }

    /* just in case the above commands are bad, we run them last so the user can still get the loader */
    switch (platform) {
        case PLATFORM_IOS:
            runCommand((char*[]){ "/cores/binpack/usr/bin/uicache", "-p", "/cores/binpack/Applications/palera1nLoader.app", NULL });
            break;
        case PLATFORM_TVOS:
            runCommand((char*[]){ "/cores/binpack/usr/bin/uicache", "-p", "/cores/binpack/Applications/palera1nLoaderTV.app", NULL });
        case PLATFORM_BRIDGEOS:
            break;
        default:
            fprintf(stderr, "uicache_loader: unsupported platform\n");
    }
    
    if (pflags & palerain_option_force_revert) {
        pthread_join(fr_notif_thread, NULL);
    }
    printf("plooshInit launchdaemons: Goodbye!\n");
    return 0;
}
