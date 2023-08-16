#include <fakedyld/fakedyld.h>
#include <mount_args.h>

void prepare_rootfs(struct systeminfo* sysinfo_p, struct paleinfo* pinfo_p) {
    int ret;
#if 0
    if ((ret = mount("devfs", "/dev", 0, "devfs"))) {
        LOG("mount devfs failed: %d", errno);
        spin();
    }
#endif
    struct stat64 statbuf;
    if (stat64("/System/Library/Frameworks/IOKit.framework", &statbuf) == 0) {
        return;
    }
    if (!(pinfo_p->flags & palerain_option_ssv)) return;
    char real_rootdev[32];
    if (sysinfo_p->osrelease.darwinMajor > 21) {
        snprintf(real_rootdev, 32, "/dev/" DARWIN22_ROOTDEV);
    } else {
        snprintf(real_rootdev, 32, "/dev/" DARWIN21_ROOTDEV); 
    }

    LOG("mounting realfs %s\n", real_rootdev);
    struct apfs_mount_args arg = {
        real_rootdev,
        MNT_RDONLY, APFS_MOUNT_FILESYSTEM /* "bdevvp failed: open" kernel panic when mount snapshot */, 0
    };
    ret = mount("apfs", "/cores/fs/real", MNT_RDONLY, &arg);
    if (ret) {
        LOG("cannot mount %s onto %s, ret=%d\n", real_rootdev, "/cores/fs/real", errno);
        spin();
    }
    fbi("/usr/standalone/update", "/cores/fs/real/usr/standalone/update");
#ifdef BOOTLOOP_ME
    fbi("/System", "/cores/fs/real/System");
#else
    fbi("/System/Library/AccessibilityBundles", "/cores/fs/real/System/Library/AccessibilityBundles");
    fbi("/System/Library/Assistant", "/cores/fs/real/System/Library/Assistant");
    fbi("/System/Library/Audio", "/cores/fs/real/System/Library/Audio");
    fbi("/System/Library/Fonts", "/cores/fs/real/System/Library/Fonts");
    fbi("/System/Library/Health", "/cores/fs/real/System/Library/Health");
    fbi("/System/Library/LinguisticData", "/cores/fs/real/System/Library/LinguisticData");
    fbi("/System/Library/OnBoardingBundles", "/cores/fs/real/System/Library/OnBoardingBundles");
    fbi("/System/Library/Photos", "/cores/fs/real/System/Library/Photos");
    fbi("/System/Library/PreferenceBundles", "/cores/fs/real/System/Library/PreferenceBundles");
    fbi("/System/Library/PreinstalledAssetsV2", "/cores/fs/real/System/Library/PreinstalledAssetsV2");

    if (sysinfo_p->osrelease.darwinMajor > 21) {
        fbi("/System/Library/PrivateFrameworks", "/cores/fs/real/System/Library/PrivateFrameworks");
        fbi("/System/Library/Caches", "/cores/fs/real/System/Library/Caches");
    }
#endif
}
