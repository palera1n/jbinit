#include <systemhook/libiosexec.h>
#include <systemhook/common.h>
#include <dyld-interpose.h>

#ifdef SYSTEMWIDE_IOSEXEC
struct passwd * 
getpwuid_hook(uid_t uid) {
    RET_TWC(getpwuid, uid);
}

int
getpwuid_r_hook(uid_t uid, struct passwd *pw, char *buffer, size_t bufsize, struct passwd **result) {
    RET_TWC(getpwuid_r, uid, pw, buffer, bufsize, result);
}

struct passwd *
getpwnam_hook(const char *name) {
	RET_TWC(getpwnam, name);
}

int
getpwnam_r_hook(const char *name, struct passwd *pw, char *buffer, size_t bufsize, struct passwd **result) {
	RET_TWC(getpwnam_r, name, pw, buffer, bufsize, result);
}

struct passwd *
getpwent_hook(void) {
	RET_TWC(getpwent);
}

void
setpwent_hook(void) {
    RET_TWC(setpwent);
}

void
endpwent_hook(void) {
    RET_TWC(endpwent);
}

char*
user_from_uid_hook(uid_t uid, int nouser) {
    RET_TWC(user_from_uid, uid, nouser);
}

int
setpassent_hook(int stayopen) {
    RET_TWC(setpassent, stayopen);
}

DYLD_INTERPOSE(getpwuid_hook, getpwuid);
DYLD_INTERPOSE(getpwuid_r_hook, getpwuid_r);
DYLD_INTERPOSE(getpwnam_hook, getpwnam);
DYLD_INTERPOSE(getpwnam_r_hook, getpwnam_r);
DYLD_INTERPOSE(getpwent_hook, getpwent);
DYLD_INTERPOSE(setpwent_hook, setpwent);
DYLD_INTERPOSE(setpassent_hook ,setpassent);
DYLD_INTERPOSE(endpwent_hook, endpwent);
DYLD_INTERPOSE(user_from_uid_hook, user_from_uid);
#endif
