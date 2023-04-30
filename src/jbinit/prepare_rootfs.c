#include <jbinit.h>
#include <common.h>

void prepare_rootfs(char* dev_rootdev, bool use_fakefs) {
    struct stat statbuf;
    if (stat("/System/Library/Frameworks/IOKit.framework", &statbuf) == 0) {
        return;
    }
    if (!use_fakefs || !checkrain_options_enabled(info.flags, checkrain_option_bind_mount)) return;
    char* real_rootdev = ios15_rootdev;
    if (darwin22) real_rootdev = ios16_rootdev;
    LOG("mounting realfs %s\n", real_rootdev);
    struct apfs_mountarg arg = {
        real_rootdev,
        0, 1 /* "bdevvp failed: open" kernel panic when mount snapshot wat */, 0
    };
    int err = mount("apfs", "/cores/fs/real", MNT_RDONLY, &arg);
    if (err) {
        LOG("cannot mount %s onto %s, err=%d\n", dev_rootdev, "/cores/fs/real", err);
        spin();
    }
    fbi("/usr/standalone/update", "/cores/fs/real/usr/standalone/update");
    fbi("/System/Library/Frameworks", "/cores/fs/real/System/Library/Frameworks");
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

    if (!darwin22) {
        fbi("/System/Library/PrivateFrameworks", "/cores/fs/real/System/Library/PrivateFrameworks");
        fbi("/System/Library/Caches", "/cores/fs/real/System/Library/Caches");
    }
}
