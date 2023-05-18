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

#define NIGHTLY_ROOTFUL "/Applications/Sileo-Nightly.app"
#define NIGHTLY_ROOTLESS "/var/jb/Applications/Sileo-Nightly.app"

#define SILEO_ROOTFUL "/Applications/Sileo.app"
#define SILEO_ROOTLESS "/var/jb/Applications/Sileo.app"

#define ZEBRA_ROOTFUL "/Applications/Zebra.app"
#define ZEBRA_ROOTLESS "/var/jb/Applications/Zebra.app"

int pm_installed() {
    int ret_val = 0;
    FILE *zebra = fopen(check_rootful()?ZEBRA_ROOTFUL:ZEBRA_ROOTLESS, "r");
    if (zebra != NULL) ret_val += 1;
    fclose(zebra);

    FILE *sileo = fopen(check_rootful()?SILEO_ROOTFUL:SILEO_ROOTLESS, "r");
    if (sileo != NULL) ret_val += 2;
    fclose(sileo);

    if (ret_val >= 2) return ret_val;
    FILE *nightly = fopen(check_rootful()?NIGHTLY_ROOTFUL:NIGHTLY_ROOTLESS, "r");
    if (nightly != NULL) ret_val += 2;
    fclose(nightly);

    return ret_val;
}

int install_deb(char *deb_path) {
    const char *dpkg = check_rootful() ? DPKG_BIN_ROOTFUL : DPKG_BIN_ROOTLESS;
    int ret, status;
    pid_t pid;
    
    if (access(check_rootful()?DPKG_BIN_ROOTFUL:DPKG_BIN_ROOTLESS, F_OK) != 0) {
        fprintf(stderr, "%s %d %s%s%s\n", "Unable to access dpkg:", errno, "(", strerror(errno), ")");
        return errno;
    }

    char* args[] = {"dpkg", "--force-all", "-i", deb_path, NULL};
    char* env_rootful[] = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:", NULL};
    char* env_rootless[] ={"PATH=/var/jb/usr/local/sbin:/var/jb/usr/local/bin:/var/jb/usr/sbin:/var/jb/usr/bin:/var/jb/sbin:/var/jb/bin:", NULL};

    ret = posix_spawnp(&pid, dpkg, NULL, NULL, args, check_rootful() ? env_rootful : env_rootless);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "dpkg failed with:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}
