#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <paleinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <spawn.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

extern char** environ;

char* waitpid_decode(int status) {
    char* retbuf = calloc(50, 1);
    if (!retbuf) return NULL;

    if (WIFEXITED(status)) {
        snprintf(retbuf, 50, "exited with code %d", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        if (WCOREDUMP(status))
            snprintf(retbuf, 50, "terminated by signal %d (Core Dumped)", WTERMSIG(status));
        else
            snprintf(retbuf, 50, "terminated by signal %d", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        snprintf(retbuf, 50, "stopped by signal %d", WTERMSIG(status));
    }

    return retbuf;

}

int load_etc_rc_d(uint64_t pflags) {
    if (pflags & (palerain_option_force_revert | palerain_option_safemode)) return 0;
    char* etcRcD;
    etcRcD = pflags & palerain_option_rootful ? "/etc/rc.d" : "/var/jb/etc/rc.d";
    DIR* dir = opendir(etcRcD);
    if (!dir) return 0;
    struct dirent* d;
    while ((d = readdir(dir))) {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")) continue;
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", etcRcD, d->d_name);
        struct stat st;
        pid_t pid;
        int status;
        int ret = posix_spawn(&pid, path, NULL, NULL, (char*[]){ path, NULL }, environ);
        if (ret) {
            fprintf(stderr, "posix_spawn %s failed: %d (%s)\n", path, ret, strerror(ret));
        }
        ret = waitpid(pid, &status, 0);
        if (ret == -1) {
            fprintf(stderr, "waitpid %d failed: %d (%s)\n", pid, errno, strerror(errno));
        } else {
            char* desc = waitpid_decode(status);
            if (desc) {
                printf("%s %s\n", path, desc);
                free(desc);
            }
        }
    }
    closedir(dir);
    return 0;
}

