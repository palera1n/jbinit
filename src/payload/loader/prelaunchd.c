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
#include <APFS/APFS.h>
#include <IOKit/IOKitLib.h>
#include <sys/kern_memorystatus.h>
#include <mount_args.h>

int prelaunchd(uint32_t payload_options, struct paleinfo* pinfo_p) {
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("plooshInit prelaunchd...\n");

    if ((payload_options & payload_option_userspace_rebooted) == 0) {
        printf("mount binpack\n");
        CHECK_ERROR(mount_dmg("ramfile://checkra1n", "hfs", "/cores/binpack", MNT_RDONLY, 1), 1, "mount binpack failed");
        printf("mount loader\n");
        CHECK_ERROR(mount_dmg("/cores/binpack/loader.dmg", "hfs", "/cores/binpack/Applications", MNT_RDONLY, 0), 1, "mount loader failed");
    }

    CHECK_ERROR(memorystatus_control(MEMORYSTATUS_CMD_SET_JETSAM_TASK_LIMIT, 1, 4096, NULL, 0), 1, "failed to set launchd memory limit");

    char dev_rootdev[32];
    snprintf(dev_rootdev, 32, "/dev/%s", pinfo_p->rootdev);
    if ((pinfo_p->flags & palerain_option_rootful) && ((pinfo_p->flags & palerain_option_force_revert))) {
        printf("will delete %s\n", dev_rootdev);
        if (access(dev_rootdev, F_OK) == 0) {
            int16_t role = 0;
            kern_return_t ret;
            CHECK_ERROR(APFSVolumeRole(dev_rootdev, &role, NULL), 0, "APFSVolumeRole(%s) Failed", dev_rootdev);
            printf("found apfs volume role: 0x%04x\n", role);
            if (role != APFS_VOL_ROLE_RECOVERY) {
                fprintf(stderr, "BUG: SAFETY: deleting non-recovery volume is not allowed\n");
                spin();
            } else {
                CHECK_ERROR(errno = APFSVolumeDelete(pinfo_p->rootdev), 1, "failed to delete fakefs");
            }
        }
    }
    
    if (pinfo_p->flags & palerain_option_setup_rootful) {
        return setup_fakefs(payload_options, pinfo_p);
    }

    return 0;
}
