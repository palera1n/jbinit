#include <jbloader.h>

int setpw(char *pw) {
    int fd[2];

    const char *bin = "/var/jb/usr/sbin/pw"; // todo - prefix
    char *arg[] = {"pw", "usermod", "501", "-h", "0", NULL};

    if (pipe(fd) == -1) {
        return EPIPE;
    }

    dup2(fd[0], STDIN_FILENO);

    pid_t pid;
    int ret = posix_spawn(&pid, bin, NULL, NULL, arg, NULL);
    if (ret) {
        fprintf(stderr, "Could not spawn %s: %d (%s)\n", bin, errno, strerror(errno));
        return ret;
    }
    dprintf(fd[1], "%s\n", pw);
    close(fd[0]);
    close(fd[1]);
    return 0;
}
