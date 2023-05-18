// 
//  pwset.c
//  src/jbloader/helper/pwset.c
//  
//  Created 30/04/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define MASTER_ROOTFUL "/etc/master.passwd"
#define MASTER_ROOTLESS "/var/jb/etc/master.passwd"

int spawn_pw(char *args[], char *input) {
    int fd[2];
    int ret, status;

    const char *bin = check_rootful() ? "/usr/sbin/pw" : "/var/jb/usr/sbin/pw"; 
    if (access(bin, F_OK) != 0) {
        fprintf(stderr, "%s %d %s%s%s\n", "Unable to access pw:", errno, "(", strerror(errno), ")");
        return errno;
    }

    if (pipe(fd) == -1) {
        fprintf(stderr, "%s\n", "Broken Pipe");
        return EPIPE;
    }

    dup2(fd[0], STDIN_FILENO);

    pid_t pid;
    ret = posix_spawnp(&pid, bin, NULL, NULL, args, NULL);
    if (ret) {
        fprintf(stderr, "%s %s %d %s%s%s\n", bin, "spawn failed:", errno, "(", strerror(errno), ")");
        return errno;
    }

    dprintf(fd[1], "%s\n", input);
    close(fd[0]);
    close(fd[1]);

    waitpid(pid, &status, 0);
    return 0;
}

int check_default_hash() {
    const char *master_pw_file = check_rootful() ? MASTER_ROOTFUL : MASTER_ROOTLESS;
    FILE* fd = fopen(master_pw_file, "r");
    if (fd == NULL) {
        fprintf(stderr, "%s\n", "Failed to open master.passwd.");
        return -1;
    }

    char buf[512];
    while (fgets(buf, 512, fd)) {
        buf[strcspn(buf, "\n")] = 0;       
        char *token = strtok(buf, ":");
        
        if (token == NULL) {
            fprintf(stderr, "%s\n", "Failed to parse master.passwd.");
            return -1;
        }

        const char *name = token;
        token = strtok(NULL, ":");
        const char *hash = token; 

        if (!strcmp(name, "root") && !strcmp(hash, "/smx7MYTQIi2M")) return 1;
    }

    fclose(fd);
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

    if (!check_rootful() || check_default_hash()) {
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
