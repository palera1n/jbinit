// 
//  revert.c
//  src/jbloader/helper/revert.c
//  
//  Created 30/04/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define RDISK "/dev/md0"

int clean_fakefs() {
    struct paleinfo pinfo;
    int ret;

    ret = get_paleinfo(&pinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read paleinfo:", ret);
        return ret;
    }

    int clean = (pinfo.flags & palerain_option_clean_fakefs) != 0;
    if (clean) return userspace_reboot();

    pinfo.flags += palerain_option_clean_fakefs;
    int rmd = open(RAMDISK, O_RDWR);
    if (ret == -1) {
        fprintf(stderr, "%s %d\n", "Failed to open ramdisk:", rmd);
        return rmd;
    }

    uint32_t rd_end = 0;
    read(rmd, &rd_end, 4);
    lseek(rmd, (long)rd_end + 0x1000L, SEEK_SET);
    write(rmd, &pinfo, sizeof(struct paleinfo));
    close(rmd);

    return userspace_reboot();
}

int revert_install() {
    if (check_rootful()) {
        return clean_fakefs();
    } else {
        if (init_info()) return -1;
        return jailbreak_obliterator();
    }
}
