#include <fakedyld/fakedyld.h>
#include <mount_args.h>

const char* volume_prefix(void) {
    static char prefix[32] = {'\0'};
    if (prefix[0] != '\0') return prefix;
    struct statfs64 rootfs_st;
    if (statfs64("/", &rootfs_st)) {
        panic("statfs64(/) failed");
    }
    if (strcmp(rootfs_st.f_fstypename, "apfs")) {
        panic("unexpected filesystem type of /");
    }

    char* pBSDName;
    if ((pBSDName = strstr(rootfs_st.f_mntfromname, "@/dev/"))) {
        pBSDName = &pBSDName[6];
    } else {
        pBSDName = rootfs_st.f_mntfromname;
    }

    char* suffix = pBSDName;
    for (size_t i = 0; pBSDName[i] != '\0'; i++) {
        if (pBSDName[i] == 's') {
            suffix = &pBSDName[i+1];
        }
    }
    suffix[0] = '\0';
    snprintf(prefix, 32, "%s", pBSDName);
    return prefix;
}

void prepare_rootfs(struct systeminfo* sysinfo_p, struct paleinfo* pinfo_p, int platform) {
    int ret;
    struct stat64 statbuf;
    if (stat64("/System/Library/Frameworks/IOKit.framework", &statbuf) == 0) {
        return;
    }
    if (!(pinfo_p->flags & palerain_option_ssv)) return;
    char real_rootdev[32];
    snprintf(real_rootdev, 32, "%s1", volume_prefix());

    LOG("mounting realfs %s\n", real_rootdev);
    struct apfs_mount_args arg = {
        real_rootdev,
        MNT_RDONLY, APFS_MOUNT_FILESYSTEM /* "bdevvp failed: open" kernel panic when mount snapshot */
        , 0, 0, { "" }, NULL, 0, 0, NULL, 0, 0, 0, 0, 0, 0
    };
    ret = mount("apfs", "/cores/fs/real", MNT_RDONLY, &arg);
    if (ret) {
        panic("cannot mount %s onto %s, ret=%d", real_rootdev, "/cores/fs/real", errno);
    }

    fbi("/usr/standalone/update", "/cores/fs/real/usr/standalone/update");
    if (platform == PLATFORM_IOS) {
        fbi("/System/Library/AccessibilityBundles", "/cores/fs/real/System/Library/AccessibilityBundles");
        fbi("/System/Library/Assistant", "/cores/fs/real/System/Library/Assistant");
        fbi("/System/Library/Audio", "/cores/fs/real/System/Library/Audio");
        fbi("/System/Library/Fonts", "/cores/fs/real/System/Library/Fonts");
        fbi("/System/Library/Frameworks", "/cores/fs/real/System/Library/Frameworks");
        fbi("/System/Library/Health", "/cores/fs/real/System/Library/Health");
        fbi("/System/Library/LinguisticData", "/cores/fs/real/System/Library/LinguisticData");
        fbi("/System/Library/OnBoardingBundles", "/cores/fs/real/System/Library/OnBoardingBundles");
        fbi("/System/Library/Photos", "/cores/fs/real/System/Library/Photos");
        fbi("/System/Library/PreferenceBundles", "/cores/fs/real/System/Library/PreferenceBundles");
        fbi("/System/Library/PreinstalledAssetsV2", "/cores/fs/real/System/Library/PreinstalledAssetsV2");

        if (sysinfo_p->osrelease.darwinMajor < 22) {
            fbi("/System/Library/PrivateFrameworks", "/cores/fs/real/System/Library/PrivateFrameworks");
            fbi("/System/Library/Caches", "/cores/fs/real/System/Library/Caches");
        }
    } else {
        fbi("/System", "/cores/fs/real/System");
    }
}
