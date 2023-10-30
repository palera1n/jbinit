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

int copyfile_fakefs_cb(int what, int stage, copyfile_state_t state, const char * src, const char * dst, void * ctx) {
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

            }
            break;
    }
    return COPYFILE_CONTINUE;

}

int setup_fakefs(uint32_t payload_options, struct paleinfo* pinfo_p) {
    CHECK_ERROR(runCommand((char*[]){ "/sbin/fsck", "-qL", NULL }), 1, "fsck failed");
    CHECK_ERROR(runCommand((char*[]){ "/sbin/mount", "-P", "1", NULL }), 1, "mount-phase-1 failed");
    CHECK_ERROR(runCommand((char*[]){ "/usr/libexec/init_data_protection",  NULL }), 1, "init_data_protection failed");
    CHECK_ERROR(runCommand((char*[]){ "/sbin/mount", "-P", "2",  NULL }), 1, "mount-phase-2 failed");
    CHECK_ERROR(runCommand((char*[]){ "/usr/libexec/keybagd", "--init",  NULL }), 1, "keybag failed");

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
    
    {
        struct attrlist alist = { 0 };
        char abuf[2048];
        int rootfs_fd = open("/", O_RDONLY | O_DIRECTORY);
        if (rootfs_fd == -1) {
            fprintf(stderr, "open(/) failed: %d (%s)\n", errno, strerror(errno));
        }
        alist.commonattr = ATTR_BULK_REQUIRED;
	    int count = fs_snapshot_list(rootfs_fd, &alist, &abuf[0], sizeof (abuf), 0);
        if (count < 0) {
            fprintf(stderr, "fs_snapshot_list(/) failed\n");
            spin();
        }
        bool found_snapshot = false;
        char *p = &abuf[0];
        io_registry_entry_t chosen = IORegistryEntryFromPath(0, "IODeviceTree:/chosen");
        if (!MACH_PORT_VALID(chosen)) {
          fprintf(stderr, "get /chosen failed");
          spin();
        }
        char expectedSnapshotName[150];
        CFDataRef root_snapshot_name_data = IORegistryEntryCreateCFProperty(chosen, CFSTR("root-snapshot-name"), kCFAllocatorDefault, 0);
        if (root_snapshot_name_data == NULL) {
          fprintf(stderr, "cannot get root-snapshot-name\n");
          spin();
        }
        CFStringRef RootSnapshotName = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, root_snapshot_name_data, kCFStringEncodingUTF8);
        IOObjectRelease(chosen);
        CFStringGetCString(RootSnapshotName, expectedSnapshotName, 150, kCFStringEncodingUTF8);
        CFRelease (RootSnapshotName);
        CFRelease (root_snapshot_name_data);
        while (count > 0) {
			    char *field = p;
	      	uint32_t len = *(uint32_t *)field;
		  	  field += sizeof (uint32_t);
			    attribute_set_t attrs = *(attribute_set_t *)field;
    			field += sizeof (attribute_set_t);

		    	if (attrs.commonattr & ATTR_CMN_NAME) {
				    attrreference_t ar = *(attrreference_t *)field;
    				char *name = field + ar.attr_dataoffset;
		    		field += sizeof(attrreference_t);
				    if (strcmp(name, expectedSnapshotName) == 0) {
              found_snapshot = true;
              break;
            }
          }
          count--;
        }
        printf("snapshot name: %s\n", expectedSnapshotName);
        if (!found_snapshot) {
            fprintf(stderr, "expected rootfs snapshot %s not found\n", expectedSnapshotName);
            spin();
        }
        CHECK_ERROR(fs_snapshot_mount(rootfs_fd, "/cores/fs/real", expectedSnapshotName, MNT_RDONLY), 1, "fs_snapshot_mount() failed");
        close(rootfs_fd);
    }

    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    int recoveryNumber = APFS_VOL_ROLE_RECOVERY;
    CFNumberRef volumeRole = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &recoveryNumber);
    CFDictionaryAddValue(dict, kAPFSVolumeRoleKey, volumeRole);
    CFDictionaryAddValue(dict, kAPFSVolumeNameKey, CFSTR("Xystem"));
    CFDictionaryAddValue(dict, kAPFSVolumeCaseSensitiveKey, kCFBooleanTrue);

    CHECK_ERROR(APFSVolumeCreate("disk0s1", dict), 1, "APFSVolumeCreate failed");
    char actual_fakefs_mntfromname[50];
    int32_t fsindex;
    CFNumberGetValue(CFDictionaryGetValue(dict, kAPFSVolumeFSIndexKey), kCFNumberSInt32Type, &fsindex);
    CFRelease(volumeRole);
    CFRelease(dict);

    if (strncmp(rootfs_st.f_mntfromname, "/dev/disk0s1s", sizeof("/dev/disk0s1s")-1) == 0) {
        snprintf(actual_fakefs_mntfromname, 50, "/dev/disk0s1s%d", fsindex+1);
    } else if (strncmp(rootfs_st.f_mntfromname, "/dev/disk1s", sizeof("/dev/disk1s")-1) == 0) {
        snprintf(actual_fakefs_mntfromname, 50, "/dev/disk1s%d", fsindex+1);
    } else {
        fprintf(stderr, "unexpected rootfs f_mntfromname\n");
        spin();
    }
    if (strcmp(actual_fakefs_mntfromname, fakefs_mntfromname)) {
        fprintf(stderr, "unexpected fakefs name %s (expected %s)\n", actual_fakefs_mntfromname, fakefs_mntfromname);
        spin();
    }
    sleep(2);
    struct apfs_mount_args args = {
        fakefs_mntfromname, 0, APFS_MOUNT_LIVEFS, 0
    };
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
    
    runCommand((char*[]){ "/usr/libexec/seputil", "--gigalocker-shutdown", NULL });

    if (access("/sbin/umount", F_OK) == 0)
        runCommand((char*[]){ "/sbin/umount", "-a", NULL });

    unmount("/private/var", MNT_FORCE);
    unmount("/cores/fs/real", MNT_FORCE);
    unmount("/cores/fs/fake", MNT_FORCE);
    unmount("/private/xarts", MNT_FORCE);

    if (access("/sbin/umount", F_OK) == 0)
        runCommand((char*[]){ "/sbin/umount", "-a", NULL });

    sync();
    return 0;
}
