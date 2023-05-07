#include <jbloader.h>

int move_rootless(char* procursusPath, char* hash) {
    char newPath[200];
    snprintf(newPath, 200, "/private/preboot/%s/jb-XXXXXXXX", hash);
    char* path = mkdtemp(newPath);
    if (path == NULL) {
        fprintf(stderr, "failed to make new path: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    int fd;
    while ((fd = open(newPath, O_DIRECTORY | O_RDONLY)) == -1) { 
        fprintf(stderr, "failed to open new path: %d (%s)\n", errno, strerror(errno));
        sleep(1);
    }
    int ret = fchmod(fd, 0755);
    assert(ret == 0);
    ret = fchown(fd, 0, 0);
    assert(ret == 0);
    ret = renameat(0, procursusPath, fd, "procursus");
    if (ret != 0) {
        printf("failed to rename %s -> %s/procursus: %d (%s)\n", procursusPath, newPath, errno, strerror(errno));
        return -1;
    }
    ret = unlink("/var/jb");
    if (ret != 0 && errno != ENOENT) {
        printf("failed to delete /var/jb: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    close(fd);
    ret = symlink(newPath, "/var/jb");
    if (ret != 0) {
        printf("failed to symlink /var/jb: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    printf("moved jb root path from %s to %s\n", procursusPath, newPath);
    return 0;
}

int fixup_var_jb(char* jbPath) {
    if (checkrain_options_enabled(jbloader_flags, jbloader_userspace_rebooted)) {
        return 0;
    }
    int ret;
    if (access("/var/jb", F_OK) == 0) {
        ret = unlink("/var/jb");
        if (ret != 0) {
            fprintf(stderr, "could not delete /var/jb: %d (%s)\n", errno ,strerror(errno));
            return -1;
        }
    }
    char jbProcursusPath[200];
    snprintf(jbProcursusPath, 200, "%s/procursus", jbPath);
    ret = symlink(jbProcursusPath, "/var/jb");
    if (ret != 0) {
        fprintf(stderr, "could not create /var/jb: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    printf("/var/jb -> %s\n", jbProcursusPath);
    return 0;
}


int move_rootless_if_required() {
    int ret;
    if (checkrain_options_enabled(pinfo.flags, palerain_option_rootful)) {
        ret = unlink("/var/jb");
        if (ret != 0 && errno != ENOENT) {
            fprintf(stderr, "could not delete /var/jb on rootful: %d (%s)\n", errno, strerror(errno));
            return -1;
        }
        return 0;
    }
    char hash[97];
    char prebootPath[200];
    char procursusPath[200] = "";
    char jbPath[200]= "";
    ret = get_boot_manifest_hash(hash);
    if (ret != 0) {
      fprintf(stderr, "cannot get boot manifest hash\n");
      return ret;
    }
    snprintf(prebootPath, 200, "/private/preboot/%s", hash);
    if (access(prebootPath, F_OK) != 0) {
        fprintf(stderr, "wat?? %s not found\n", prebootPath);
        return -1;
    }
    DIR* d = opendir(prebootPath);
    struct dirent *dirp;
    while ((dirp = readdir(d))) {
        if (!strcmp(dirp->d_name, "procursus")) {
            snprintf(procursusPath, 200, "/private/preboot/%s/procursus", hash);
        } else if (!strncmp(dirp->d_name, "jb-", 3)) {
            if (jbPath[0] != '\0') {
                fprintf(stderr, "Duplicate /private/preboot/%s/jb-* path found, bailing out\n", hash);
                fprintf(stderr, "current jb path: /private/preboot/%s/%s\n", hash, dirp->d_name);
                fprintf(stderr, "stored jb path: %s\n", jbPath);
                return -1;
            }
            snprintf(jbPath, 200, "/private/preboot/%s/%s", hash, dirp->d_name);
        }
    }
    if (procursusPath[0] != '\0' && jbPath[0] != '\0') {
        fprintf(stderr, "both old and new style rootless path found, bailing out\n");
        return -1;
    }
    closedir(d);
    if (procursusPath[0] != '\0') 
        return move_rootless(procursusPath, hash);
    else if (jbPath[0] != '\0')
        return fixup_var_jb(jbPath);

    printf("%s: no jb root path found, deleting /var/jb\n", __FUNCTION__);
    ret = unlink("/var/jb");
    if (ret != 0) {
        fprintf(stderr, "failed to unlink /var/jb: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}
