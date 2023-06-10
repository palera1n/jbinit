// 
//  safemode.c
//  src/jbloader/helper/safemode.c
//  
//  Created 16/05/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define LAUNCHCTL_BIN "/cores/binpack/bin/launchctl"
#define RDISK "/dev/md0"

int userspace_reboot() {
    int ret;
    char* args[] = {"launchctl", "reboot", "userspace",  NULL};
    pid_t pid;
    int status;

    if (access(LAUNCHCTL_BIN, F_OK) != 0) {
        fprintf(stderr, "%s %s %s%d%s\n", "could not access launchctl:", strerror(errno), "(", errno, ")");
        return errno;
    }

    ret = posix_spawnp(&pid, LAUNCHCTL_BIN, NULL, NULL, args, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "launchctl failed with:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}

int safemode(int enter_safemode) {
    struct kerninfo kinfo;
    int ret;
    
    ret = get_kerninfo(&kinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read kerninfo:", ret);
        return ret;
    }

    int safemode = (kinfo.flags & checkrain_option_safemode) != 0;
    if (safemode == enter_safemode) {
        const char *mode = safemode ? "Already in safemode." : "Already in normal mode.";
        fprintf(stdout, "%s\n", mode);
        return 0;
    } 

    if (enter_safemode) kinfo.flags += checkrain_option_safemode;
    else kinfo.flags -= checkrain_option_safemode;

    int rmd = open(RAMDISK, O_RDWR);
    if (ret == -1) {
        fprintf(stderr, "%s %d\n", "Failed to open ramdisk:", rmd);
        return rmd;
    }

    uint32_t rd_end = 0;
    read(rmd, &rd_end, 4);
    lseek(rmd, (long)rd_end, SEEK_SET);
    write(rmd, &kinfo, sizeof(struct kerninfo));
    close(rmd);

    return userspace_reboot();
}
