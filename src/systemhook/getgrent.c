#include <systemhook/libiosexec.h>
#include <systemhook/common.h>
#include <dyld-interpose.h>

#ifdef HAVE_SYSTEMWIDE_IOSEXEC
struct group * 
getgrgid_hook(gid_t gid) {
    RET_TWC(getgrgid, gid);
}

int
getgrgid_r_hook(gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result) {
    RET_TWC(getgrgid_r, gid, grp, buffer, bufsize, result);
}

struct group *
getgrnam_hook(const char *name) {
	RET_TWC(getgrnam, name);
}

int
getgrnam_r_hook(const char *name, struct group *grp, char *buffer, size_t bufsize, struct group **result) {
	RET_TWC(getgrnam_r, name, grp, buffer, bufsize, result);
}

struct group *
getgrent_hook(void) {
	RET_TWC(getgrent);
}

void
setgrent_hook(void) {
    RET_TWC(setgrent);
}

void
endgrent_hook(void) {
    RET_TWC(endgrent);
}

char*
group_from_gid_hook(gid_t gid, int nouser) {
    RET_TWC(group_from_gid, gid, nouser);
}

DYLD_INTERPOSE(getgrgid_hook, getgrgid);
DYLD_INTERPOSE(getgrgid_r_hook, getgrgid_r);
DYLD_INTERPOSE(getgrnam_hook, getgrnam);
DYLD_INTERPOSE(getgrnam_r_hook, getgrnam_r);
DYLD_INTERPOSE(getgrent_hook, getgrent);
DYLD_INTERPOSE(setgrent_hook, setgrent);
DYLD_INTERPOSE(endgrent_hook, endgrent);
DYLD_INTERPOSE(group_from_gid_hook, group_from_gid);
#endif
