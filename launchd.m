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

#define serverURL "http://static.palera.in" // if doing development, change this to your local server

@import Foundation;
@import Dispatch;
@import SystemConfiguration;

typedef  void *posix_spawnattr_t;
typedef  void *posix_spawn_file_actions_t;
int posix_spawn(pid_t *, const char *,const posix_spawn_file_actions_t *,const posix_spawnattr_t *,char *const __argv[],char *const __envp[]);

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
    printf("Execting: %s (posix_spawn returned: %d)\n",printbuf,retval);
    {
        int pidret = 0;
        printf("waiting for '%s' to finish...\n",printbuf);
        retval = waitpid(pid, &pidret, 0);
        printf("waitpid for '%s' returned: %d\n",printbuf,retval);
        return pidret;
    }
    return retval;
}

int downloadFile(const char *url, const char *path) {
    NSLog(@"Downloading %s to %s", url, path);
    char *wgetArgs[] = {"/wget", "-O", (char *)path, (char *)url, NULL};
    return run("/wget", wgetArgs);
}

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

int downloadAndInstallBootstrap() {
    if (access("/var/pkg", F_OK) != -1) {
        run("/var/pkg/usr/sbin/dropbear", (char * const []){"/var/pkg/usr/sbin/dropbear", "-S", "/var/pkg/bin/bash", "-r", "/var/pkg/dropbear_rsa_host_key", "-p", "44", "-F", NULL});
        return 0;
    }
    downloadFile(serverURL "/binpack.tar", "/tmp/binpack.tar");
    unlink("/var/pkg");
    mkdir("/var/pkg", 0755);
    char *tarArgs[] = {"/tar", "-xvf", "/tmp/binpack.tar", "-C", "/var/pkg", NULL};
    run("/tar", tarArgs);
    run("/var/pkg/usr/sbin/dropbear", (char * const []){"/var/pkg/usr/sbin/dropbear", "-S", "/var/pkg/bin/bash", "-r", "/var/pkg/dropbear_rsa_host_key", "-p", "44", "-F", NULL});
    // we gamin now
    return 0;
}

SCNetworkReachabilityRef reachability;

void destroy_reachability_ref(void) {
    SCNetworkReachabilitySetCallback(reachability, nil, nil);
    SCNetworkReachabilitySetDispatchQueue(reachability, nil);
    reachability = nil;
}

void given_callback(SCNetworkReachabilityRef ref, SCNetworkReachabilityFlags flags, void *p) {
    if (flags & kSCNetworkReachabilityFlagsReachable) {
        NSLog(@"connectable");
        if (!deviceReady) {
            deviceReady = true;
            downloadAndInstallBootstrap();
        }
        destroy_reachability_ref();
    }
}

void startMonitoring(void) {
    struct sockaddr addr = {0};
    addr.sa_len = sizeof (struct sockaddr);
    addr.sa_family = AF_INET;
    reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, &addr);
    if (!reachability && !deviceReady) {
        deviceReady = true;
        downloadAndInstallBootstrap();
        return;
    }

    SCNetworkReachabilityFlags existingFlags;
    // already connected
    if (SCNetworkReachabilityGetFlags(reachability, &existingFlags) && (existingFlags & kSCNetworkReachabilityFlagsReachable)) {
        deviceReady = true;
        downloadAndInstallBootstrap();
    }
    
    SCNetworkReachabilitySetCallback(reachability, given_callback, nil);
    SCNetworkReachabilitySetDispatchQueue(reachability, dispatch_get_main_queue());
}

int main(int argc, char **argv){
    unlink(argv[0]);
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("========================================\n");
    printf("palera1n: init!\n");
    printf("pid: %d",getpid());
    printf("uid: %d",getuid());
    printf("palera1n: goodbye!\n");
    printf("========================================\n");

    startMonitoring();

    dispatch_main();

    return 0;
}
