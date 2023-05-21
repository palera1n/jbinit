// 
//  actions.c
//  src/jbloader/helper/actions.c
//  
//  Created 19/05/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define ELLEKIT 1
#define SUBSTITUTE 2
#define LIBHOOKER 3

#define ELLEKIT_ROOTFUL "/usr/libexec/ellekit/loader"
#define ELLEKIT_ROOTLESS "/var/jb/usr/libexec/ellekit/loader"

#define SUBSTITUTE_ROOTFUL "/etc/rc.d/substitute-launcher"

#define LIBHOOKER_ROOTFUL "/etc/rc.d/libhooker"
#define LIBHOOKER_ROOTLESS "/var/jb/etc/rc.d/libhooker"

#define LAUNCHDAEMONS_ROOTFUL "/Library/LaunchDaemons"
#define LAUNCHDAEMONS_ROOTLESS "/var/jb/Library/LaunchDaemons"


int spawn(char* bin, char* args[]) {
    int ret, status;
    pid_t pid;

    ret = posix_spawnp(&pid, bin, NULL, NULL, args, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s %s %d\n", args[0], "failed with error:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}


#pragma mark Starting Tweak Injection Library

int library_installed() {
    FILE *ellekit = fopen(check_rootful()?ELLEKIT_ROOTFUL:ELLEKIT_ROOTLESS, "r");
    if (ellekit != NULL) return ELLEKIT;
    fclose(ellekit);

    FILE *substitute = fopen(SUBSTITUTE_ROOTFUL, "r");
    if (substitute != NULL) return SUBSTITUTE;
    fclose(substitute);

    FILE *libhooker = fopen(check_rootful()?LIBHOOKER_ROOTFUL:LIBHOOKER_ROOTLESS, "r");
    if (libhooker != NULL) return LIBHOOKER;
    fclose(libhooker);

    return 0;
}

int activate_tweaks() {
    char* ellekit_bin = check_rootful() ? ELLEKIT_ROOTFUL : ELLEKIT_ROOTLESS;
    char* libhooker_bin = check_rootful() ? LIBHOOKER_ROOTFUL : LIBHOOKER_ROOTLESS;
    int ret;

    switch(library_installed()) {
        case ELLEKIT: 
            ret = spawn(ellekit_bin, (char*[]){"loader", NULL}); 
            if (ret != 0) {
                fprintf(stderr, "%s %d\n", "Failed to spawn ellekit loader:", ret);
                return ret;
            }
            break;
        case SUBSTITUTE: 
            ret = spawn(SUBSTITUTE_ROOTFUL, (char*[]){"substitute-launcher", NULL});
            if (ret != 0) {
                fprintf(stderr, "%s %d\n", "Failed to spawn substitute loader:", ret);
                return ret;
            }
            break;
        case LIBHOOKER:
            ret = spawn(libhooker_bin, (char*[]){"libhooker", NULL}); 
            if (ret != 0) {
                fprintf(stderr, "%s %d\n", "Failed to spawn libhooker loader:", ret);
                return ret;
            }
            break;
        default: 
            fprintf(stderr, "%s\n", "No valid tweak injection library was found."); 
            return -1; 
    };

    return 0;
}


#pragma mark Starting Launch Daemons

int start_launch_daemons() {
    if (access("/cores/binpack/usr/bin/launchctl", F_OK) != 0) {
        fprintf(stderr, "%s %d %s%s%s\n", "Unable to access launchctl:", errno, "(", strerror(errno), ")");
        return EACCES;
    }

    int ret;
    char *bin = "/cores/binpack/usr/bin/launchctl";
    char *launchdaemons_dir = check_rootful() ? LAUNCHDAEMONS_ROOTFUL : LAUNCHDAEMONS_ROOTLESS;

    ret = spawn(bin, (char*[]){"launchctl", "bootstrap", "system", launchdaemons_dir, NULL}); 
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to spawn launchctl:", ret);
        return ret;
    }

    return 0;
}


#pragma mark Mounting Directories

int mount_directories() {
    int ret;
    struct paleinfo pinfo;

    ret = get_paleinfo(&pinfo, RAMDISK);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to read paleinfo:", ret);
        return ret;
    }
    ret = remount(pinfo.rootdev);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to (re)mount directories:", ret);
        return ret;
    }

    return 0;
}