#include <jbloader.h>

int setpw(char *pw) {
    int fd[2];
    char buf[256];

    const char *bin = "/var/jb/usr/sbin/pw"; // todo - prefix
    char *arg[] = {"pw", "usermod", "501", "-h", "0", NULL};

    if (pipe(fd) == -1) {
        return EPIPE;
    }
    
    sprintf(buf, "%s\n", pw);
    write(fd[1], &buf, strlen(buf) + 1);

    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    close(fd[1]);

    pid_t pid;
    return posix_spawn(&pid, bin, NULL, NULL, arg, NULL);
}
