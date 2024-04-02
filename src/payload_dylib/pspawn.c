#include <payload_dylib/common.h>
#include <payload_dylib/crashreporter.h>
#include <libjailbreak/libjailbreak.h>
#include <spawn.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <systemhook/common.h>
#include <dlfcn.h>
#include <sys/utsname.h>
#include <substrate.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysctl.h>

bool bound_libiosexec = false;

// #define LOG_PROCESS_LAUNCHES 1

static void *posix_spawn_orig;

int (*init_libiosexec_hook_with_ellekit)(void);

static int posix_spawn_orig_wrapper(pid_t *restrict pid, const char *restrict path,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict])
{
	int (*orig)(pid_t *restrict, const char *restrict, const posix_spawn_file_actions_t *restrict, const posix_spawnattr_t *restrict, char *const[restrict], char *const[restrict]) = posix_spawn_orig;

	// we need to disable the crash reporter during the orig call
	// otherwise the child process inherits the exception ports
	// and this would trip jailbreak detections
	crashreporter_pause();	
	int r = orig(pid, path, file_actions, attrp, argv, envp);
	crashreporter_resume();

	return r;
}

static int posix_spawn_hook(pid_t *restrict pid, const char *restrict path,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict])
{
	if (path) {
		char executablePath[1024];
		uint32_t bufsize = sizeof(executablePath);
		_NSGetExecutablePath(&executablePath[0], &bufsize);
		if (!strcmp(path, executablePath)) {
			// This spawn will perform a userspace reboot...
			// Instead of the ordinary hook, we want to reinsert this dylib
			// This has already been done in envp so we only need to call the regular posix_spawn

            

#if LOG_PROCESS_LAUNCHES
			FILE *f = fopen("/cores/launch_log.txt", "a");
            if (f) {
                fprintf(f, "==== USERSPACE REBOOT ====\n");
			    fclose(f);
            }
#endif

			// Say goodbye to this process
			return posix_spawn_orig_wrapper(pid, path, file_actions, attrp, argv, envp);
		} else if ((pflags & palerain_option_rootful) == 0 && !strcmp(path, "/cores/payload")) {
            load_bootstrapped_jailbreak_env();
        }
	}

#if LOG_PROCESS_LAUNCHES
	if (path) {
		FILE *f = fopen("/cores/launch_log.txt", "a");
        if (f) {
            fprintf(f, "%s", path);
		    int ai = 0;
		    while (true) {
			    if (argv[ai]) {
				    if (ai >= 1) {
				    	fprintf(f, " %s", argv[ai]);
				    }
				    ai++;
			    }
			    else {
				    break;
			    }
		    }
		    fprintf(f, "\n");
		    fclose(f);
        }
		/*if (!strcmp(path, "/usr/libexec/xpcproxy")) {
			const char *tmpBlacklist[] = {
				"com.apple.logd"
			};
			size_t blacklistCount = sizeof(tmpBlacklist) / sizeof(tmpBlacklist[0]);
			for (size_t i = 0; i < blacklistCount; i++)
			{
				if (!strcmp(tmpBlacklist[i], firstArg)) {
					FILE *f = fopen("/var/mobile/launch_log.txt", "a");
					fprintf(f, "blocked injection %s\n", firstArg);
					fclose(f);
					int (*orig)(pid_t *restrict, const char *restrict, const posix_spawn_file_actions_t *restrict, const posix_spawnattr_t *restrict, char *const[restrict], char *const[restrict]) = posix_spawn_orig;
					return orig(pid, path, file_actions, attrp, argv, envp);
				}
			}
		}*/
	}
#endif

	return spawn_hook_common_p(pid, path, file_actions, attrp, argv, envp, posix_spawn_orig_wrapper);
}
//#define ENABLE_CONSOLE_HOOK

#ifdef ENABLE_CONSOLE_HOOK
dev_t dev_console_d = 0;

ssize_t (*write_orig)(int fildes, const void *buf, size_t nbyte);
ssize_t write_hook(int fildes, const void *buf, size_t nbyte) {
    ssize_t retval = write_orig(fildes, buf, nbyte);
    if (retval == -1) return retval;
    
    struct stat st;
    int ret = fstat(fildes, &st);
    if (retval == -1) return retval;
    if (dev_console_d && (st.st_dev == dev_console_d)) {
        int fd = open("/cores/log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd != -1) {
            write_orig(fd, buf, nbyte);
            close(fd);
        }
    }
    return retval;
}

int (*posix_spawn_file_actions_addopen_orig)(posix_spawn_file_actions_t * __restrict actions, int fd, const char * __restrict path, int oflag, mode_t mode);

int posix_spawn_file_actions_addopen_hook(posix_spawn_file_actions_t * __restrict actions, int fd, const char * __restrict path, int oflag, mode_t mode) {
    if (path) {
        if (strcmp(path, "/dev/console") == 0) {
            return posix_spawn_file_actions_addopen_orig(actions, fd, "/cores/log.txt", O_RDWR | O_CREAT | O_APPEND, 0644);
        }
    }
    
    return posix_spawn_file_actions_addopen_orig(actions, fd, path, oflag, mode);
}
#endif

void initSpawnHooks(void)
{
#ifdef ENABLE_CONSOLE_HOOK
    struct stat st;
    stat("/dev/console", &st);
    dev_console_d = st.st_dev;
   
    MSHookFunction_p(&posix_spawn_file_actions_addopen, (void *)posix_spawn_file_actions_addopen_hook, (void**)&posix_spawn_file_actions_addopen_orig);
    MSHookFunction_p(&write, (void *)write_hook, (void**)&write_orig);
#endif
	MSHookFunction_p(&posix_spawn, (void *)posix_spawn_hook, (void**)&posix_spawn_orig);
}
