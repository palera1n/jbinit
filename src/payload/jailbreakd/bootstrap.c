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
#include <spawn_private.h>
#include <sys/utsname.h>
#include <sys/spawn_internal.h>
#include <sys/kern_memorystatus.h>
#include <CoreFoundation/CoreFoundation.h>
#include <removefile.h>
#include <sys/snapshot.h>
#include <copyfile.h>
#include <sys/mount.h>

#include <sys/stat.h>

#define removefile(path, ...) do { unlink(path); rmdir(path); int ret = removefile(path, __VA_ARGS__); if (ret) { PALERA1ND_LOG_ERROR("removefile ret: %d, errno: %d", ret, errno); } } while (0)

#define BOOTSTRAP_ERROR(code, desc, ...) do { \
    if (desc) { \
        char* d;\
        asprintf(&d, desc, ##__VA_ARGS__); \
        xpc_dictionary_set_string(xreply, "errorDescription", d); \
        PALERA1ND_LOG_DEBUG("%s", d); \
        free(d); \
    } \
    if (code != -1) xpc_dictionary_set_int64(xreply, "error", code); \
    return; \
    } while (0)

#define BOOTSTRAP_ASSURE(cond, code, desc, ...) do { if (unlikely(!(cond))) BOOTSTRAP_ERROR(code, desc, ##__VA_ARGS__); } while(0)

#define BOOTSTRAP_ASSURE_CLEANUP(cond, cleanup, code, desc, ...) do { if (unlikely(!(cond))) { cleanup; BOOTSTRAP_ERROR(code, desc, ##__VA_ARGS__); } } while(0)

#define BOOTSTRAP_ASSURE_F_CLEANUP(cond, cleanup, code, desc, ...) BOOTSTRAP_ASSURE_CLEANUP(likely(!(cond)), cleanup, code, desc, ##__VA_ARGS__)

extern char** environ;

struct nslog_stderr_info {
    const char* desc;
    int fd;
};

void* write_log(void* arg) {
    int fd = ((struct nslog_stderr_info*)arg)->fd;
    ssize_t didRead;
    char buf[0x1000];
    memset(buf, 0, 0x1000);

    while ((didRead = read(fd, buf, 0x1000)) != -1) {
        if (didRead > 0) {
            PALERA1ND_LOG("%s: %s", ((struct nslog_stderr_info*)arg)->desc, buf);
            memset(buf, 0, 0x1000);
        }
    }
    return NULL;
}

void bootstrap(xpc_object_t xrequest, xpc_object_t xreply, struct paleinfo* pinfo) {
    const char* password = xpc_dictionary_get_string(xrequest, "password");
    const char* path = xpc_dictionary_get_string(xrequest, "path");
    const char* bootstrapper_name = xpc_dictionary_get_string(xrequest, "bootstrapper-name");
    const char* bootstrapper_version = xpc_dictionary_get_string(xrequest, "bootstrapper-version");
    
    bool no_password = xpc_dictionary_get_bool(xrequest, "no-password");
    
    BOOTSTRAP_ASSURE((pinfo->flags & palerain_option_force_revert) == 0, ENOTSUP, "bootstrapping in force revert is not supported");
    
    if (pinfo->flags & palerain_option_rootful) {
        BOOTSTRAP_ASSURE(access("/.procursus_strapped", F_OK) != 0, EEXIST, "Already Bootstrapped");
        struct statfs fs;
        BOOTSTRAP_ASSURE(statfs("/", &fs) == 0, errno, "statfs(/) failed");
        BOOTSTRAP_ASSURE(strstr(fs.f_mntfromname, "@/dev") == NULL, ENOTSUP, "cannot bootstrap since we rooted from snapshot %s", fs.f_mntfromname);
    } else {
        char existingPath[150] = {'\0'};
        int prebootpath_ret  = jailbreak_get_prebootPath(existingPath);
        BOOTSTRAP_ASSURE(prebootpath_ret, EEXIST, "Already Bootstrappped");
        BOOTSTRAP_ASSURE(prebootpath_ret == ENOENT, prebootpath_ret, "failed to check for existing strap");
    }
    
    BOOTSTRAP_ASSURE(path, EINVAL, "Missing bootstrap path");
    BOOTSTRAP_ASSURE((password || no_password), EINVAL, "Missing password and no-password is not specified");
    BOOTSTRAP_ASSURE(path[0] == '/', EINVAL, "path is not an absolute path");
    
    struct stat st;
    int ret = stat(path, &st);
    
    BOOTSTRAP_ASSURE(ret == 0, errno, "could not access bootstrap file");
    
    struct utsname name;
    ret = uname(&name);
    BOOTSTRAP_ASSURE(ret == 0, errno, "uname failed");
    
    int (*remount_func)(struct utsname* name_p);
    if (pinfo->flags & palerain_option_rootful) {
        remount_func = &remount_rootfs;
    } else {
        remount_func = &remount_preboot;
    }
    
    ret = remount_func(&name);
    BOOTSTRAP_ASSURE(ret == 0, errno, "remount failed");
    
    if ((pinfo->flags & palerain_option_ssv) == 0) {
        char hash[97], snapshotName[150];
        BOOTSTRAP_ASSURE(jailbreak_get_bmhash(hash) == 0, errno, "could not get boot-manifest-hash");
        snprintf(snapshotName, 150, "com.apple.os.update-%s", hash);
        int dirfd = open("/", O_RDONLY, 0);
        ret = fs_snapshot_rename(dirfd, snapshotName, "orig-fs", 0);
        if (ret != 0) {
            BOOTSTRAP_ASSURE(errno == 2, errno, "fs_snapshot_rename failed");
        }
        close(dirfd);
    }
    
    char tarPath[150];
    if (pinfo->flags & palerain_option_rootful) {
        tarPath[0] = '/';
        tarPath[1] = '\0';
    } else {
        char prebootPath[150], bmhash[97];
        BOOTSTRAP_ASSURE(jailbreak_get_bmhash(bmhash) == 0, errno, "could not get boot-manifest-hash");
        
        snprintf(prebootPath, 150, "/private/preboot/%s/jb-XXXXXXXX", bmhash);
        PALERA1ND_LOG("new jailbreak directory: %s", prebootPath);
        
        char* template = mkdtemp(prebootPath);
        BOOTSTRAP_ASSURE(template, errno, "failed to create jailbreak directory");
        
        ret = chmod(prebootPath, 0755);
        BOOTSTRAP_ASSURE(ret == 0, errno, "failed to chmod jailbreak directory");
        
        remove_bogus_var_jb();
        
        ret = lstat("/var/jb", &st);
        if (ret) {
            BOOTSTRAP_ASSURE(errno == ENOENT, errno, "lstat /var/jb failed");
        } else if (ret == 0) {
            BOOTSTRAP_ASSURE(S_ISLNK(st.st_mode), -1, "/var/jb is exists not a link after cleaning up bogus directories. Mode: 0x%" PRIx16, st.st_mode);
        }
        
        ret = unlink("/var/jb");
        BOOTSTRAP_ASSURE(ret == 0 || errno == ENOENT, errno, "failed to remove /var/jb");
        snprintf(tarPath, 150, "%s", prebootPath);
    }
    PALERA1ND_LOG_INFO("tarPath: %s", tarPath);
    
    int stream_pipefd[2], tar_stderr_pipe[2], zstd_stderr_pipe[2];
#define SMFD_CLOSE do { close(stream_pipefd[0]); close(stream_pipefd[1]); } while(0)
#define TSDE_CLOSE do { close(tar_stderr_pipe[0]); close(stream_pipefd[1]); } while(0)
#define ZSDE_CLOSE do { close(stream_pipefd[0]); close(stream_pipefd[1]); } while(0)
    
    
    BOOTSTRAP_ASSURE((ret = pipe(stream_pipefd)) == 0, errno, "pipe zstd -> tar failed");
    BOOTSTRAP_ASSURE_CLEANUP((ret = pipe(tar_stderr_pipe)) == 0, SMFD_CLOSE, errno, "pipe tar -> jailbreakd failed");
    BOOTSTRAP_ASSURE_CLEANUP((ret = pipe(zstd_stderr_pipe)) == 0, SMFD_CLOSE; TSDE_CLOSE ,errno, "pipe zstd -> jailbreakd failed");
    
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
    
    PALERA1ND_LOG("spawn tar");
    ret = posix_spawn(&tar_pid,"/cores/binpack/usr/bin/tar", &tar_actions, &tar_attr, (char*[]){ "/cores/binpack/usr/bin/tar","-C", (char*)tarPath, "--preserve-permissions", "-x" , NULL }, environ);
    BOOTSTRAP_ASSURE_CLEANUP(ret == 0, {
        SMFD_CLOSE; TSDE_CLOSE; ZSDE_CLOSE;
    }, ret,"spawn /cores/binpack/usr/bin/tar failed");
    
    PALERA1ND_LOG("spawn zstd");
    ret = posix_spawn(&zstd_pid,"/cores/binpack/usr/bin/zstd", &zstd_actions, &zstd_attr, (char*[]){ "/cores/binpack/usr/bin/zstd", "-dcT0", (char*)path, NULL }, environ);
    
    BOOTSTRAP_ASSURE_CLEANUP(ret == 0, {
        SMFD_CLOSE; TSDE_CLOSE; ZSDE_CLOSE;
        kill(tar_pid, SIGKILL);
    }, ret, "spawn /cores/binpack/usr/bin/zstd failed");
    
    posix_spawn_file_actions_destroy(&zstd_actions);
    posix_spawn_file_actions_destroy(&tar_actions);
    posix_spawnattr_destroy(&zstd_attr);
    posix_spawnattr_destroy(&tar_attr);
    
    SMFD_CLOSE;
    close(tar_stderr_pipe[1]);
    close(zstd_stderr_pipe[1]);
    
    pthread_t zstdLogThread, tarLogThread;
    struct nslog_stderr_info zstd_info = { "zstd stderr", zstd_stderr_pipe[0] };
    struct nslog_stderr_info tar_info = { "tar stderr", tar_stderr_pipe[0] };
    
#define KILL_TAR_ZSTD do { kill(tar_pid, SIGKILL); kill(zstd_pid, SIGKILL); } while(0)
    
    BOOTSTRAP_ASSURE_CLEANUP((ret = pthread_create(&zstdLogThread, NULL, write_log, &zstd_info)) == 0, KILL_TAR_ZSTD, ret, "failed to create zstd log-reading thread");
    BOOTSTRAP_ASSURE_CLEANUP((ret = pthread_create(&tarLogThread, NULL, write_log, &tar_info)) == 0, KILL_TAR_ZSTD, ret, "failed to create tar log-reading thread");
    BOOTSTRAP_ASSURE_CLEANUP(kill(tar_pid, SIGCONT) == 0, KILL_TAR_ZSTD, errno, "failed to start tar");
    BOOTSTRAP_ASSURE_CLEANUP(kill(zstd_pid, SIGCONT) == 0, KILL_TAR_ZSTD, errno, "failed to start zstd");
    
#define JOIN_TAR_ZSTD do { \
pthread_cancel(zstdLogThread); \
pthread_cancel(tarLogThread); \
close(tar_stderr_pipe[0]); \
close(zstd_stderr_pipe[0]); \
pthread_join(zstdLogThread, NULL); \
pthread_join(tarLogThread, NULL); \
} while (0)
    
    int stat_zstd, stat_tar;
    
    BOOTSTRAP_ASSURE_CLEANUP(waitpid(zstd_pid, &stat_zstd, 0) != -1, JOIN_TAR_ZSTD, errno, "failed to waitpid zstd");
    BOOTSTRAP_ASSURE_CLEANUP(waitpid(tar_pid, &stat_tar, 0) != -1, JOIN_TAR_ZSTD, errno, "failed to waitpid tar");
    
    JOIN_TAR_ZSTD;
    
#define REMOVE_TPATH do { \
if ((pinfo->flags & palerain_option_rootless) && strstr(tarPath, "/jb-")); \
removefile(tarPath, NULL, REMOVEFILE_RECURSIVE);\
} while(0)
    
    BOOTSTRAP_ASSURE_F_CLEANUP(WIFEXITED(stat_zstd) && WEXITSTATUS(stat_zstd) != 0, REMOVE_TPATH, -1, "zstd exited with status %d", WEXITSTATUS(stat_zstd));
    BOOTSTRAP_ASSURE_F_CLEANUP(WIFSIGNALED(stat_zstd), REMOVE_TPATH, -1, "zstd is terminated by signal %d", WEXITSTATUS(stat_zstd));
    BOOTSTRAP_ASSURE_F_CLEANUP(WIFEXITED(stat_tar) && WEXITSTATUS(stat_tar) != 0, REMOVE_TPATH, -1, "tar exited with status %d", WEXITSTATUS(stat_tar));
    BOOTSTRAP_ASSURE_F_CLEANUP(WIFSIGNALED(stat_tar), REMOVE_TPATH, -1, "tar is terminated by signal %d", WEXITSTATUS(stat_tar));
    
    char finalBootstrapPath[150];
    if (pinfo->flags & palerain_option_rootless) {
        char bootstrapExtractedPath[150], extrationVarPath[150];
        snprintf(bootstrapExtractedPath, 150, "%s/var/jb", tarPath);
        BOOTSTRAP_ASSURE_F_CLEANUP(access(bootstrapExtractedPath, W_OK), REMOVE_TPATH, errno, "failed to access path %s", bootstrapExtractedPath);
        
        
        snprintf(finalBootstrapPath, 150, "%s/procursus", tarPath);
        snprintf(extrationVarPath, 150, "%s/var", tarPath);
        ret = rename(bootstrapExtractedPath, finalBootstrapPath);
        PALERA1ND_LOG_INFO("rename %s -> %s", bootstrapExtractedPath, finalBootstrapPath);
        
        if (strstr(tarPath, "/jb-"))
            rmdir(extrationVarPath);
        
        BOOTSTRAP_ASSURE_F_CLEANUP(ret, REMOVE_TPATH, errno, "rename %s -> %s failed", bootstrapExtractedPath, finalBootstrapPath);
        
        ret = symlink(finalBootstrapPath, "/var/jb");
        PALERA1ND_LOG_INFO("symlink %s -> %s", "/var/jb", finalBootstrapPath);
        
        BOOTSTRAP_ASSURE_F_CLEANUP(ret, REMOVE_TPATH, errno, "symlink /var/jb -> %s failed", finalBootstrapPath);
        
#define RM_STRAP do { REMOVE_TPATH; unlink("/var/jb"); } while(0);
        
#define COPYFILE_WITH_CHECK(from, to) do { \
PALERA1ND_LOG_INFO("Copy %s -> %s", from, to); \
BOOTSTRAP_ASSURE_F_CLEANUP((ret = copyfile(from, to, NULL, 0)), RM_STRAP, errno, "failed to copy %s to %s", from, to); \
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
    
    BOOTSTRAP_ASSURE_F_CLEANUP(access(prepBootstrapPath, X_OK), RM_STRAP, errno, "failed to access path %s", prepBootstrapPath);
    
    int prepBootstrapStdErrPipe[2], prepBootstrapStdOutPipe[2];
    BOOTSTRAP_ASSURE_F_CLEANUP(pipe(prepBootstrapStdErrPipe), RM_STRAP, errno, "pipe prep_bootstrap stderr failed");
    
    BOOTSTRAP_ASSURE_F_CLEANUP(pipe(prepBootstrapStdOutPipe), {
        close(prepBootstrapStdErrPipe[0]);
        close(prepBootstrapStdErrPipe[1]);
        RM_STRAP
    }, errno, "pipe prep_bootstrap stderr failed");
    
#define CL_PSTRAP_FD do {close(prepBootstrapStdErrPipe[0]); close(prepBootstrapStdErrPipe[1]); close(prepBootstrapStdOutPipe[0]); close(prepBootstrapStdOutPipe[1]);} while(0)
    
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
    
    BOOTSTRAP_ASSURE_F_CLEANUP(pthread_create(&prepBootstrapStdErrLogThread, NULL, write_log, &prepBootstrapStdErrInfo), CL_PSTRAP_FD; RM_STRAP, ret, "failed to create prep_bootstrap stderr-reading thread");
    
    BOOTSTRAP_ASSURE_F_CLEANUP(pthread_create(&prepBootstrapStdOutLogThread, NULL, write_log, &prepBootstrapStdOutInfo),CL_PSTRAP_FD; RM_STRAP, ret, "failed to create prep_bootstrap stdout-reading thread");
    
    PALERA1ND_LOG("spawn %s %s", shellPath, prepBootstrapPath);
    ret = posix_spawn(&prepBootstrapPid, shellPath, &prepBootstrapActions, NULL, (char*[]){ shellPath, prepBootstrapPath, NULL }, (char*[]){ pathEnv, "NO_PASSWORD_PROMPT=1", NULL });
    posix_spawn_file_actions_destroy(&prepBootstrapActions);
    
    close(prepBootstrapStdOutPipe[1]);
    close(prepBootstrapStdErrPipe[1]);
    
#define PSTRAP_END_C do { \
pthread_cancel(prepBootstrapStdErrLogThread); \
pthread_cancel(prepBootstrapStdOutLogThread); \
close(prepBootstrapStdOutPipe[0]); \
close(prepBootstrapStdErrPipe[0]); \
pthread_join(prepBootstrapStdErrLogThread, NULL); \
pthread_join(prepBootstrapStdOutLogThread, NULL); \
} while (0)
    
    BOOTSTRAP_ASSURE_F_CLEANUP(ret, PSTRAP_END_C; RM_STRAP, ret, "failed to spawn prep_bootstrap.sh");
    
    int stat_prep_bootstrap;
    BOOTSTRAP_ASSURE_F_CLEANUP((ret = waitpid(prepBootstrapPid, &stat_prep_bootstrap, 0)) == -1, PSTRAP_END_C; RM_STRAP, ret, "failed to waitpid prep_bootstrap.sh");
    
    PSTRAP_END_C;
    
    BOOTSTRAP_ASSURE_F_CLEANUP(WIFEXITED(stat_prep_bootstrap) && WEXITSTATUS(stat_prep_bootstrap) != 0, RM_STRAP, -1, "prep_bootstrap exited with status %d", WEXITSTATUS(stat_prep_bootstrap));
    BOOTSTRAP_ASSURE_F_CLEANUP(WIFSIGNALED(stat_prep_bootstrap), RM_STRAP, -1, "prep_bootstrap is terminated by signal %d", WTERMSIG(stat_prep_bootstrap));
    
    
#define CL_PW_INFD do { close(pw_pipe[0]); close(pw_pipe[1]); } while(0)
#define CL_PW_OUTFD do { close(pw_stdout_pipe[0]); close(pw_pipe[1]); } while(0)
#define CL_PW_ERRFD do { close(pw_stderr_pipe[0]); close(pw_pipe[1]); } while(0)
#define CL_PWFDS do { CL_PW_INFD; CL_PW_OUTFD; CL_PW_ERRFD; } while(0)
#define PW_CLEAN do { \
    pthread_cancel(pw_stderr_thread); \
    pthread_cancel(pw_stdout_thread); \
    close(pw_stderr_pipe[0]); \
    close(pw_stdout_pipe[0]); \
    pthread_join(pw_stdout_thread, NULL); \
    pthread_join(pw_stderr_thread, NULL); \
} while(0)
    
#define SPAWN_PW(input, arg3, arg4) do { \
    pid_t pw_pid; \
    int ret; \
    int pw_pipe[2]; \
    BOOTSTRAP_ASSURE_F_CLEANUP(pipe(pw_pipe), RM_STRAP, errno, "pipe pw stdin failed"); \
    int pw_stdout_pipe[2]; \
    BOOTSTRAP_ASSURE_F_CLEANUP(pipe(pw_stdout_pipe), { \
        RM_STRAP; \
        CL_PW_INFD; \
    }, errno, "pipe pw stdout failed"); \
    int pw_stderr_pipe[2]; \
    BOOTSTRAP_ASSURE_F_CLEANUP(pipe(pw_stderr_pipe), { \
        RM_STRAP; \
        CL_PW_INFD; \
        CL_PW_OUTFD; \
    }, errno, "pipe pw stderr failed"); \
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
    BOOTSTRAP_ASSURE_F_CLEANUP((ret = pthread_create(&pw_stderr_thread, NULL, write_log, &pw_stderr_info)), CL_PWFDS; RM_STRAP, ret, "failed to create pw stderr-reading thread"); \
    BOOTSTRAP_ASSURE_F_CLEANUP((ret = pthread_create(&pw_stdout_thread, NULL, write_log, &pw_stdout_info)), { \
        pthread_cancel(pw_stderr_thread);\
        CL_PWFDS; RM_STRAP; \
        pthread_join(pw_stderr_thread, NULL); \
    }, ret, "failed to create pw stderr-reading thread"); \
    char* pwPath = (pinfo->flags & palerain_option_rootless) ? "/var/jb/usr/sbin/pw" : "/usr/sbin/pw"; \
    BOOTSTRAP_ASSURE_F_CLEANUP(access(pwPath, X_OK), { \
        pthread_cancel(pw_stderr_thread); pthread_cancel(pw_stdout_thread); \
        CL_PWFDS; \
        pthread_join(pw_stderr_thread, NULL); pthread_join(pw_stdout_thread, NULL); \
    }, errno, "failed to access %s", pwPath); \
    PALERA1ND_LOG("spawn pw"); \
    ret = posix_spawn(&pw_pid, pwPath, &pwActions, NULL, (char*[]){ "pw", "usermod", arg3, arg4, "0", NULL}, NULL); \
    posix_spawn_file_actions_destroy(&pwActions); \
    close(pw_pipe[0]); \
    close(pw_stderr_pipe[1]); \
    close(pw_stdout_pipe[1]); \
    BOOTSTRAP_ASSURE_F_CLEANUP(ret, { \
        close(pw_pipe[1]); \
        PW_CLEAN; \
        RM_STRAP; \
    }, ret, "failed to spawn pw"); \
    dprintf(pw_pipe[1], "%s\n", input); \
    close(pw_pipe[1]); \
    int stat_pw; \
    ret = waitpid(pw_pid, &stat_pw, 0); \
    BOOTSTRAP_ASSURE_F_CLEANUP(ret == -1 , { \
        PW_CLEAN; \
        RM_STRAP; \
    }, ret, "failed to waitpid pw");\
    PW_CLEAN; \
    /* XXX: pw stderr: pw: user '' disappeared during update */ \
    BOOTSTRAP_ASSURE_F_CLEANUP((WIFEXITED(stat_pw) && WEXITSTATUS(stat_pw) != 0 && WEXITSTATUS(stat_pw) != 67), RM_STRAP, -1, "pw exited with status %d", WEXITSTATUS(stat_pw)); \
    BOOTSTRAP_ASSURE_F_CLEANUP(WIFSIGNALED(stat_pw), RM_STRAP, -1, "pw is terminated by signal %d", WTERMSIG(stat_pw)); \
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
    BOOTSTRAP_ASSURE_F_CLEANUP(installed_fd == -1, RM_STRAP, errno, "failed to open %s file", installed_file);
    lseek(installed_fd, 0, SEEK_END);

    dprintf(installed_fd, "Bootstrapper-Name=%s\n", bootstrapper_name ? bootstrapper_name : "Unknown");
    dprintf(installed_fd, "Bootstrapper-Version=%s\n", bootstrapper_version ? bootstrapper_version : "Unknown");
    close(installed_fd);

    xpc_object_t msg;
    char* launch_path;
    if ((pinfo->flags & palerain_option_rootless)) {
        launch_path = "/var/jb/Library/LaunchDaemons";
    } else {
        launch_path = "/Library/LaunchDaemons";
    }

    BOOTSTRAP_ASSURE_F_CLEANUP(access(launch_path, F_OK) != 0, RM_STRAP, errno, "failed to find launch daemons path %s", launch_path);

    reload_launchd_env();

    PALERA1ND_LOG("loading %s", launch_path);
    ret = bootstrap_cmd(&msg, 3, (char*[]){ "bootstrap", "system", launch_path, NULL }, environ, (char*[]){ NULL });
    xpc_release(msg);

    PALERA1ND_LOG("bootstrap_cmd returned %d", ret);

    if (pinfo->flags & palerain_option_rootless) {
        xpc_dictionary_set_string(xreply, "bootstrapRootPath", finalBootstrapPath);
    } else {
        xpc_dictionary_set_string(xreply, "bootstrapRootPath", "/");
    }
    xpc_dictionary_set_string(xreply, "message", "Success");

    return;
}
