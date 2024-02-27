#include <systemhook/libiosexec.h>
#include <systemhook/common.h>
#include <dlfcn.h>
#include <substrate.h>

#ifdef HAVE_SYSTEMWIDE_IOSEXEC
#define BIND_IOSEXEC_SYMBOL(sym) ie_ ## sym = dlsym(libiosexec_handle, "ie_" #sym)

void* libiosexec_handle;
int get_libiosexec(void) {
    if (has_libiosexec) return -1;
    if (libiosexec_handle) return 0;
    /* on rootful libiosexec will call into us => infinite recursion */
    if (pflags & (palerain_option_force_revert | palerain_option_rootful)) return -1;
    const char* ie_path = JB_ROOT_PATH("/usr/lib/libiosexec.1.dylib");
    if (access(ie_path, F_OK) != 0) return -1;
    libiosexec_handle = dlopen(ie_path, RTLD_NOW);
    if (!libiosexec_handle) return -1;

    BIND_IOSEXEC_SYMBOL(getusershell);
    BIND_IOSEXEC_SYMBOL(setusershell);
    BIND_IOSEXEC_SYMBOL(endusershell);
    BIND_IOSEXEC_SYMBOL(getpwent);
    BIND_IOSEXEC_SYMBOL(getpwnam);
    BIND_IOSEXEC_SYMBOL(getpwnam_r);
    BIND_IOSEXEC_SYMBOL(getpwuid);
    BIND_IOSEXEC_SYMBOL(getpwuid_r);
    BIND_IOSEXEC_SYMBOL(getpwuid_r);
    BIND_IOSEXEC_SYMBOL(setpassent);
    BIND_IOSEXEC_SYMBOL(setpwent);
    BIND_IOSEXEC_SYMBOL(endpwent);
    BIND_IOSEXEC_SYMBOL(user_from_uid);
    BIND_IOSEXEC_SYMBOL(getgrgid);
    BIND_IOSEXEC_SYMBOL(getgrgid_r);
    BIND_IOSEXEC_SYMBOL(getgrnam);
    BIND_IOSEXEC_SYMBOL(getgrnam_r);
    BIND_IOSEXEC_SYMBOL(getgrent);
    BIND_IOSEXEC_SYMBOL(setgrent);
    BIND_IOSEXEC_SYMBOL(endgrent);
    BIND_IOSEXEC_SYMBOL(group_from_gid);
    
    return 0;
}

#define ELLEKIT_PATH "/cores/binpack/usr/lib/libellekit.dylib"
#define INIT_IOSEXEC_HOOK(func) MSHookFunction_p(func, func ## _hook, (void**)&orig_ ## func)

void (*MSHookFunction_p)(void*, void*, void**);
/* this function will be called by launchd */
SHOOK_EXPORT int init_libiosexec_hook_with_ellekit(void) {
    if (get_libiosexec()) return -1;
    void* ellekit_handle = dlopen(ELLEKIT_PATH, RTLD_NOW);
    if (!ellekit_handle) return -1;
    MSHookFunction_p = dlsym(ellekit_handle, "MSHookFunction");
    if (!MSHookFunction_p) return -1;

    INIT_IOSEXEC_HOOK(getusershell);
    INIT_IOSEXEC_HOOK(setusershell);
    INIT_IOSEXEC_HOOK(endusershell);
    INIT_IOSEXEC_HOOK(getpwent);
    INIT_IOSEXEC_HOOK(getpwnam);
    INIT_IOSEXEC_HOOK(getpwnam_r);
    INIT_IOSEXEC_HOOK(getpwuid);
    INIT_IOSEXEC_HOOK(getpwuid_r);
    INIT_IOSEXEC_HOOK(setpassent);
    INIT_IOSEXEC_HOOK(setpwent);
    INIT_IOSEXEC_HOOK(endpwent);
    INIT_IOSEXEC_HOOK(user_from_uid);
    INIT_IOSEXEC_HOOK(getgrgid);
    INIT_IOSEXEC_HOOK(getgrgid_r);
    INIT_IOSEXEC_HOOK(getgrnam);
    INIT_IOSEXEC_HOOK(getgrnam_r);
    INIT_IOSEXEC_HOOK(getgrent);
    INIT_IOSEXEC_HOOK(setgrent);
    INIT_IOSEXEC_HOOK(endgrent);
    INIT_IOSEXEC_HOOK(group_from_gid);
    return 0;
}
#endif
