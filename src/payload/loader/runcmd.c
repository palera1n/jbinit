#include <payload/payload.h>
#include <spawn.h>
#include <sys/wait.h>

extern char** environ;

int runCommand(char* argv[]) {
    pid_t pid;
    int ret = posix_spawn(&pid, argv[0], NULL, NULL, argv, environ);
    if (ret) return -1;
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) return 0;
    return status;
}
