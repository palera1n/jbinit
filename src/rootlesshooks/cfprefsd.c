#include <substrate.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <sys/param.h>
#include <CoreFoundation/CoreFoundation.h>
#include <errno.h>

void NSLog(CFStringRef format, ...);

bool preferencePlistNeedsRedirection(char* plistPath) {
	if (
		strncmp(plistPath, "/private/var/mobile/Containers",sizeof("/private/var/mobile/Containers")-1) == 0 ||
		strncmp(plistPath, "/var/db",sizeof("/var/db")-1) == 0 ||
		strncmp(plistPath, "/var/jb",sizeof("/var/jb")-1) == 0
	) return false;
	char plistName[MAXPATHLEN];
	char* ptr = basename_r(plistPath, plistName);
	if (ptr == NULL) {
		NSLog(CFSTR("cfprefsd_hook: basename_r failed: %d (%s)"), errno, strerror(errno));
		abort();
	}
	if (
		strncmp(plistName, "com.apple.", sizeof("com.apple.")-1) == 0 ||
		strncmp(plistName, "systemgroup.com.apple.", sizeof("systemgroup.com.apple.")-1) == 0 ||
		strncmp(plistName, "group.com.apple.", sizeof("group.com.apple.")-1) == 0
	) return false;

	char* additionalSystemPlistNames[] = {
		".GlobalPreferences.plist",
		".GlobalPreferences_m.plist",
		"bluetoothaudiod.plist",
		"NetworkInterfaces.plist",
		"OSThermalStatus.plist",
		"preferences.plist",
		"osanalyticshelper.plist",
		"UserEventAgent.plist",
		"wifid.plist",
		"dprivacyd.plist",
		"silhouette.plist",
		"memoryanalyticsd.plist",
		"nfcd.plist",
		"kNPProgressTrackerDomain.plist",
		"siriknowledged.plist",
		"UITextInputContextIdentifiers.plist",
		"mobile_storage_proxy.plist",
		"splashboardd.plist",
		"mobile_installation_proxy.plist",
		"languageassetd.plist",
		"ptpcamerad.plist",
		"com.google.gmp.measurement.monitor.plist",
		"com.google.gmp.measurement.plist",
		NULL
	};

	for (uint8_t i = 0; additionalSystemPlistNames[i] != NULL; i++) {
		if (strcmp(plistName, additionalSystemPlistNames[i]) == 0) return false;
	}
	return true;
}

bool (*orig_CFPrefsGetPathForTriplet)(CFStringRef, CFStringRef, bool, CFStringRef, char*);
bool new_CFPrefsGetPathForTriplet(CFStringRef bundleIdentifier, CFStringRef user, bool byHost, CFStringRef path, char *buffer) {
	bool orig = orig_CFPrefsGetPathForTriplet(bundleIdentifier, user, byHost, path, buffer);
	if (orig && buffer) {
		bool needsRedirection = preferencePlistNeedsRedirection(buffer);
		if (needsRedirection) {
			NSLog(CFSTR("cfprefsd_hook: Plist redirected to /var/jb: %s"), buffer);
			char newPath[MAXPATHLEN];
			snprintf(newPath, MAXPATHLEN, "/var/jb/%s", buffer);
			snprintf(buffer, MAXPATHLEN, "%s", newPath);
		}
	}
	return orig;
}

void cfprefsdInit(void)
{
	MSImageRef coreFoundationImage = MSGetImageByName("/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation");
	void* CFPrefsGetPathForTriplet_ptr = MSFindSymbol(coreFoundationImage, "__CFPrefsGetPathForTriplet");
	if(CFPrefsGetPathForTriplet_ptr)
	{
		MSHookFunction(CFPrefsGetPathForTriplet_ptr, (void *)&new_CFPrefsGetPathForTriplet, (void **)&orig_CFPrefsGetPathForTriplet);
	}
}
