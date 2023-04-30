#include <jbloader.h>

int move_rootless_if_required() {
    if (checkrain_options_enabled(pinfo.flags, palerain_option_rootful) || (access("/var/jb", F_OK) != 0))
        return 0;
    char hash[97];
    int ret = get_boot_manifest_hash(hash);
    if (ret != 0) {
      fprintf(stderr, "cannot get boot manifest hash\n");
      return ret;
    }
    char oldPrebootPath[PATH_MAX];
    char currentVarJbDest[PATH_MAX];
    memset(oldPrebootPath, '\0', PATH_MAX);
    memset(currentVarJbDest, '\0', PATH_MAX);
    snprintf(oldPrebootPath, PATH_MAX, "/private/preboot/%s/procursus", hash);
    ssize_t currentVarJbDestLen = readlink("/var/jb", currentVarJbDest, PATH_MAX);
    if (currentVarJbDestLen == -1) {
        printf("cannot readlink /var/jb: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    printf("CurrentVarJbDest: %s\noldPrebootPath: %s\n", currentVarJbDest, oldPrebootPath);
    if (
        strncmp(oldPrebootPath, currentVarJbDest, strlen(oldPrebootPath))
    ) return 0;
    printf("will move jb root path\n");
    char newPrebootPath[PATH_MAX];
    snprintf(newPrebootPath, PATH_MAX, "/private/preboot/%s/jb-XXXXXXXX", hash);
    printf("newPrebootPath (template): %s\n", newPrebootPath);
    char* path = mkdtemp(newPrebootPath);
    if (path == NULL) {
        fprintf(stderr, "failed to make new path: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    printf("newPrebootPath: %s\n", newPrebootPath);
    int fd;
    while ((fd = open(newPrebootPath, O_DIRECTORY | O_RDONLY)) == -1) { 
        fprintf(stderr, "failed to open new path: %d (%s)\n", errno, strerror(errno));
        sleep(1);
    }
    ret = fchmod(fd, 0755);
    assert(ret == 0);
    ret = fchown(fd, 0, 0);
    assert(ret == 0);
    ret = renameat(0, oldPrebootPath, fd, "procursus");
    if (ret != 0) {
        printf("failed to rename %s -> %s/procursus: %d (%s)\n", oldPrebootPath, newPrebootPath, errno, strerror(errno));
        return -1;
    }
    ret = unlink("/var/jb");
    if (ret != 0) {
        printf("failed to delete /var/jb: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    close(fd);
    char newVarJbDest[PATH_MAX];
    snprintf(newVarJbDest, PATH_MAX, "%s/procursus", newPrebootPath);
    ret = symlink(newVarJbDest, "/var/jb");
    if (ret != 0) {
        printf("failed to symlink /var/jb: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    printf("moved jb root path from %s to %s\n", currentVarJbDest, newVarJbDest);
    return 0;
}
