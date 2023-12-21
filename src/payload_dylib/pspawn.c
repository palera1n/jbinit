#include <payload_dylib/common.h>
#include <payload_dylib/crashreporter.h>
#include <libjailbreak/libjailbreak.h>
#include <spawn.h>
#include <string.h>
#include <errno.h>
#include <mach-o/dyld.h>
#include <systemhook/common.h>
#include <dlfcn.h>
#include <sys/utsname.h>
#include <substrate.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

#if 0
static bool has_fixup_prebootPath = false;

int posix_spawnp_orig_wrapper(pid_t *pid,
                              const char *path,
                              const posix_spawn_file_actions_t *action,
                              const posix_spawnattr_t *attr,
                              char *const argv[], char *envp[])
{
    int ret;
    pid_t current_pid = getpid();
    if (current_pid == 1) crashreporter_pause();
    ret = posix_spawnp(pid, path, action, attr, argv, envp);
    if (current_pid == 1) crashreporter_resume();
    return ret;
}

int hook_posix_spawnp_launchd(pid_t *pid,
                              const char *path,
                              const posix_spawn_file_actions_t *action,
                              const posix_spawnattr_t *attr,
                              char *const argv[], char *envp[])
{
    if ((pflags & palerain_option_rootless) && !strcmp(path, "/cores/payload") && !has_fixup_prebootPath) {
        struct utsname name;
        int ret = uname(&name);
        if (!ret) {
            if (atoi(name.release) < 20) {
                has_fixup_prebootPath = true;
            } else {
                char jbPath[150];
                ret = jailbreak_get_prebootPath(jbPath);
                if (!ret) {
                    has_fixup_prebootPath = true;
                    setenv("JB_ROOT_PATH", jbPath, 1);
                    void* systemhook_handle = dlopen(HOOK_DYLIB_PATH, RTLD_NOW);
                    if (systemhook_handle) {
                        char** pJB_RootPath = dlsym(systemhook_handle, "JB_RootPath");
                        if (pJB_RootPath) {
                            char* old_rootPath = *pJB_RootPath;
                            *pJB_RootPath = strdup(jbPath);
                            free(old_rootPath);
                        }
                        dlclose(systemhook_handle);
                    }
                }
            }
        }
    }
    if (spawn_hook_common_p) {
        return resolvePath(path, NULL, ^int(char *file) {
		    return spawn_hook_common_p(pid, file, action, attr, argv, envp, posix_spawnp_orig_wrapper);
	    });
    } else return posix_spawnp_orig_wrapper(pid, path, action, attr, argv, envp);
}

int hook_posix_spawn_launchd(pid_t *pid,
                              const char *path,
                              const posix_spawn_file_actions_t *action,
                              const posix_spawnattr_t *attr,
                              char *const argv[], char *envp[]) {

                              }

DYLD_INTERPOSE(hook_posix_spawnp_launchd, posix_spawnp);
DYLD_INTERPOSE(hook_posix_spawn_launchd, posix_spawn);
#endif

static bool has_fixup_prebootPath = false, bound_libiosexec = false;
// #define LOG_PROCESS_LAUNCHES 1

void *posix_spawn_orig;

int (*init_libiosexec_hook_with_ellekit)(void);

int posix_spawn_orig_wrapper(pid_t *restrict pid, const char *restrict path,
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

int posix_spawn_hook(pid_t *restrict pid, const char *restrict path,
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
		} else if (
            (pflags & palerain_option_rootless) 
            && !has_fixup_prebootPath
            && !strcmp(path, "/cores/payload")) {
                struct utsname name;
                int ret = uname(&name);
                if (!ret) {
                if (atoi(name.release) < 20) {
                    has_fixup_prebootPath = true;
                } else {
                    char jbPath[150];
                    ret = jailbreak_get_prebootPath(jbPath);
                    if (!ret) {
                        setenv("JB_ROOT_PATH", jbPath, 1);
                        void* systemhook_handle = dlopen(HOOK_DYLIB_PATH, RTLD_NOW);
                        if (systemhook_handle) {
                            char** pJB_RootPath = dlsym(systemhook_handle, "JB_RootPath");
                            if (pJB_RootPath) {
                                char* old_rootPath = *pJB_RootPath;
                                *pJB_RootPath = strdup(jbPath);
                                free(old_rootPath);
                                has_fixup_prebootPath = true;
                            }
#ifdef SYSTEMWIDE_IOSEXEC
                            init_libiosexec_hook_with_ellekit = dlsym(systemhook_handle, "init_libiosexec_hook_with_ellekit");
                            if (init_libiosexec_hook_with_ellekit) {
                                if (!init_libiosexec_hook_with_ellekit()) bound_libiosexec = true;
                            }
#endif
                            dlclose(systemhook_handle);
                        }
                    }
                }
            }
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

void initSpawnHooks(void)
{
	MSHookFunction_p(&posix_spawn, (void *)posix_spawn_hook, &posix_spawn_orig);
}
