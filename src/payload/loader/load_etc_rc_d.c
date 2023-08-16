#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <paleinfo.h>
#include <fcntl.h>
#include <spawn.h>
#include <limits.h>

extern char** environ;
int load_etc_rc_d(uint64_t pflags) {
    if (pflags & (palerain_option_force_revert | palerain_option_safemode)) return 0;
    char* etcRcD;
    etcRcD = pflags & palerain_option_rootful ? "/etc/rc.d" : "/var/jb/etc/rc.d";
    DIR* dir = opendir(etcRcD);
    if (!dir) return 0;
    struct dirent* d;
    while ((d = readdir(dir))) {
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", etcRcD, d->d_name);
        struct stat st;
        pid_t pid;
        posix_spawn(&pid, path, NULL, NULL, (char*[]){ path, NULL }, environ);
    }
    closedir(dir);
    return 0;
}
