#include <payload/payload.h>
#include <libjailbreak/libjailbreak.h>
#include <removefile.h>
#include <sys/mount.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <mount_args.h>
#include <alloca.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/kern_memorystatus.h>

#define SB_PREF_PLIST_PATH "/var/mobile/Library/Preferences/com.apple.springboard.plist"
#define CF_STRING_GET_CSTRING_PTR(cfStr, cPtr) do { \
    cPtr = (char*)CFStringGetCStringPtr(cfStr, kCFStringEncodingUTF8); \
        if (!cPtr) { \
            CFIndex length = CFStringGetLength(cfStr) + 1; \
            cPtr = alloca(length); \
            Boolean CStringStatus = CFStringGetCString(cfStr, cPtr, length, kCFStringEncodingUTF8); \
            if (!CStringStatus) { \
                cPtr = "Internal Error: failed to get error description";\
            }\
        } \
    \
} while(0)

int enable_non_default_system_apps(void) {
    CFURLRef fileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR(SB_PREF_PLIST_PATH), kCFURLPOSIXPathStyle, false);
    CFReadStreamRef ReadStream = CFReadStreamCreateWithFile(kCFAllocatorDefault, fileURL);
    if (!ReadStream) {
        fprintf(stderr, "CFReadStreamCreateWithFile for %s failed\n", SB_PREF_PLIST_PATH);
        CFRelease(fileURL);
        return -1;
    }
    Boolean ReadStreamStatus = CFReadStreamOpen(ReadStream);
    if (!ReadStreamStatus) {
        fprintf(stderr, "CFReadStreamOpen failed\n");
        CFRelease(ReadStream);
        CFRelease(fileURL);
        return -1;
    }
    CFPropertyListFormat format;
    CFErrorRef CFPropertyListCreateWithStreamError;
    CFPropertyListRef plist = CFPropertyListCreateWithStream(kCFAllocatorDefault, ReadStream, 0, kCFPropertyListMutableContainersAndLeaves, &format, &CFPropertyListCreateWithStreamError);
    CFReadStreamClose(ReadStream);
    CFRelease(ReadStream);
    if (!plist) {
        CFIndex code = CFErrorGetCode(CFPropertyListCreateWithStreamError);
        char* errStr; CF_STRING_GET_CSTRING_PTR(CFErrorCopyDescription(CFPropertyListCreateWithStreamError), errStr);
        fprintf(stderr, "CFPropertyListCreateWithStream failed: %ld (%s)\n", (long)code, errStr);
        CFRelease(CFPropertyListCreateWithStreamError);
        CFRelease(fileURL);
        return -1;
    }
    CFTypeID type = CFGetTypeID(plist);
    if (type != CFDictionaryGetTypeID()) {
        CFStringRef typeCFString = CFCopyTypeIDDescription(type); 
        char* typeDesc; CF_STRING_GET_CSTRING_PTR(typeCFString, typeDesc);
        char* dictDesc; CF_STRING_GET_CSTRING_PTR(CFCopyTypeIDDescription(CFDictionaryGetTypeID()), dictDesc);
        fprintf(stderr, "Expecting %s to be of type %s, got a %s instead\n", SB_PREF_PLIST_PATH, dictDesc, typeDesc);
        CFRelease(plist);
        CFRelease(fileURL);
        return -1;
    }
    CFDictionarySetValue((CFMutableDictionaryRef)plist, CFSTR("SBShowNonDefaultSystemApps"), kCFBooleanTrue);
    CFWriteStreamRef WriteStream = CFWriteStreamCreateWithFile(kCFAllocatorDefault, fileURL);
    CFRelease(fileURL);
    if (!WriteStream) {
        fprintf(stderr, "CFWriteStreamCreateWithFile for %s failed\n", SB_PREF_PLIST_PATH);
        CFRelease(plist);
        return -1;
    }
    Boolean WriteStreamStatus = CFWriteStreamOpen(WriteStream);
    if (!WriteStreamStatus) {
        fprintf(stderr, "CFWriteStreamOpen failed\n");
        CFRelease(WriteStream);
        CFRelease(plist);
        return -1;
    }
    CFErrorRef CFPropertyListWriteError;
    CFIndex written = CFPropertyListWrite(plist, WriteStream, format, 0, &CFPropertyListWriteError);
    CFRelease(plist);
    CFWriteStreamClose(WriteStream);
    CFRelease(WriteStream);
    if (!written) {
        CFIndex code = CFErrorGetCode(CFPropertyListWriteError);
        char* errStr; CF_STRING_GET_CSTRING_PTR(CFErrorCopyDescription(CFPropertyListWriteError), errStr);
        fprintf(stderr, "CFPropertyListWrite failed: %ld (%s)\n", (long)code, errStr);
        CFRelease(CFPropertyListWriteError);
        return -1;
    }
    printf("Wrote %ld bytes to %s\n", (long)written, SB_PREF_PLIST_PATH);
    return 0;

}

int remove_jailbreak_files(uint64_t pflags) {
    removefile_state_t state = removefile_state_alloc();
    if (pflags & palerain_option_rootful) {
        printf("delete /var/lib\n");
        removefile("/var/lib", state, REMOVEFILE_RECURSIVE);
        printf("delete /var/cache\n");
        removefile("/var/cache", state, REMOVEFILE_RECURSIVE);
    } else {
        char bmhash[97], prebootPath[150], jbPaths[150];
        int ret = jailbreak_get_bmhash(bmhash);
        if (ret) {
            fprintf(stderr, "get bmhash failed\n");
            return -1;
        }
        snprintf(prebootPath, 150, "/private/preboot/%s", bmhash);
        DIR* dir = opendir(prebootPath);
        if (!dir) return 0;
        struct dirent* d;
        while ((d = readdir(dir)) != NULL) {
            if (strncmp(d->d_name, "jb-", 3)) continue;
            snprintf(jbPaths, 150, "%s/%s", prebootPath, d->d_name);
            if ((strcmp(jbPaths, prebootPath) == 0)) {
                fprintf(stderr, "disaster averted\n");
                return -1;
            }
            printf("delete %s\n", jbPaths);
            removefile(jbPaths, state, REMOVEFILE_RECURSIVE);
        }
        printf("delete /var/jb\n");
        removefile("/var/jb", state, REMOVEFILE_RECURSIVE);
    }
    removefile_state_free(state);
    return 0;
}

int fixup_databases(void);
int sysstatuscheck(uint32_t payload_options, uint64_t pflags) {
    printf("plooshInit sysstatuscheck...\n");
    int retval;
    memorystatus_memlimit_properties2_t mmprops;
    retval = memorystatus_control(MEMORYSTATUS_CMD_GET_MEMLIMIT_PROPERTIES, 1, 0, &mmprops, sizeof(mmprops));
    if (!retval) {
        printf("memory limit for launchd: %d MiB\n", mmprops.v1.memlimit_active);
    }

    remount();
    enable_non_default_system_apps();
    if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0) {
        printf("generating ssh host key...\n");
        runCommand((char*[]){ "/cores/binpack/usr/bin/dropbearkey", "-f",  "/private/var/dropbear_rsa_host_key", "-t", "rsa", "-s", "4096", NULL });
    }
    if ((pflags & palerain_option_force_revert)) remove_jailbreak_files(pflags);
    if (pflags & palerain_option_rootful) {
        remove_bogus_var_jb();
        unlink("/var/jb");
    } else {
        remove_bogus_var_jb();
        create_var_jb();
#ifdef HAVE_SYSTEMWIDE_IOSEXEC
        if (access("/var/jb", F_OK) == 0) {
            fixup_databases();
        }
#endif
    }
    if (
        (pflags & palerain_option_safemode) == 0 &&
        (pflags & palerain_option_force_revert) == 0
        ) {
            load_etc_rc_d(pflags);
    }

    printf("plooshInit sysstatuscheck: bye\n");
    return execv("/usr/libexec/sysstatuscheck", (char*[]){ "/usr/libexec/sysstatuscheck", NULL });
}
