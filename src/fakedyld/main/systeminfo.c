#include <fakedyld/fakedyld.h>


void systeminfo(struct systeminfo* sysinfo_p) {
    size_t bootargs_len = MAX_BOOTARGS_LEN;
    CHECK_ERROR(sys_sysctlbyname("kern.bootargs", sizeof("kern.bootargs"), sysinfo_p->bootargs, &bootargs_len, NULL, 0), "Unable to read boot-args");

    size_t kversion_len = MAX_KVERSION_LEN;
    CHECK_ERROR(sys_sysctlbyname("kern.version", sizeof("kern.version"), sysinfo_p->kversion, &kversion_len, NULL, 0), "Unable to get kernel version");

    char* p = strstr(sysinfo_p->kversion, "Darwin Kernel Version ");
    if (p == NULL) {
        LOG("failed to find \"Darwin Kernel Version \" in kernel version");
        goto kver_fail;
    }
    p += (sizeof("Darwin Kernel Version ")-1);

    if (!isdigit(*p)) goto kver_fail;
    sysinfo_p->osrelease.darwinMajor = atoi(p);
    if (sysinfo_p->osrelease.darwinMajor == 0) {
        LOG("darwinMajor parsing error");
        goto kver_fail;
    }

    p = (strchr(p, '.'));
    if (p == NULL) {
        LOG("darwinMinor parsing error");
        goto kver_fail;
    }
    sysinfo_p->osrelease.darwinMinor = atoi(p+1);

    p += 1;
    p = (strchr(p, '.'));
    if (p == NULL) {
        LOG("darwinRevision parsing error");
        goto kver_fail;
    }
    sysinfo_p->osrelease.darwinRevision = atoi(p+1);

    p += 1;
    char* xnu_p;
    if ((xnu_p = strstr(p, "root:xnu-")) != NULL) {
        p = xnu_p + (sizeof("root:xnu-")-1);
    } else if ((xnu_p = strstr(p, "root:xnu_development-")) != NULL) {
        p = xnu_p + (sizeof("root:xnu_development-")-1);
    } else if ((xnu_p = strstr(p, "root:xnu_debug-")) != NULL) {
        p = xnu_p + (sizeof("root:xnu_debug-")-1);
    } else if ((xnu_p = strstr(p, "root:xnu_kasan-")) != NULL) {
        p = xnu_p + (sizeof("root:xnu_kasan-")-1);
    } else {
        LOG("xnuMajor location failed");
        goto kver_fail;
    }
    
    sysinfo_p->xnuMajor = atoi(p);
    if (sysinfo_p->xnuMajor == 0) {
        LOG("xnuMajor parsing error: %p", p);
        goto kver_fail;
    }
    return;
kver_fail:
    LOG("Error parsing kernel version");
    spin();
    __builtin_unreachable();
}
