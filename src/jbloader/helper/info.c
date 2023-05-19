// 
//  info.c
//  src/jbloader/helper/info.c
//  
//  Created 30/04/2023
//  jbloader (helper)
//

#include <jbloader.h>

int get_kflags() {
    struct kerninfo kinfo;
    int ret = get_kerninfo(&kinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read kerninfo:", ret);
        return ret;
    }
    char buf[16];
    snprintf(buf, 16, "%d\n", kinfo.flags);
    write(STDOUT_FILENO, &buf, strlen(buf) + 1);
    return 0;
}

int get_pflags() {
    struct paleinfo pinfo;
    int ret = get_paleinfo(&pinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read paleinfo:", ret);
        return ret;
    }
    char buf[16];
    snprintf(buf, 16, "%d\n", pinfo.flags);
    write(STDOUT_FILENO, &buf, strlen(buf) + 1);
    return 0;
}

int get_bmhash() {
    char hash[97];
    int ret = get_boot_manifest_hash(hash);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to get boot manifest hash:", ret);
        return ret;
    }
 
    write(STDOUT_FILENO, &hash, strlen(hash) + 1);
    return 0;
}

int check_forcerevert() {
    char str[3];
    struct kerninfo kinfo;
    int ret = get_kerninfo(&kinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read kerninfo:", ret);
        return ret;
    }

    return (kinfo.flags & checkrain_option_force_revert) != 0;
}

int check_rootful() {
    char str[3];
    struct paleinfo pinfo;
    int ret = get_paleinfo(&pinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read paleinfo:", ret);
        return ret;
    }

    return (pinfo.flags & palerain_option_rootful) != 0;
}

void output_flags(uint32_t flags, const char* prefix, const char* (strflags)(checkrain_option_t opt)) {
    for (uint8_t bit = 0; bit < 32; bit++) {
        if (checkrain_options_enabled(flags, (1 << bit))) {
            char printbuf[0x30];
            const char* opt_str = strflags(1 << bit);
            if (opt_str == NULL) {
                snprintf(printbuf, 0x30, "%s_option_unknown_%" PRIu8, prefix, bit);
            }
            printf("%s,", opt_str == NULL ? printbuf : opt_str);
        }
    }
}

void print_pflags_str() {
    struct paleinfo pinfo;
    int ret = get_paleinfo(&pinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read paleinfo:", ret);
        return;
    }

    output_flags(pinfo.flags, "palerain", str_palerain_flags);
}

void print_kflags_str() {
    struct kerninfo kinfo;
    int ret = get_kerninfo(&kinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read kerninfo:", ret);
        return;
    }

    output_flags(kinfo.flags, "checkrain", str_checkrain_flags);
}
