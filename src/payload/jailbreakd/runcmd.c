#include <stdio.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <xpc/connection.h>
#include <libjailbreak/libjailbreak.h>
#include <CoreFoundation/CoreFoundation.h>
#include <payload/payload.h>
#include <paleinfo.h>
#include <errno.h>
#include <spawn.h>
#include <pthread.h>
#define TARGET_OS_IPHONE 1
#include <spawn_private.h>
#include <sys/spawn_internal.h>
#include <sys/kern_memorystatus.h>
#include <CoreFoundation/CoreFoundation.h>
#include <removefile.h>
#include <copyfile.h>
#include <sys/reboot.h>

#include <sys/stat.h>

void runcmd(xpc_object_t xrequest, xpc_object_t xreply, struct paleinfo* __unused pinfo) {
    const char* path = xpc_dictionary_get_string(xrequest, "path");
    if (!path) {
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        return;
    }
    
    const char** argv = NULL;
    const char** envp = NULL;
    xpc_object_t xargv = xpc_dictionary_get_array(xrequest, "argv");
    if (xargv) {
        size_t argc = xpc_array_get_count(xargv);
        argv = malloc((argc+1) * sizeof(char*));
        for (size_t i = 0; i < argc; i++) {
            argv[i] = xpc_array_get_string(xargv, i);
        }
        argv[argc] = NULL;
    }
    xpc_object_t xenvp = xpc_dictionary_get_array(xrequest, "envp");
    if (xenvp) {
        size_t envc = xpc_array_get_count(xenvp);
        envp = malloc((envc+1) * sizeof(char*));
        for (size_t i = 0; i < envc; i++) {
            envp[i] = xpc_array_get_string(xenvp, i);
        }
        envp[envc] = NULL;
    }
    
    int pipeStdout[2], pipeStderr[2];
    pipe(pipeStdout);
    pipe(pipeStderr);
    
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addclose(&actions, pipeStdout[0]);
    posix_spawn_file_actions_addclose(&actions, pipeStderr[0]);
    posix_spawn_file_actions_adddup2(&actions, pipeStdout[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipeStderr[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipeStdout[1]);
    posix_spawn_file_actions_addclose(&actions, pipeStderr[1]);
    
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED);
    
    pid_t pid;
    int retval = posix_spawn(&pid, path, &actions, &attr, (char**)argv, (char**)envp);

    free(envp);
    free(argv);
    close(pipeStdout[1]);
    close(pipeStderr[1]);
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attr);
    xpc_dictionary_set_int64(xreply, "retval", retval);

    if (retval) {
        close(pipeStdout[0]);
        close(pipeStderr[0]);
        return;
    }
    
    char stdoutDescription[PATH_MAX+50], stderrDescription[PATH_MAX+50];
    snprintf(stdoutDescription, PATH_MAX + 50, "%s stdout", path);
    snprintf(stderrDescription, PATH_MAX + 50, "%s stderr", path);
    
    struct nslog_stderr_info stdout_info = { stdoutDescription, pipeStdout[0] };
    struct nslog_stderr_info stderr_info = { stderrDescription, pipeStderr[0] };
    
    pthread_t stdout_thr, stderr_thr;
    pthread_create(&stdout_thr, NULL, write_log, &stdout_info);
    pthread_create(&stderr_thr, NULL, write_log, &stderr_info);
    
    kill(pid, SIGCONT);
    int status;
    waitpid(pid, &status, 0);
    pthread_cancel(stdout_thr);
    pthread_cancel(stderr_thr);
    close(pipeStdout[0]);
    close(pipeStderr[0]);
    pthread_join(stdout_thr, NULL);
    pthread_join(stderr_thr, NULL);
    
    xpc_dictionary_set_int64(xreply, "status", status);
    return;
}
