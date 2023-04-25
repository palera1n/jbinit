/*
 * bakera1n - injector.m
 *
 * Copyright (c) 2023 dora2ios
 *
 */

#include <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>
#include <mach/mach.h>
#include <mach-o/dyld.h>

#define DYLD_INTERPOSE(_replacment, _replacee) \
__attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
__attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };

typedef  void *posix_spawnattr_t;
typedef  void *posix_spawn_file_actions_t;
int posix_spawnp(pid_t *pid,
                 const char *path,
                 const posix_spawn_file_actions_t *action,
                 const posix_spawnattr_t *attr,
                 char *const argv[], char *const envp[]);
int hook_posix_spawnp(pid_t *pid,
                      const char *path,
                      const posix_spawn_file_actions_t *action,
                      const posix_spawnattr_t *attr,
                      char *const argv[], char *envp[])
{
    if(!strcmp(argv[0], "/usr/sbin/cfprefsd"))
    {
        int envcnt = 0;
        while (envp[envcnt] != NULL)
        {
            envcnt++;
        }
        
        char** newenvp = malloc((envcnt + 2) * sizeof(char **));
        int j = 0;
        char* currentenv = NULL;
        for (int i = 0; i < envcnt; i++){
            if (strstr(envp[j], "DYLD_INSERT_LIBRARIES") != NULL)
            {
                currentenv = envp[j];
                continue;
            }
            newenvp[i] = envp[j];
            j++;
        }
        
        char *newlib = "/cores/cfprefsdhook.dylib";
        char *inj = NULL;
        if(currentenv)
        {
            inj = malloc(strlen(currentenv) + 1 + strlen(newlib) + 1);
            inj[0] = '\0';
            strcat(inj, currentenv);
            strcat(inj, ":");
            strcat(inj, newlib);
        }
        else
        {
            inj = malloc(strlen("DYLD_INSERT_LIBRARIES=") + strlen(newlib) + 1);
            inj[0] = '\0';
            strcat(inj, "DYLD_INSERT_LIBRARIES=");
            strcat(inj, newlib);
        }
        newenvp[j] = inj;
        newenvp[j + 1] = NULL;
        
        int ret = posix_spawnp(pid, path, action, attr, argv, newenvp);
        return ret;
    }
    
    return posix_spawnp(pid, path, action, attr, argv, envp);
}
DYLD_INTERPOSE(hook_posix_spawnp, posix_spawnp);

__attribute__((constructor))
static void customConstructor(int argc, const char **argv)
{
    //NSLog(@"hello injector! (pid=%d)", getpid());
}
