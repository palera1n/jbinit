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

void spin(){
  puts("jbinit DIED!");
  while (1){
    sleep(5);
  }
}

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

int check_and_mount_dmg() {
    if (access("/binpack/bin/sh", F_OK) != -1) {
        /* binpack already mounted */
        return 0;
    }
    if (access("/binpack.dmg", F_OK) != 0) {
        fprintf(stderr, "/binpack.dmg not found\n");
        return -1;
    }
    if (access("/binpack", F_OK) != 0) {
        fprintf(stderr, "/binpack cannot be accessed! errno=%d\n", errno);
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
    char* hdik_argv[] = { "/usr/sbin/hdik", "-nomount", "/binpack.dmg", NULL };
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
    char* mount_hfs_argv[] = { "/sbin/mount_hfs", "-o", "ro", disk, "/binpack", NULL };
    run("/sbin/mount_hfs", mount_hfs_argv);
    if (access("/binpack/bin/sh", F_OK) != 0) {
        fprintf(stderr, "/binpack.dmg mount failed\n");
        retval = -1;
        goto out;
    }
    retval = 0;
    printf("/binpack.dmg -> %s mounted on /binpack\n", disk);
out:
    return retval;
}

extern char **environ;

void enable_ssh() {
    if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0) {
        char* dropbearkey_argv[] = { "/binpack/usr/bin/dropbearkey", "-f", "/private/var/dropbear_rsa_host_key", "-t", "rsa", "-s", "4096", NULL };
        run(dropbearkey_argv[0], dropbearkey_argv);
    }
    char* launchctl_argv[] = { "/binpack/bin/launchctl", "load", "-w", "/binpack/Library/LaunchDaemons/dropbear.plist", NULL };
    run(launchctl_argv[0], launchctl_argv);
}

int jbloader_main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("========================================\n");
    printf("palera1n: init!\n");
    printf("pid: %d\n",getpid());
    printf("uid: %d\n",getuid());
    enable_ssh();
    printf("palera1n: goodbye!\n");
    printf("========================================\n");
    // startMonitoring();
    // dispatch_main();

    return 0;
}

int launchd_main(int argc, char **argv) {
  check_and_mount_dmg();
  struct stat statbuf;
  {
    int err = 0;
    if ((err = stat("/sbin/launchd", &statbuf))) {
      printf("stat /sbin/launchd FAILED with err=%d!\n",err);
      spin();
    }else{
      puts("stat /sbin/launchd OK");
      
    }
  }
  puts("Closing console, goodbye!");
  /*
    Launchd doesn't like it when the console is open already!
  */
  for (size_t i = 0; i < 10; i++) {
    close(i);
  }
  char* launchd_envp[] = {
	  "DYLD_INSERT_LIBRARIES=/jbin/jb.dylib",
	  NULL
  };
  char* launchd_argv[] = {
    "/sbin/launchd",
    NULL
  };
  int ret = execve(launchd_argv[0], launchd_argv, launchd_envp);
  fprintf(stderr, "execve FAILED with ret=%d\n", ret);
  spin();
  return -1;
}

int main(int argc, char **argv) {
    if (getpid() == 1) {
        return launchd_main(argc, argv);
    } else return jbloader_main(argc, argv);
}