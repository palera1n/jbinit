#include <payload/payload.h>
#include <paleinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/types.h>
#include <limits.h>
#include <spawn.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/snapshot.h>
#include <APFS/APFS.h>
#include <copyfile.h>
#include <mount_args.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <fcntl.h>

#define MINIMUM_EXTRA_SPACE 256 * 1024 * 1024
#define kIONVRAMForceSyncNowPropertyKey "IONVRAM-FORCESYNCNOW-PROPERTY"

struct cb_context {
    struct paleinfo* pinfo_p;
    uint64_t bytesToCopy;
};

void notch_clear(char* machine) {
    printf("\033[H\033[2J");
    if (!strcmp(machine, "iPhone10,3") || !strcmp(machine, "iPhone10,6")) {
        printf("\n\n\n\n\n\n");
    }
}

int copyfile_fakefs_cb(int what, int __unused stage, copyfile_state_t __unused state, const char * src, const char * __unused dst, void * ctx) {
    char basename_buf[PATH_MAX];
    struct paleinfo* pinfo_p = ((struct cb_context*)ctx)->pinfo_p;
    switch (what) {
        case COPYFILE_PROGRESS:
        case COPYFILE_RECURSE_ERROR:
        case COPYFILE_RECURSE_DIR_CLEANUP:
            break;
        case COPYFILE_RECURSE_FILE:
        case COPYFILE_RECURSE_DIR:
            if (!strcmp(basename_r(src, basename_buf), ".fseventsd")) return COPYFILE_SKIP;
            if (pinfo_p->flags & palerain_option_setup_partial_root) {
#ifdef BOOTLOOP_ME
                if (
                    strcmp(src, "/cores/fs/real/./System") == 0
                ) {
                    if (access(src, F_OK) != 0) CHECK_ERROR(mkdir(src, 0755), 1, "bindfs mkdir failed");
                    printf("skip %s\n", src);
                    return COPYFILE_SKIP;
                }
#else
                if (
                    strcmp(src, "/cores/fs/real/./System/Library/Frameworks") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/AccessibilityBundles") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/Assistant") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/Audio") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/Fonts") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/Health") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/LinguisticData") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/OnBoardingBundles") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/Photos") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/PreferenceBundles") == 0 ||
                    strcmp(src, "/cores/fs/real/./System/Library/PreinstalledAssetsV2") == 0
                ) {
                    if (access(src, F_OK) != 0) CHECK_ERROR(mkdir(src, 0755), 1, "bindfs mkdir failed");
                    printf("skip %s\n", src);
                    return COPYFILE_SKIP;
                }

                if ((
                      strcmp(src, "/cores/fs/real/./System/Library/PrivateFrameworks") == 0 ||
                      strcmp(src, "/cores/fs/real/./System/Library/Caches") == 0
                    )
                    && strncmp(pinfo_p->rootdev, "disk0s1s", 8) == 0)
                 {
                    if (access(src, F_OK) != 0) CHECK_ERROR(mkdir(src, 0755), 1, "bindfs mkdir failed");
                    printf("skip %s\n", src);
                    return COPYFILE_SKIP;

                }
#endif
            }
            break;
    }
    return COPYFILE_CONTINUE;

}

int setup_fakefs(uint32_t __unused payload_options, struct paleinfo* pinfo_p) {
    CHECK_ERROR(runCommand((char*[]){ "/sbin/fsck", "-qL", NULL }), 1, "fsck failed");

    struct statfs rootfs_st;
    CHECK_ERROR(statfs("/", &rootfs_st), 1, "statfs / failed");
    if (strcmp(rootfs_st.f_fstypename, "apfs")) {
        fprintf(stderr, "unexpected filesystem type of /\n");
        spin();
    }

    char fakefs_mntfromname[50];
    snprintf(fakefs_mntfromname, 50, "/dev/%s", pinfo_p->rootdev);

    if (access(fakefs_mntfromname, F_OK) == 0) {
        fprintf(stderr, "fakefs already exists\n");
        spin();
    }

    struct cb_context context = { .pinfo_p = pinfo_p, .bytesToCopy = 0 };

    if ((pinfo_p->flags & palerain_option_setup_partial_root) == 0) {
        struct {
		    uint32_t size;
		    uint64_t spaceused;
	    } __attribute__((aligned(4), packed)) attrbuf = {0};

        struct attrlist attrs = { .bitmapcount = ATTR_BIT_MAP_COUNT, .volattr = ATTR_VOL_INFO | ATTR_VOL_SPACEUSED };
        CHECK_ERROR(getattrlist(rootfs_st.f_mntonname, &attrs, &attrbuf, sizeof(attrbuf), 0), 1,"getattrlist(/) failed");

        context.bytesToCopy = attrbuf.spaceused;
        if ((attrbuf.spaceused + MINIMUM_EXTRA_SPACE) > (rootfs_st.f_bavail * rootfs_st.f_bsize)) {
            fprintf(stderr, "Not enough space! need %lld bytes (%d bytes buffer), have %lld bytes.\n", (attrbuf.spaceused + MINIMUM_EXTRA_SPACE), MINIMUM_EXTRA_SPACE, (rootfs_st.f_bavail * rootfs_st.f_bsize));
            spin();
        }
    }

    CHECK_ERROR(mount("bindfs", "/cores/fs/real", MNT_RDONLY, "/"), 1, "mount_bindfs(/ -> /cores/fs/real) failed");

    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    int recoveryNumber = APFS_VOL_ROLE_RECOVERY;
    CFNumberRef volumeRole = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &recoveryNumber);
    CFDictionaryAddValue(dict, kAPFSVolumeRoleKey, volumeRole);
    CFDictionaryAddValue(dict, kAPFSVolumeNameKey, CFSTR("Xystem"));
    CFDictionaryAddValue(dict, kAPFSVolumeCaseSensitiveKey, kCFBooleanTrue);

    const char* container = "disk0s1";
    if (strncmp(pinfo_p->rootdev, "disk1s", 6) == 0) {
        container = "disk1";
    }

    printf("container=%s\n", container);

    int retval = APFSVolumeCreate(container, dict);
    if (retval) {
        fprintf(stderr, "APFSVolumeCreate failed: %d: %s\n", retval, mach_error_string(retval));
        spin();
    }

    char actual_fakefs_mntfromname[50];
    int32_t fsindex;
    CFNumberGetValue(CFDictionaryGetValue(dict, kAPFSVolumeFSIndexKey), kCFNumberSInt32Type, &fsindex);
    CFRelease(volumeRole);
    CFRelease(dict);

    if (strstr(rootfs_st.f_mntfromname, "/dev/disk0s1s") != NULL) {
        snprintf(actual_fakefs_mntfromname, 50, "/dev/disk0s1s%d", fsindex+1);
    } else if (strstr(rootfs_st.f_mntfromname, "/dev/disk1s") != NULL) {
        snprintf(actual_fakefs_mntfromname, 50, "/dev/disk1s%d", fsindex+1);
    } else {
        fprintf(stderr, "unexpected rootfs f_mntfromname %s\n", rootfs_st.f_mntfromname);
        spin();
    }
    if (strcmp(actual_fakefs_mntfromname, fakefs_mntfromname)) {
        fprintf(stderr, "unexpected fakefs name %s (expected %s)\n", actual_fakefs_mntfromname, fakefs_mntfromname);
        spin();
    }
    sleep(2);
    struct apfs_mount_args args = {
        fakefs_mntfromname, 0, APFS_MOUNT_FILESYSTEM, 0, 0, { "" }, NULL, 0, 0, NULL, 0, 0, 0, 0, 0, 0 };
    CHECK_ERROR(mount("apfs", "/cores/fs/fake", 0, &args), 1, "mount fakefs failed");

    struct utsname name;
    uname(&name);

    notch_clear(name.machine);
    printf(
    "=========================================================\n"
    "\n"
    "\n"
    "** COPYING FILES TO FAKEFS (MAY TAKE UP TO 10 MINUTES) **\n"
    "\n"
    "\n"
    "=========================================================\n"
        );

    copyfile_state_t state = copyfile_state_alloc();
    copyfile_state_set(state, COPYFILE_STATE_STATUS_CTX, &context);
    copyfile_state_set(state, COPYFILE_STATE_STATUS_CB, &copyfile_fakefs_cb);

    CHECK_ERROR(copyfile("/cores/fs/real/.", "/cores/fs/fake", state, COPYFILE_ALL | COPYFILE_RECURSIVE | COPYFILE_NOFOLLOW_SRC | COPYFILE_NOFOLLOW_DST | COPYFILE_DATA_SPARSE | COPYFILE_DATA), 1, "copyfile() failed");
    printf("done copying files to fakefs\n");
    copyfile_state_free(state);

    int fd_fakefs = open("/cores/fs/fake", O_RDONLY | O_DIRECTORY);
    if (fd_fakefs == -1) {
        fprintf(stderr, "cannot open fakefs fd\n");
        spin();
    }

    CHECK_ERROR(fs_snapshot_create(fd_fakefs, "orig-fs", 0), 1, "cannot create orig-fs snapshot on fakefs");
    close(fd_fakefs);
    sync();
    sleep(2);

    notch_clear(name.machine);
    printf(
    "=========================================================\n"
    "\n"
    "\n"
    "** FakeFS is finished! **\n"
    "\n"
    "\n"
    "=========================================================\n"
        );

    unmount("/cores/fs/real", MNT_FORCE);
    unmount("/cores/fs/fake", MNT_FORCE);

    if (access("/sbin/umount", F_OK) == 0)
        runCommand((char*[]){ "/sbin/umount", "-a", NULL });

    io_registry_entry_t nvram = IORegistryEntryFromPath(kIOMasterPortDefault, kIODeviceTreePlane ":/options");
    kern_return_t ret = IORegistryEntrySetCFProperty(nvram, CFSTR("auto-boot"), CFSTR("false"));
    printf("set nvram auto-boot=false ret: %d\n",ret);
    ret = IORegistryEntrySetCFProperty(nvram, CFSTR(kIONVRAMForceSyncNowPropertyKey), CFSTR("auto-boot"));
    printf("sync nvram ret: %d\n",ret);
    IOObjectRelease(nvram);

    sync();
    return 0;
}
