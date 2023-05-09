// 
//  pwset.c
//  src/jbloader/helper/pwset.c
//  
//  Created 30/04/2023
//  jbloader (helper)
//

#include <jbloader.h>

int spawn_pw(char *args[], char *input) {
    int fd[2];
    int ret, status;

    const char *bin = check_rootful() ? "/usr/sbin/pw" : "/var/jb/usr/sbin/pw"; 
    if (access(bin, F_OK) != 0) {
        fprintf(stderr, "Unable to access pw: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    if (pipe(fd) == -1) {
        fprintf(stderr, "%s\n", "Broken Pipe");
        return EPIPE;
    }

    dup2(fd[0], STDIN_FILENO);

    pid_t pid;
    ret = posix_spawnp(&pid, bin, NULL, NULL, args, NULL);
    if (ret) {
        fprintf(stderr, "%s spawn failed: %d (%s)\n", bin, errno, strerror(errno));
        return ret;
    }

    dprintf(fd[1], "%s\n", input);
    close(fd[0]);
    close(fd[1]);

    waitpid(pid, &status, 0);
    return 0;
}

int setpw(char *pw) {
    int ret;
    char *args[] = {"pw", "usermod", "501", "-h", "0", NULL};

    ret = spawn_pw(args, pw);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Setting password failed:", ret);
        return ret;
    }

    if (!check_rootful()) {
        char *fake_hash = "!";
        char *root_args[] = {"pw", "usermod", "0", "-H", "0", NULL};

        ret = spawn_pw(root_args, fake_hash);
        if (ret != 0) {
            fprintf(stderr, "%s %d\n", "Setting password failed:", ret);
            return ret;
        }
    }
    
    return 0;
}

