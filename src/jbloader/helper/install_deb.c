// 
//  install_deb.c
//  src/jbloader/helper/install_deb.c
//  
//  Created 07/05/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define DPKG_BIN_ROOTFUL "/usr/bin/dpkg"
#define DPKG_BIN_ROOTLESS "/var/jb/usr/bin/dpkg"

int dpkg_checks() {
    const char *dpkg = check_rootful() ? DPKG_BIN_ROOTFUL : DPKG_BIN_ROOTLESS;
    if (access(dpkg, F_OK) != 0) {
        fprintf(stderr, "Unable to access dpkg: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

int dpkg_check_install(char* package) {
    int ret, status;
    pid_t pid;

    char* args[] = {"dpkg", "-s", package, NULL};
    const char *apt = check_rootful() ? DPKG_BIN_ROOTFUL : DPKG_BIN_ROOTLESS;
    if (access(apt, F_OK) != 0) {
        fprintf(stderr, "Unable to access dpkg: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    ret = posix_spawnp(&pid, apt, NULL, NULL, args, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "dpkg failed with error:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}

int install_deb(char *deb_path) {
    const char *dpkg = check_rootful() ? DPKG_BIN_ROOTFUL : DPKG_BIN_ROOTLESS;
    int ret, status;
    pid_t pid;
    
    ret = dpkg_checks();
    if (ret != 0) {
        fprintf(stderr, "%s\n", "Pre checks for dpkg have failed.");
        return -1;
    }
    
    char* args[] = {"dpkg", "--force-all", "-i", deb_path, NULL};
    char* env_rootful[] = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:", NULL};
    char* env_rootless[] ={"PATH=/var/jb/usr/local/sbin:/var/jb/usr/local/bin:/var/jb/usr/sbin:/var/jb/usr/bin:/var/jb/sbin:/var/jb/bin:", NULL};

    ret = posix_spawnp(&pid, dpkg, NULL, NULL, args, check_rootful() ? env_rootful : env_rootless);
    if (ret != 0) {
        fprintf(stderr, "%s %d", "dpkg failed with:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}
