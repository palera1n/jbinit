#include <jbloader.h>

int get_kflags() {
    struct kerninfo kinfo;
    int ret = get_kerninfo(&kinfo, RAMDISK);
    if (ret != 0) {
        printf("%s %d\n", "Failed to read kerninfo:", ret);
        return ret;
    }
    char buf[256];
    sprintf(buf, "%d\n", kinfo.flags);
    write(STDOUT_FILENO, &buf, strlen(buf) + 1);
    return 0;
}

int get_pflags() {
    struct paleinfo pinfo;
    int ret = get_paleinfo(&pinfo, RAMDISK);
    if (ret != 0) {
        printf("%s %d\n", "Failed to read paleinfo:", ret);
        return ret;
    }
    char buf[256];
    sprintf(buf, "%d\n", pinfo.flags);
    write(STDOUT_FILENO, &buf, strlen(buf) + 1);
    return 0;
}

int get_bmhash() {
    char hash[97];
    int ret = get_boot_manifest_hash(hash);
    if (ret != 0) {
        printf("%s %d\n", "Failed to get boot manifest hash:", ret);
        return ret;
    }
 
    char buf[256];
    sprintf(buf, "%s\n", hash);
    write(STDOUT_FILENO, &buf, strlen(buf) + 1);
    return 0;
}
