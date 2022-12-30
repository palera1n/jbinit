#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <sys/clonefile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <mach/mach.h>
#include <stdbool.h>
#include <spawn.h>
#include <sys/mount.h>
#include <sys/utsname.h>
#include <sys/mman.h>

extern char** environ;
#define serverURL "http://static.palera.in" // if doing development, change this to your local server

@import Foundation;
@import Dispatch;
@import SystemConfiguration;

bool deviceReady = false;

int run(const char *cmd, char * const *args){
    int pid = 0;
    int retval = 0;
    char printbuf[0x1000] = {};
    for (char * const *a = args; *a; a++) {
        size_t csize = strlen(printbuf);
        if (csize >= sizeof(printbuf)) break;
        snprintf(printbuf+csize,sizeof(printbuf)-csize, "%s ",*a);
    }

    retval = posix_spawn(&pid, cmd, NULL, NULL, args, NULL);
    printf("Executing: %s (posix_spawn returned: %d)\n",printbuf,retval);
    {
        int pidret = 0;
        printf("waiting for '%s' to finish...\n",printbuf);
        retval = waitpid(pid, &pidret, 0);
        printf("waitpid for '%s' returned: %d\n",printbuf,retval);
        return pidret;
    }
    return retval;
}

#if 0
int copyFile(const char* dst, const char* src, int mode) {
    struct stat statbuf;
    printf("Copying %s -> %s\n", src, dst);
    int fd = open(src, O_RDONLY, 0);
    printf("%s read fd = %d\n", src, fd);
    if (fd == -1) {
        fprintf(stderr, "Failed to open %s for reading\n", src);
        return -1;
    }
    size_t fsize = lseek(fd, 0, SEEK_END);
    printf("size of %s=%lu\n", src, fsize);
    lseek(fd, 0, SEEK_SET);
    printf("reading %s\n", src);
    void* data = mmap(NULL, (fsize & ~0x3fff) + 0x4000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    printf("%s data=0x%p\n",src , data);
    if (data == MAP_FAILED) {
        fprintf(stderr, "failed to mmap\n");
        return -1;
    }
    int didread = read(fd, data, fsize);
    printf("didread=%d\n",didread);
    close(fd);
    printf("Writing to %s\n", dst);
    fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd == -1) {
        fprintf(stderr, "failed to open %s for writing\n", dst);
        return -1;
    }
    int didwrite = write(fd, data, fsize);
    printf("didwrite=%d\n", didwrite);
    close(fd);
    printf("Copied %s -> %s\n", src, dst);
    int err = 0;
    if ((err = stat(dst, &statbuf))) {
      printf("stat %s FAILED with err=%d!\n",dst, err);
      return -1;
    }else{
      printf("stat %s OK\n", dst);
    }
    return didwrite;
}

int check_and_mount_dmg() {
    if (access("/cores/bin/sh", F_OK) != -1) {
        /* binpack already mounted */
        return 0;
    }
    if (access("/private/var/palera1n.dmg", F_OK) != 0) {
        fprintf(stderr, "/private/var/palera1n.dmg not found\n");
        return -1;
    }
    if (access("/cores", F_OK) != 0 && mkdir("/cores", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        fprintf(stderr, "/cores cannot be accessed or created! errno=%d\n", errno);
        return -1;
    }
    char* disk;
    struct utsname name;
    uname(&name);
    if (atoi(name.release) > 21) {
        disk = "/dev/disk4";
    } else {
        disk = "/dev/disk3";
    }
    int retval = 0;
    int pid = 0;
    int pidret = 0;
    char* hdik_argv[] = { "/usr/sbin/hdik", "-nomount", "/private/var/palera1n.dmg", NULL };
    retval = posix_spawn(&pid, "/usr/sbin/hdik",  NULL, NULL, hdik_argv, environ);
    if (retval != 0) {
        fprintf(stderr, "posix_spawn() failed errno=%d\n", errno);
        retval = -1;
        goto out;
    }
    retval = waitpid(pid, &pidret, 0);
    if (!WIFEXITED(pidret)) {
        fprintf(stderr, "hdik was unexpectedly terminated\n");
        retval = -1;
        goto out;
    }
    if (WEXITSTATUS(pidret) != 0) {
        fprintf(stderr, "hdik exited with a non-zero exit code: %d\n", WEXITSTATUS(pidret));
        retval = -1;
        goto out;
    }
    char* mount_hfs_argv[] = { "/sbin/mount_hfs", "-o", "ro", disk, "/cores", NULL };
    run("/sbin/mount_hfs", mount_hfs_argv);
    if (access("/cores/bin/sh", F_OK) != 0) {
        fprintf(stderr, "/private/var/palera1n.dmg mount failed\n");
        retval = -1;
        goto out;
    }
    retval = 0;
    printf("/private/var/palera1n.dmg -> %s mounted on /cores\n", disk);
out:
    return retval;
}

int umount_cores() {
    char* umount_argv[] = {
        "/sbin/umount",
        "-f",
        "/cores",
        NULL
    };
    return run(umount_argv[0], umount_argv);
}

#endif

extern char **environ;

int runCommand(char *argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execve(argv[0], argv, environ);
        fprintf(stderr, "child: Failed to launch! Error: %s\r\n", strerror(errno));
        exit(-1);
    }
    
    // Now wait for child
    int status;
    waitpid(pid, &status, 0);
    
    return WEXITSTATUS(status);
}

void enable_ssh() {
    if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0) {
        char* dropbearkey_argv[] = { "/binpack/usr/bin/dropbearkey", "-f", "/private/var/dropbear_rsa_host_key", "-t", "rsa", "-s", "4096", NULL };
        run(dropbearkey_argv[0], dropbearkey_argv);
    }
    char* launchctl_argv[] = { "/binpack/bin/launchctl", "load", "-w", "/binpack/Library/LaunchDaemons/dropbear.plist", NULL };
    run(launchctl_argv[0], launchctl_argv);
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("========================================\n");
    printf("palera1n: init!\n");
    printf("pid: %d\n",getpid());
    printf("uid: %d\n",getuid());
    enable_ssh();
    printf("palera1n: goodbye from stage2!\n");
    printf("========================================\n");
    // startMonitoring();
    // dispatch_main();

    return 0;
}

