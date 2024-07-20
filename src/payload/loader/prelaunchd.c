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
#include <libjailbreak/libjailbreak.h>
#include <APFS/APFS.h>
#include <IOKit/IOKitLib.h>
#include <sys/kern_memorystatus.h>
#include <mount_args.h>
#include <sys/snapshot.h>
#include <dlfcn.h>

bool has_verbose_boot;
bool panic_did_enter = false;

int prelaunchd(uint32_t payload_options, struct paleinfo* pinfo_p) {
    has_verbose_boot = (strcmp(getenv("JB_HAS_VERBOSE_BOOT"), "1") == 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("plooshInit prelaunchd...\n");
    int platform = get_platform();
    if (platform == -1) {
        _panic("failed to determine current platform\n");
    }

    if ((payload_options & payload_option_userspace_rebooted) == 0) {
        printf("mount binpack\n");
        CHECK_ERROR(mount_dmg("ramfile://checkra1n", "hfs", "/cores/binpack", MNT_RDONLY, 1), 1, "mount binpack failed");
        printf("mount loader\n");
        if (platform == PLATFORM_IOS) {
            CHECK_ERROR(mount_dmg("/cores/binpack/loader.dmg", "hfs", "/cores/binpack/Applications", MNT_RDONLY, 0), 1, "mount loader failed");
        } else if (platform == PLATFORM_TVOS) {
            CHECK_ERROR(mount_dmg("/cores/binpack/tvloader.dmg", "hfs", "/cores/binpack/Applications", MNT_RDONLY, 0), 1, "mount loader failed");
        }
        if (platform == PLATFORM_BRIDGEOS) {
            printf("mount strap\n");
            if (access("/usr/share/palera1n-strap", F_OK) == 0 && access("/usr/share/palera1n-strap.dmg", F_OK) == 0) {
                CHECK_ERROR(mount_dmg("/usr/share/palera1n-strap.dmg", "hfs", "/usr/share/palera1n-strap", MNT_RDONLY, 0), 0, "mount strap failed");
            }
        }
    }

    char dev_rootdev[32];
    snprintf(dev_rootdev, 32, "/dev/%s", pinfo_p->rootdev);
    if ((pinfo_p->flags & (palerain_option_rootful | palerain_option_force_revert)) == (palerain_option_rootful | palerain_option_force_revert)) {
        if (pinfo_p->flags & palerain_option_ssv) {
            printf("will delete %s\n", dev_rootdev);
            if (access(dev_rootdev, F_OK) == 0) {
                int16_t role = 0;
                CHECK_ERROR(APFSVolumeRole(dev_rootdev, &role, NULL), 0, "APFSVolumeRole(%s) Failed", dev_rootdev);
                printf("found apfs volume role: 0x%04x\n", role);
                if (role != APFS_VOL_ROLE_RECOVERY) {
                    _panic("BUG: SAFETY: deleting non-recovery volume is not allowed\n");
                } else {
                    CHECK_ERROR(errno = APFSVolumeDelete(pinfo_p->rootdev), 1, "failed to delete fakefs");
                }
            }
        }
    }
    
    #define ELLEKIT_ACTUAL_PATH "/cores/binpack/usr/lib/libellekit.dylib"
    void* ellekit_handle = dlopen(ELLEKIT_ACTUAL_PATH, RTLD_NOW);
    
    if (ellekit_handle == NULL) {
        printf("%s\n", dlerror());
        CHECK_ERROR(mount("bindfs", "/cores/binpack/Library/Frameworks/CydiaSubstrate.framework", MNT_RDONLY, "/cores/binpack/Library/Frameworks/CydiaSubstrateBridgeOS.framework"), 1, "failed to bindfs /cores/binpack/Library/Frameworks/CydiaSubstrateBridgeOS.framework -> /cores/binpack/Library/Frameworks/CydiaSubstrate.framework");
    } else {
        dlclose(ellekit_handle);
    }

    if (pinfo_p->flags & palerain_option_setup_rootful) {
        return setup_fakefs(payload_options, pinfo_p);
    }

    return 0;
}

_Noreturn void _panic(char* fmt, ...) {
    panic_did_enter = true;
  char reason[1024], reason_real[1024];
  va_list va;
  va_start(va, fmt);
  vsnprintf(reason, 1024, fmt, va);
  va_end(va);
  snprintf(reason_real, 1024, "payload: %s", reason);
  int fd = open("/cores/panic.txt", O_WRONLY | O_CREAT, 0644);
  if (fd != -1) {
    write(fd, reason_real, 1024);
    close(fd);
  }
  kill(1, SIGUSR1);
    while (1) sleep (86400);
}
