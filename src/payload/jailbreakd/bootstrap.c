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
#include <sys/utsname.h>
#include <sys/spawn_internal.h>
#include <sys/kern_memorystatus.h>
#include <CoreFoundation/CoreFoundation.h>
#include <removefile.h>
#include <copyfile.h>

#include <sys/stat.h>

#define removefile(path, ...) do { unlink(path); rmdir(path); int ret = removefile(path, __VA_ARGS__); if (ret) { NSLog(CFSTR("removefile ret: %d, errno: %d"), ret, errno); } } while (0)

extern char** environ;

struct nslog_stderr_info {
    const char* desc;
    int fd;
};

void NSLog(CFStringRef, ...);

void* write_log(void* arg) {
    int fd = ((struct nslog_stderr_info*)arg)->fd;
    ssize_t didRead;
    char buf[0x1000];
    memset(buf, 0, 0x1000);

    while ((didRead = read(fd, buf, 0x1000)) != -1) {
        if (didRead > 0) {
            NSLog(CFSTR("%s: %s"), ((struct nslog_stderr_info*)arg)->desc, buf);
            memset(buf, 0, 0x1000);   
        }
    }
    return NULL;
}

void bootstrap(xpc_object_t xrequest, xpc_object_t xreply, struct paleinfo* pinfo) {
    const char* password = xpc_dictionary_get_string(xrequest, "password");
    const char* path = xpc_dictionary_get_string(xrequest, "path");
    bool no_password = xpc_dictionary_get_bool(xrequest, "no-password");

    if (pinfo->flags & palerain_option_force_revert) {
        xpc_dictionary_set_string(xreply, "errorDescription", "bootstrapping in force revert is not supported");
        xpc_dictionary_set_int64(xreply, "error", ENOTSUP);
        return;
    }

    if (pinfo->flags & palerain_option_rootful) {
        if (access("/.procursus_strapped", F_OK) == 0) {
            xpc_dictionary_set_string(xreply, "errorDescription", "Already Btrapped");
            xpc_dictionary_set_int64(xreply, "error", EEXIST);
            return;
        }
    } else {
        int prebootpath_ret;
        char existingPath[150] = {'\0'};
        if ((prebootpath_ret = jailbreak_get_prebootPath(existingPath)) != ENOENT) {
            if (prebootpath_ret == 0) {
                xpc_dictionary_set_string(xreply, "errorDescription", "Already Bootstrapped");
                xpc_dictionary_set_int64(xreply, "error", EEXIST);
            } else {
                xpc_dictionary_set_string(xreply, "errorDescription", "failed to check for existing strap");
                xpc_dictionary_set_int64(xreply, "error", prebootpath_ret);
            }
            return;
        }
    }

    if (!path) {
        xpc_dictionary_set_string(xreply, "errorDescription", "Missing bootstrap path");
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        return;
    }

    if (!password && !no_password) {
        xpc_dictionary_set_string(xreply, "errorDescription", "Missing password and no-password is not specified");
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        return;
    }

    if (path[0] != '/') {
        xpc_dictionary_set_string(xreply, "errorDescription", "path is not an absolute path");
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        return;
    }

    struct stat st;
    int ret = stat(path, &st);
    
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "could not access bootstrap file");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    struct utsname name;
    ret = uname(&name);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "remount failed");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    int (*remount_func)(struct utsname* name_p);
    if (pinfo->flags & palerain_option_rootful) {
        remount_func = &remount_rootfs;
    } else {
        remount_func = &remount_preboot;
    }

    ret = remount_func(&name);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "remount failed");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    char tarPath[150];
    if (pinfo->flags & palerain_option_rootful) {
        tarPath[0] = '/';
        tarPath[1] = '\0';
    } else {
        char prebootPath[150], bmhash[97];
        if (jailbreak_get_bmhash(bmhash)) {
            xpc_dictionary_set_string(xreply, "errorDescription", "could not get boot-manifest-hash");
            xpc_dictionary_set_int64(xreply, "error", errno);
            return;
        }
        snprintf(prebootPath, 150, "/private/preboot/%s/jb-XXXXXXXX", bmhash);
        NSLog(CFSTR("new jailbreak directory: %s"), prebootPath);

        char* template = mkdtemp(prebootPath);
        if (!template) {
            xpc_dictionary_set_string(xreply, "errorDescription", "failed to create jailbreak directory");
            xpc_dictionary_set_int64(xreply, "error", errno);
        }

        ret = chmod(prebootPath, 0755);
        if (ret) {
            xpc_dictionary_set_string(xreply, "errorDescription", "failed to chmod jailbreak directory");
            xpc_dictionary_set_int64(xreply, "error", errno);
        }

        ret = unlink("/var/jb");
        if (ret && errno != ENOENT) {
            xpc_dictionary_set_string(xreply, "errorDescription", "failed to remove /var/jb");
            xpc_dictionary_set_int64(xreply, "error", errno);
        }
        snprintf(tarPath, 150, "%s", prebootPath);
    }
    NSLog(CFSTR("tarPath: %s"), tarPath);

    int stream_pipefd[2], tar_stderr_pipe[2], zstd_stderr_pipe[2];
    ret = pipe(stream_pipefd);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe zstd -> tar failed");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    ret = pipe(tar_stderr_pipe);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe tar -> jailbreakd failed");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    ret = pipe(zstd_stderr_pipe);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe zstd -> jailbreakd failed");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    posix_spawn_file_actions_t tar_actions, zstd_actions;

    posix_spawn_file_actions_init(&tar_actions);
    posix_spawn_file_actions_init(&zstd_actions);

    posix_spawn_file_actions_addclose(&tar_actions, stream_pipefd[1]);
    posix_spawn_file_actions_addclose(&tar_actions, tar_stderr_pipe[0]);
    posix_spawn_file_actions_addclose(&tar_actions, zstd_stderr_pipe[0]);
    posix_spawn_file_actions_addclose(&tar_actions, zstd_stderr_pipe[1]);

    posix_spawn_file_actions_addclose(&zstd_actions, stream_pipefd[0]);
    posix_spawn_file_actions_addclose(&zstd_actions, zstd_stderr_pipe[0]);
    posix_spawn_file_actions_addclose(&zstd_actions, tar_stderr_pipe[0]);
    posix_spawn_file_actions_addclose(&zstd_actions, tar_stderr_pipe[1]);

    posix_spawn_file_actions_adddup2(&tar_actions, stream_pipefd[0], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&zstd_actions, stream_pipefd[1], STDOUT_FILENO);

    posix_spawn_file_actions_addclose(&tar_actions, stream_pipefd[0]);
    posix_spawn_file_actions_addclose(&zstd_actions, stream_pipefd[1]);

    posix_spawnattr_t tar_attr, zstd_attr;

    posix_spawnattr_init(&tar_attr);
    posix_spawnattr_init(&zstd_attr);

    posix_spawnattr_setprocesstype_np(&tar_attr, POSIX_SPAWN_PROCESS_TYPE_DEFAULT);
    posix_spawnattr_setprocesstype_np(&zstd_attr, POSIX_SPAWN_PROCESS_TYPE_DEFAULT);

    posix_spawnattr_setjetsam_ext(&tar_attr, POSIX_SPAWN_JETSAM_USE_EFFECTIVE_PRIORITY, JETSAM_PRIORITY_FOREGROUND, 192, 192);
    posix_spawnattr_setjetsam_ext(&zstd_attr, POSIX_SPAWN_JETSAM_USE_EFFECTIVE_PRIORITY, JETSAM_PRIORITY_FOREGROUND, 512, 512);

    posix_spawnattr_setflags(&tar_attr, POSIX_SPAWN_START_SUSPENDED);
    posix_spawnattr_setflags(&zstd_attr, POSIX_SPAWN_START_SUSPENDED);

    pid_t tar_pid, zstd_pid;

    NSLog(CFSTR("spawn tar"));
    ret = posix_spawn(&tar_pid,"/cores/binpack/usr/bin/tar", &tar_actions, &tar_attr, (char*[]){ "/cores/binpack/usr/bin/tar","-C", (char*)tarPath, "--preserve-permissions", "-x" , NULL }, environ);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "spawn /cores/binpack/usr/bin/tar failed");
        xpc_dictionary_set_int64(xreply, "error", ret);
        return;
    }

    NSLog(CFSTR("spawn zstd"));
    ret = posix_spawn(&zstd_pid,"/cores/binpack/usr/bin/zstd", &zstd_actions, &zstd_attr, (char*[]){ "/cores/binpack/usr/bin/zstd", "-dcT0", (char*)path, NULL }, environ);
    if (ret) {
        kill(tar_pid, SIGKILL);
        xpc_dictionary_set_string(xreply, "errorDescription", "spawn /cores/binpack/usr/bin/zstd failed");
        xpc_dictionary_set_int64(xreply, "error", ret);
        return;
    }

    posix_spawn_file_actions_destroy(&zstd_actions);
    posix_spawn_file_actions_destroy(&tar_actions);
    posix_spawnattr_destroy(&zstd_attr);
    posix_spawnattr_destroy(&tar_attr);

    close(stream_pipefd[0]);
    close(stream_pipefd[1]);
    close(tar_stderr_pipe[1]);
    close(zstd_stderr_pipe[1]);

    pthread_t zstdLogThread, tarLogThread;
    struct nslog_stderr_info zstd_info = { "zstd stderr", zstd_stderr_pipe[0] };
    struct nslog_stderr_info tar_info = { "tar stderr", tar_stderr_pipe[0] };

    ret = pthread_create(&zstdLogThread, NULL, write_log, &zstd_info);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to create zstd log-reading thread");
        xpc_dictionary_set_int64(xreply, "error", ret);
        return;
    }

    ret = pthread_create(&tarLogThread, NULL, write_log, &tar_info);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to create tar log-reading thread");
        xpc_dictionary_set_int64(xreply, "error", ret);
        return;
    }

    ret = kill(tar_pid, SIGCONT);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to start tar");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    ret = kill(zstd_pid, SIGCONT);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to start zstd");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    int stat_zstd, stat_tar;
    ret = waitpid(zstd_pid, &stat_zstd, 0);
    if (ret == -1) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to waitpid zstd");
        xpc_dictionary_set_int64(xreply, "error", errno);
        pthread_cancel(zstdLogThread);
        pthread_cancel(tarLogThread);
        close(tar_stderr_pipe[0]);
        close(zstd_stderr_pipe[0]);
        pthread_join(zstdLogThread, NULL);
        pthread_join(tarLogThread, NULL);
        return;
    }

    ret = waitpid(tar_pid, &stat_tar, 0);
    if (ret == -1) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to waitpid tar");
        xpc_dictionary_set_int64(xreply, "error", errno);
        pthread_cancel(zstdLogThread);
        pthread_cancel(tarLogThread);
        close(tar_stderr_pipe[0]);
        close(zstd_stderr_pipe[0]);
        pthread_join(zstdLogThread, NULL);
        pthread_join(tarLogThread, NULL);
        return;
    }

    pthread_cancel(zstdLogThread);
    pthread_cancel(tarLogThread);

    close(tar_stderr_pipe[0]);
    close(zstd_stderr_pipe[0]);

    pthread_join(zstdLogThread, NULL);
    pthread_join(tarLogThread, NULL);

    if (WIFEXITED(stat_zstd) && WEXITSTATUS(stat_zstd) != 0) {
        char descriptionStr[40];
        snprintf(descriptionStr, 40, "zstd exited with status %d", WEXITSTATUS(stat_zstd));
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) 
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
        return;
    } else if (WIFSIGNALED(stat_zstd)) {
        char descriptionStr[40];
        snprintf(descriptionStr, 40, "zstd is terminated by signal %d", WEXITSTATUS(stat_zstd));
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) 
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
        return;
    }

    if (WIFEXITED(stat_tar) && WEXITSTATUS(stat_tar) != 0) {
        char descriptionStr[40];
        snprintf(descriptionStr, 40, "tar exited with status %d", WEXITSTATUS(stat_tar));
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) 
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
        return;
    } else if (WIFSIGNALED(stat_tar)) {
        char descriptionStr[40];
        snprintf(descriptionStr, 40, "tar is terminated by signal %d", WEXITSTATUS(stat_tar));
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) 
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
        return;
    }
    
    char finalBootstrapPath[150];
    if (pinfo->flags & palerain_option_rootless) {
        char bootstrapExtractedPath[150], extrationVarPath[150];
        snprintf(bootstrapExtractedPath, 150, "%s/var/jb", tarPath);
        if (access(bootstrapExtractedPath, W_OK) != 0) {
            char descriptionStr[200];
            snprintf(descriptionStr, 200, "failed to access path %s", bootstrapExtractedPath);
            xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
            xpc_dictionary_set_int64(xreply, "error", errno);
            if (strlen(tarPath) > 118) 
                removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            return;
        }

        snprintf(finalBootstrapPath, 150, "%s/procursus", tarPath);
        snprintf(extrationVarPath, 150, "%s/var", tarPath);
        ret = rename(bootstrapExtractedPath, finalBootstrapPath);
        NSLog(CFSTR("rename %s -> %s"), bootstrapExtractedPath, finalBootstrapPath);

        if (strlen(tarPath) > 118) 
            rmdir(extrationVarPath);

        if (ret) {
            char descriptionStr[400];
            snprintf(descriptionStr, 200, "rename %s -> %s failed", bootstrapExtractedPath, finalBootstrapPath);
            xpc_dictionary_set_int64(xreply, "error", errno);

            if (strlen(tarPath) > 118) 
                removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            return;
        }

        ret = symlink(finalBootstrapPath, "/var/jb");
        NSLog(CFSTR("symlink %s -> %s"), "/var/jb", finalBootstrapPath);

        if (ret) {
            char descriptionStr[400];
            snprintf(descriptionStr, 200, "symlink /var/jb -> %s failed", finalBootstrapPath);
            xpc_dictionary_set_int64(xreply, "error", errno);
            if (strlen(tarPath) > 118) removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            return;
        }
#define COPYFILE_WITH_CHECK(from, to) do { \
    NSLog(CFSTR("Copy %s -> %s"), from, to); \
    int ret = copyfile(from, to, NULL, 0); \
    if (ret) { \
        char descriptionStr[300]; \
        snprintf(descriptionStr, 200, "failed to copy %s to %s", from, to); \
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr); \
        xpc_dictionary_set_int64(xreply, "error", errno); \
        if (strlen(tarPath) > 118) removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
        unlink("/var/jb"); \
        return; \
    } \
    } while(0)
        COPYFILE_WITH_CHECK("/etc/passwd", "/var/jb/etc/passwd");
        COPYFILE_WITH_CHECK("/etc/master.passwd", "/var/jb/etc/master.passwd");
        COPYFILE_WITH_CHECK("/etc/group", "/var/jb/etc/group");
    }

    char *prepBootstrapPath, *shellPath, *pathEnv;
    if (pinfo->flags & palerain_option_rootless) {
        prepBootstrapPath = "/var/jb/prep_bootstrap.sh";
        shellPath = "/var/jb/bin/sh";
        pathEnv = "PATH=/var/jb/usr/bin:/var/jb/usr/sbin:/var/jb/bin:/var/jb/sbin:/var/jb/usr/bin/X11:/var/jb/usr/games";
    } else {
        prepBootstrapPath = "/prep_bootstrap.sh";
        shellPath = "/bin/sh";
        pathEnv = "PATH=/usr/bin:/usr/sbin:/bin:/sbin:/usr/bin/X11:/usr/games";
    }

    if (access(prepBootstrapPath, X_OK) != 0) {
        char descriptionStr[200];
        snprintf(descriptionStr, 200, "failed to access path %s", prepBootstrapPath);
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        xpc_dictionary_set_int64(xreply, "error", errno);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

    int prepBootstrapStdErrPipe[2], prepBootstrapStdOutPipe[2];
    ret = pipe(prepBootstrapStdErrPipe);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe prep_bootstrap stderr failed");
        xpc_dictionary_set_int64(xreply, "error", errno);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

    ret = pipe(prepBootstrapStdOutPipe);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe prep_bootstrap stdout failed");
        xpc_dictionary_set_int64(xreply, "error", errno);
        close(prepBootstrapStdErrPipe[0]);
        close(prepBootstrapStdOutPipe[0]);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

    pid_t prepBootstrapPid;
    posix_spawn_file_actions_t prepBootstrapActions;
    posix_spawn_file_actions_init(&prepBootstrapActions);

    pthread_t prepBootstrapStdErrLogThread, prepBootstrapStdOutLogThread;
    struct nslog_stderr_info prepBootstrapStdOutInfo = { "prep_bootstrap stdout", prepBootstrapStdErrPipe[0] };
    struct nslog_stderr_info prepBootstrapStdErrInfo = { "prep_bootstrap stderr", prepBootstrapStdOutPipe[0] };

    posix_spawn_file_actions_addclose(&prepBootstrapActions, prepBootstrapStdErrPipe[0]);
    posix_spawn_file_actions_addclose(&prepBootstrapActions, prepBootstrapStdOutPipe[0]);
    posix_spawn_file_actions_adddup2(&prepBootstrapActions, prepBootstrapStdErrPipe[1], STDERR_FILENO);
    posix_spawn_file_actions_adddup2(&prepBootstrapActions, prepBootstrapStdOutPipe[1], STDERR_FILENO);

    posix_spawn_file_actions_addclose(&prepBootstrapActions, prepBootstrapStdErrPipe[1]);
    posix_spawn_file_actions_addclose(&prepBootstrapActions, prepBootstrapStdOutPipe[1]);

    ret = pthread_create(&prepBootstrapStdErrLogThread, NULL, write_log, &prepBootstrapStdErrInfo);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to create prep_bootstrap stderr-reading thread");
        xpc_dictionary_set_int64(xreply, "error", ret);
        if (pinfo->flags & palerain_option_rootless) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

    ret = pthread_create(&prepBootstrapStdOutLogThread, NULL, write_log, &prepBootstrapStdOutInfo);
    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to create prep_bootstrap stdout-reading thread");
        xpc_dictionary_set_int64(xreply, "error", ret);
        pthread_cancel(prepBootstrapStdErrLogThread);
        close(prepBootstrapStdOutPipe[0]);
        close(prepBootstrapStdErrPipe[0]);
        pthread_join(prepBootstrapStdErrLogThread, NULL);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

    NSLog(CFSTR("spawn %s %s"), shellPath, prepBootstrapPath);
    ret = posix_spawn(&prepBootstrapPid, shellPath, &prepBootstrapActions, NULL, (char*[]){ shellPath, prepBootstrapPath, NULL }, (char*[]){ pathEnv, "NO_PASSWORD_PROMPT=1", NULL });
    posix_spawn_file_actions_destroy(&prepBootstrapActions);
    
    close(prepBootstrapStdOutPipe[1]);
    close(prepBootstrapStdErrPipe[1]);

    if (ret) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to spawn prep_bootstrap.sh");
        xpc_dictionary_set_int64(xreply, "error", ret);
        pthread_cancel(prepBootstrapStdErrLogThread);
        pthread_cancel(prepBootstrapStdOutLogThread);
        close(prepBootstrapStdOutPipe[0]);
        close(prepBootstrapStdErrPipe[0]);
        pthread_join(prepBootstrapStdErrLogThread, NULL);
        pthread_join(prepBootstrapStdOutLogThread, NULL);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

    int stat_prep_bootstrap;
    ret = waitpid(prepBootstrapPid, &stat_prep_bootstrap, 0);

    if (ret == -1) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to waitpid prep_bootstrap.sh");
        xpc_dictionary_set_int64(xreply, "error", errno);
        pthread_cancel(prepBootstrapStdErrLogThread);
        pthread_cancel(prepBootstrapStdOutLogThread);
        close(prepBootstrapStdOutPipe[0]);
        close(prepBootstrapStdErrPipe[0]);
        pthread_join(prepBootstrapStdErrLogThread, NULL);
        pthread_join(prepBootstrapStdOutLogThread, NULL);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

    pthread_cancel(prepBootstrapStdErrLogThread);
    pthread_cancel(prepBootstrapStdOutLogThread);
    close(prepBootstrapStdOutPipe[0]);
    close(prepBootstrapStdErrPipe[0]);
    pthread_join(prepBootstrapStdErrLogThread, NULL);
    pthread_join(prepBootstrapStdOutLogThread, NULL);
    
    if (WIFEXITED(stat_prep_bootstrap) && WEXITSTATUS(stat_prep_bootstrap) != 0) {
        char descriptionStr[40];
        snprintf(descriptionStr, 40, "prep_bootstrap exited with status %d", WEXITSTATUS(stat_prep_bootstrap));
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    } else if (WIFSIGNALED(stat_prep_bootstrap)) {
        char descriptionStr[40];
        snprintf(descriptionStr, 40, "prep_bootstrap is terminated by signal %d", WEXITSTATUS(stat_prep_bootstrap));
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) {
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);
            unlink("/var/jb");
        }
        return;
    }

#define SPAWN_PW(input, arg3, arg4) do { \
    pid_t pw_pid; \
    int ret; \
    int pw_pipe[2]; \
    ret = pipe(pw_pipe); \
    if (ret) { \
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe pw stdin failed"); \
        xpc_dictionary_set_int64(xreply, "error", errno); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(finalBootstrapPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    int pw_stdout_pipe[2]; \
    ret = pipe(pw_stdout_pipe); \
    if (ret) { \
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe pw stdout failed"); \
        xpc_dictionary_set_int64(xreply, "error", errno); \
        close(pw_pipe[0]); \
        close(pw_pipe[1]); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(finalBootstrapPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    int pw_stderr_pipe[2]; \
    ret = pipe(pw_stderr_pipe); \
    if (ret) { \
        xpc_dictionary_set_string(xreply, "errorDescription", "pipe pw stderr failed"); \
        xpc_dictionary_set_int64(xreply, "error", errno); \
        close(pw_stdout_pipe[0]); \
        close(pw_stdout_pipe[1]); \
        close(pw_pipe[0]); \
        close(pw_pipe[1]); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(finalBootstrapPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    posix_spawn_file_actions_t pwActions; \
    posix_spawn_file_actions_init(&pwActions); \
    posix_spawn_file_actions_adddup2(&pwActions, pw_pipe[0], STDIN_FILENO); \
    posix_spawn_file_actions_adddup2(&pwActions, pw_stdout_pipe[1], STDOUT_FILENO); \
    posix_spawn_file_actions_adddup2(&pwActions, pw_stderr_pipe[1], STDERR_FILENO); \
    posix_spawn_file_actions_addclose(&pwActions, pw_pipe[1]); \
    posix_spawn_file_actions_addclose(&pwActions, pw_stdout_pipe[1]); \
    posix_spawn_file_actions_addclose(&pwActions, pw_stderr_pipe[1]); \
    posix_spawn_file_actions_addclose(&pwActions, pw_pipe[0]); \
    posix_spawn_file_actions_addclose(&pwActions, pw_stdout_pipe[0]); \
    posix_spawn_file_actions_addclose(&pwActions, pw_stderr_pipe[0]); \
    pthread_t pw_stdout_thread, pw_stderr_thread; \
    struct nslog_stderr_info pw_stdout_info = { "pw stdout", pw_stdout_pipe[0] }; \
    struct nslog_stderr_info pw_stderr_info = { "pw stderr", pw_stderr_pipe[0] }; \
    ret = pthread_create(&pw_stderr_thread, NULL, write_log, &pw_stderr_info); \
    if (ret) { \
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to create pw stderr-reading thread"); \
        xpc_dictionary_set_int64(xreply, "error", ret); \
        close(pw_stdout_pipe[0]); \
        close(pw_stderr_pipe[0]); \
        close(pw_stdout_pipe[1]); \
        close(pw_stderr_pipe[1]); \
        close(pw_pipe[0]); \
        close(pw_pipe[1]); \
        if (pinfo->flags & palerain_option_rootless) { \
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    ret = pthread_create(&pw_stdout_thread, NULL, write_log, &pw_stdout_info); \
    if (ret) { \
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to create pw stdout-reading thread"); \
        xpc_dictionary_set_int64(xreply, "error", ret); \
        pthread_cancel(pw_stderr_thread); \
        close(pw_stdout_pipe[0]); \
        close(pw_stderr_pipe[0]); \
        close(pw_stdout_pipe[1]); \
        close(pw_stderr_pipe[1]); \
        close(pw_pipe[0]); \
        close(pw_pipe[1]); \
        pthread_join(pw_stderr_thread, NULL); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    char* pwPath = (pinfo->flags & palerain_option_rootless) ? "/var/jb/usr/sbin/pw" : "/usr/sbin/pw"; \
    if (access(pwPath, X_OK) != 0) { \
        char descriptionStr[120];\
        snprintf(descriptionStr, 120, "failed to access %s", pwPath); \
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr); \
        pthread_cancel(pw_stderr_thread); \
        pthread_cancel(pw_stdout_thread); \
        close(pw_stdout_pipe[0]); \
        close(pw_stderr_pipe[0]); \
        close(pw_stdout_pipe[1]); \
        close(pw_stderr_pipe[1]); \
        close(pw_pipe[0]); \
        close(pw_pipe[1]); \
        pthread_join(pw_stderr_thread, NULL); \
        pthread_join(pw_stdout_thread, NULL); \
        return; \
    } \
    NSLog(CFSTR("spawn pw")); \
    ret = posix_spawn(&pw_pid, pwPath, &pwActions, NULL, (char*[]){ "pw", "usermod", arg3, arg4, "0", NULL}, NULL); \
    posix_spawn_file_actions_destroy(&pwActions); \
    close(pw_pipe[0]); \
    close(pw_stderr_pipe[1]); \
    close(pw_stdout_pipe[1]); \
    if (ret) { \
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to spawn pw"); \
        xpc_dictionary_set_int64(xreply, "error", ret); \
        pthread_cancel(pw_stderr_thread); \
        pthread_cancel(pw_stdout_thread); \
        close(pw_pipe[1]); \
        close(pw_stderr_pipe[0]); \
        close(pw_stdout_pipe[0]); \
        pthread_join(pw_stdout_thread, NULL); \
        pthread_join(pw_stderr_thread, NULL); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    dprintf(pw_pipe[1], "%s\n", input); \
    close(pw_pipe[1]); \
    int stat_pw; \
    ret = waitpid(pw_pid, &stat_pw, 0); \
    if (ret == -1) { \
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to waitpid pw"); \
        xpc_dictionary_set_int64(xreply, "error", ret); \
        pthread_cancel(pw_stderr_thread); \
        pthread_cancel(pw_stdout_thread); \
        close(pw_stderr_pipe[0]); \
        close(pw_stdout_pipe[0]); \
        pthread_join(pw_stdout_thread, NULL); \
        pthread_join(pw_stderr_thread, NULL); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    pthread_cancel(pw_stderr_thread); \
    pthread_cancel(pw_stdout_thread); \
    close(pw_stderr_pipe[0]); \
    close(pw_stdout_pipe[0]); \
    pthread_join(pw_stdout_thread, NULL); \
    pthread_join(pw_stderr_thread, NULL); \
    /* XXX: pw stderr: pw: user '' disappeared during update */ \
    if (WIFEXITED(stat_pw) && WEXITSTATUS(stat_pw) != 0 && WEXITSTATUS(stat_pw) != 67) { \
        char descriptionStr[40]; \
        snprintf(descriptionStr, 40, "pw exited with status %d", WEXITSTATUS(stat_pw)); \
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } else if (WIFSIGNALED(stat_pw)) { \
        char descriptionStr[40]; \
        snprintf(descriptionStr, 40, "pw is terminated by signal %d", WEXITSTATUS(stat_pw)); \
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr); \
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb"); \
        } \
        return; \
    } \
    } while(0)

    if (no_password) {
        SPAWN_PW("!", "501", "-H");
    } else {
        SPAWN_PW(password, "501", "-h");
    }
    SPAWN_PW("!", "0", "-H");

    const char* installed_file;
    int installed_fd = -1;
    if ((pinfo->flags & palerain_option_rootless)) 
        installed_file = "/var/jb/.installed_palera1n";
    else
        installed_file = "/.installed_palera1n";

    installed_fd = open(installed_file, O_RDWR | O_CREAT);
    if (installed_fd == -1) {
        char descriptionStr[100];
        snprintf(descriptionStr, 100, "failed to open %s file", installed_file);
        xpc_dictionary_set_string(xreply, "errorDescription", descriptionStr);
        xpc_dictionary_set_int64(xreply, "error", errno);
        if ((pinfo->flags & palerain_option_rootless) && strlen(tarPath) > 118) { \
            removefile(tarPath, NULL, REMOVEFILE_RECURSIVE); \
            unlink("/var/jb");
        }
        return;
    }
    lseek(installed_fd, 0, SEEK_END);

    dprintf(installed_fd, "Bootstrapper-Name=p1ctl\n");
    dprintf(installed_fd, "Bootstrapper-Version=3.0\n");
    close(installed_fd);

    xpc_object_t msg;
    char* launch_path;
    if ((pinfo->flags & palerain_option_rootless)) {
        launch_path = "/var/jb/Library/LaunchDaemons";
    } else {
        launch_path = "/Library/LaunchDaemons";
    }

    NSLog(CFSTR("loading %s"), launch_path);
    ret = bootstrap_cmd(&msg, 3, (char*[]){ "bootstrap", "system", launch_path, NULL }, environ, (char*[]){ NULL });

    NSLog(CFSTR("bootstrap_cmd returned %d"), ret);

    if (pinfo->flags & palerain_option_rootless) {
        xpc_dictionary_set_string(xreply, "bootstrapRootPath", finalBootstrapPath);
    } else {
        xpc_dictionary_set_string(xreply, "bootstrapRootPath", "/");
    }
    xpc_dictionary_set_string(xreply, "message", "Success");

    return;
}
