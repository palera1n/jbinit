#ifndef SYSTEMHOOK_LIBIOSEXEC_H
#define SYSTEMHOOK_LIBIOSEXEC_H

#include <pwd.h>
#include <grp.h>
#include <stdbool.h>

#ifdef SYSTEMWIDE_IOSEXEC

#define THREE_WAY_CALL(func, ...) ( ie_ ## func && !has_libiosexec ) ? ie_ ## func( __VA_ARGS__ ) : (orig_ ## func ? orig_ ## func( __VA_ARGS__ ) : func( __VA_ARGS__ ))
#define RET_TWC(...) return THREE_WAY_CALL( __VA_ARGS__ )
#define DECLARE_THREE(retval, func, ...) retval (*ie_ ## func)( __VA_ARGS__ ); retval (*orig_ ## func)( __VA_ARGS__ ); retval (func ## _hook)( __VA_ARGS__ ); 

int get_libiosexec(void);
extern void* libiosexec_handle;
extern bool has_libiosexec;

DECLARE_THREE(char*, getusershell, void);
DECLARE_THREE(void, setusershell, void);
DECLARE_THREE(void, endusershell, void);

DECLARE_THREE(struct passwd *, getpwent, void);
DECLARE_THREE(struct passwd*, getpwnam, const char*);
DECLARE_THREE(int, getpwnam_r, const char *name, struct passwd *pw, char *buf, size_t buflen, struct passwd **pwretp);
DECLARE_THREE(struct passwd*, getpwuid, uid_t uid);
DECLARE_THREE(int, getpwuid_r, uid_t uid, struct passwd *pw, char *buf, size_t buflen, struct passwd **pwretp);
DECLARE_THREE(int, setpassent, int stayopen);
DECLARE_THREE(void, setpwent, void);
DECLARE_THREE(void, endpwent, void);
DECLARE_THREE(char*, user_from_uid, uid_t, int);

DECLARE_THREE(struct group*, getgrgid, gid_t gid);
DECLARE_THREE(int, getgrgid_r, gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result);
DECLARE_THREE(struct group*, getgrnam, const char* name);
DECLARE_THREE(int, getgrnam_r, const char *name, struct group *grp, char *buffer, size_t bufsize, struct group **result);
DECLARE_THREE(struct group*, getgrent, void);
DECLARE_THREE(void, setgrent, void);
DECLARE_THREE(void, endgrent, void);
DECLARE_THREE(char*, group_from_gid, gid_t, int);

#endif

#endif
