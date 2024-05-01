#include <systemhook/common.h>
#include <systemhook/libiosexec.h>
#include <libjailbreak/libjailbreak.h>

#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <paths.h>
#include <sandbox.h>
#include <sys/utsname.h>
#include <sandbox/private.h>
#include <termios.h>
#include <util.h>
#include <uuid/uuid.h>

extern char **environ;

int ptrace(int request, pid_t pid, caddr_t addr, int data);
#define PT_ATTACH       10      /* trace some running process */
#define PT_ATTACHEXC    14      /* attach to running process with signal exception */

void* dlopen_from(const char* path, int mode, void* addressInCaller);
void* dlopen_audited(const char* path, int mode);
bool dlopen_preflight(const char* path);

#define DYLD_INTERPOSE(_replacement,_replacee) \
   __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee \
			__attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

static bool in_jailbreakd;

void unsandbox(void) {
	char extensionsCopy[strlen(JB_SandboxExtensions)];
	strcpy(extensionsCopy, JB_SandboxExtensions);
	char *extensionToken = strtok(extensionsCopy, "|");
	while (extensionToken != NULL) {
		sandbox_extension_consume(extensionToken);
		extensionToken = strtok(NULL, "|");
	}
}

static char *gExecutablePath = NULL;
SHOOK_EXPORT char* JB_TweakLoaderPath;
static void loadExecutablePath(void)
{
	uint32_t bufsize = 0;
	_NSGetExecutablePath(NULL, &bufsize);
	char *executablePath = malloc(bufsize);
	_NSGetExecutablePath(executablePath, &bufsize);
	if (executablePath) {
		gExecutablePath = realpath(executablePath, NULL);
		free(executablePath);
	}
}
static void freeExecutablePath(void)
{
	if (gExecutablePath) {
		free(gExecutablePath);
		gExecutablePath = NULL;
	}
}

void killall(const char *executablePathToKill, bool softly)
{
	static int maxArgumentSize = 0;
	if (maxArgumentSize == 0) {
		size_t size = sizeof(maxArgumentSize);
		if (sysctl((int[]){ CTL_KERN, KERN_ARGMAX }, 2, &maxArgumentSize, &size, NULL, 0) == -1) {
			perror("sysctl argument size");
			maxArgumentSize = 4096; // Default
		}
	}
	int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL};
	struct kinfo_proc *info;
	size_t length;
	int count;
	
	if (sysctl(mib, 3, NULL, &length, NULL, 0) < 0)
		return;
	if (!(info = malloc(length)))
		return;
	if (sysctl(mib, 3, info, &length, NULL, 0) < 0) {
		free(info);
		return;
	}
	count = length / sizeof(struct kinfo_proc);
	for (int i = 0; i < count; i++) {
		pid_t pid = info[i].kp_proc.p_pid;
		if (pid == 0) {
			continue;
		}
		size_t size = maxArgumentSize;
		char* buffer = (char *)malloc(length);
		if (sysctl((int[]){ CTL_KERN, KERN_PROCARGS2, pid }, 3, buffer, &size, NULL, 0) == 0) {
			char *executablePath = buffer + sizeof(int);
			if (strcmp(executablePath, executablePathToKill) == 0) {
				if(softly)
				{
					kill(pid, SIGTERM);
				}
				else
				{
					kill(pid, SIGKILL);
				}
			}
		}
		free(buffer);
	}
	free(info);
}

int posix_spawn_hook(pid_t *restrict pid, const char *restrict path,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict])
{
	return spawn_hook_common(pid, path, file_actions, attrp, argv, envp, (void *)posix_spawn);
}

int posix_spawnp_hook(pid_t *restrict pid, const char *restrict file,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict])
{
	return resolvePath(file, NULL, ^int(char *path) {
		return spawn_hook_common(pid, path, file_actions, attrp, argv, envp, (void *)posix_spawn);
	});
}


int execve_hook(const char *path, char *const argv[], char *const envp[])
{
	posix_spawnattr_t attr = NULL;
	posix_spawnattr_init(&attr);
	posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETEXEC);
	int result = spawn_hook_common(NULL, path, NULL, &attr, argv, envp, (void *)posix_spawn);
	if (attr) {
		posix_spawnattr_destroy(&attr);
	}
	
	if(result != 0) { // posix_spawn will return errno and restore errno if it fails
		errno = result; // so we need to set errno by ourself
		return -1;
	}

	return result;
}

int execle_hook(const char *path, const char *arg0, ... /*, (char *)0, char *const envp[] */)
{
	va_list args;
	va_start(args, arg0);

	// Get argument count
	va_list args_copy;
	va_copy(args_copy, args);
	int arg_count = 1;
	for (char *arg = va_arg(args_copy, char *); arg != NULL; arg = va_arg(args_copy, char *)) {
		arg_count++;
	}
	va_end(args_copy);

	char *argv[arg_count+1];
	argv[0] = (char*)arg0;
	for (int i = 0; i < arg_count-1; i++) {
		char *arg = va_arg(args, char*);
		argv[i+1] = arg;
	}
	argv[arg_count] = NULL;

	__unused char* nullChar = va_arg(args, char*);
    
	char **envp = va_arg(args, char**);
	return execve_hook(path, argv, envp);
}

int execlp_hook(const char *file, const char *arg0, ... /*, (char *)0 */)
{
	va_list args;
	va_start(args, arg0);

	// Get argument count
	va_list args_copy;
	va_copy(args_copy, args);
	int arg_count = 1;
	for (char *arg = va_arg(args_copy, char*); arg != NULL; arg = va_arg(args_copy, char*)) {
		arg_count++;
	}
	va_end(args_copy);

	char **argv = malloc((arg_count+1) * sizeof(char *));
	argv[0] = (char*)arg0;
	for (int i = 0; i < arg_count-1; i++) {
		char *arg = va_arg(args, char*);
		argv[i+1] = arg;
	}
	argv[arg_count] = NULL;

	int r = resolvePath(file, NULL, ^int(char *path) {
		return execve_hook(path, argv, environ);
	});

	free(argv);

	return r;
}

int execl_hook(const char *path, const char *arg0, ... /*, (char *)0 */)
{
	va_list args;
	va_start(args, arg0);

	// Get argument count
	va_list args_copy;
	va_copy(args_copy, args);
	int arg_count = 1;
	for (char *arg = va_arg(args_copy, char*); arg != NULL; arg = va_arg(args_copy, char*)) {
		arg_count++;
	}
	va_end(args_copy);

	char *argv[arg_count+1];
	argv[0] = (char*)arg0;
	for (int i = 0; i < arg_count-1; i++) {
		char *arg = va_arg(args, char*);
		argv[i+1] = arg;
	}
	argv[arg_count] = NULL;

	return execve_hook(path, argv, environ);
}

int execv_hook(const char *path, char *const argv[])
{
	return execve_hook(path, argv, environ);
}

int execvP_hook(const char *file, const char *search_path, char *const argv[])
{
	__block bool execve_failed = false;
	int err = resolvePath(file, search_path, ^int(char *path) {
		(void)execve_hook(path, argv, environ);
		execve_failed = true;
		return 0;
	});
	if (!execve_failed) {
		errno = err;
	}
	return -1;
}

int execvp_hook(const char *name, char * const *argv)
{
	const char *path;
	/* Get the path we're searching. */
	if ((path = getenv("PATH")) == NULL)
		path = _PATH_DEFPATH;
	return execvP_hook(name, path, argv);
}

SHOOK_EXPORT const char * libroot_get_jbroot_prefix(void) {
    if ((pflags & palerain_option_rootful) == 0) return "/var/jb";
    else return "";
}
SHOOK_EXPORT const char * libroot_get_root_prefix(void) { return ""; }
SHOOK_EXPORT const char * libroot_get_boot_uuid(void) {
    static char uuid_string[37] = { '\0' };
    if (uuid_string[0] != '\0') return &uuid_string[0];
    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(xdict, "cmd", LAUNCHD_CMD_GET_BOOT_UUID);
    xpc_object_t xreply;
    int retval = jailbreak_send_launchd_message(xdict, &xreply);
    xpc_release(xdict);
    if (retval) {
        goto fail;
    }
    const uint8_t* uuid_buf = xpc_dictionary_get_uuid(xreply, "uuid");
    if (!uuid_buf) goto fail;
    uuid_unparse_upper(uuid_buf, uuid_string);
    xpc_release(xreply);
    return &uuid_string[0];
fail:
    return "00000000-0000-0000-0000-000000000000";
}


void* dlopen_from_hook(const char* path, int mode, void* addressInCaller)
{
    return dlopen_from(path, mode, addressInCaller);
}

void* dlopen_hook(const char* path, int mode)
{
	void* callerAddress = __builtin_return_address(0);
    return dlopen_from_hook(path, mode, callerAddress);
}

void* dlopen_audited_hook(const char* path, int mode)
{
	return dlopen_audited(path, mode);
}

bool dlopen_preflight_hook(const char* path)
{
	return dlopen_preflight(path);
}

int sandbox_init_hook(const char *profile, uint64_t flags, char **errorbuf)
{
	int retval = sandbox_init(profile, flags, errorbuf);
	if (retval == 0) {
		unsandbox();
	}
	return retval;
}

int sandbox_init_with_parameters_hook(const char *profile, uint64_t flags, const char *const parameters[], char **errorbuf)
{
	int retval = sandbox_init_with_parameters(profile, flags, parameters, errorbuf);
	if (retval == 0) {
		unsandbox();
	}
	return retval;
}

int sandbox_init_with_extensions_hook(const char *profile, uint64_t flags, const char *const extensions[], char **errorbuf)
{
	int retval = sandbox_init_with_extensions(profile, flags, extensions, errorbuf);
	if (retval == 0) {
		unsandbox();
	}
	return retval;
}

int ptrace_hook(int request, pid_t pid, caddr_t addr, int data)
{
	int retval = ptrace(request, pid, addr, data);
	return retval;
}

pid_t fork_hook(void)
{
	return fork();
}

pid_t vfork_hook(void)
{
	return vfork();
}

pid_t forkpty_hook(int *amaster, char *name, struct termios *termp, struct winsize *winp)
{
	return forkpty(amaster, name, termp, winp);
}

int daemon_hook(int __nochdir, int __noclose)
{
	return daemon(__nochdir, __noclose);
}

bool shouldEnableTweaks(void)
{
	if (pflags & (palerain_option_safemode | palerain_option_force_revert | palerain_option_setup_rootful)) {
		return false;
	}

	if (getpid() == 1)
		return false;

	if (access("/cores/.safe_mode", F_OK) == 0) {
		return false;
	}

	char *tweaksDisabledEnv = getenv("DISABLE_TWEAKS");
	if (tweaksDisabledEnv) {
		if (!strcmp(tweaksDisabledEnv, "1")) {
			return false;
		}
	}

	if (getenv("XPC_NULL_BOOTSTRAP") || getenv("XPC_SERVICES_UNAVAILABLE")) {
		return false;
	}

	const char *tweaksDisabledPathSuffixes[] = {
		// System binaries
		"/usr/libexec/xpcproxy",

		// Dopamine app itself (jailbreak detection bypass tweaks can break it)
		"Dopamine.app/Dopamine",
	};
	for (size_t i = 0; i < sizeof(tweaksDisabledPathSuffixes) / sizeof(const char*); i++)
	{
		if (stringEndsWith(gExecutablePath, tweaksDisabledPathSuffixes[i])) return false;
	}

	return true;
}

#ifdef HAVE_SYSTEMWIDE_IOSEXEC
bool has_libiosexec = false;
#endif
__attribute__((constructor)) static void initializer(void)
{
	JB_SandboxExtensions = strdup(getenv("JB_SANDBOX_EXTENSIONS"));
	unsetenv("JB_SANDBOX_EXTENSIONS");
	JB_RootPath = strdup(getenv("JB_ROOT_PATH"));
	JB_PinfoFlags = strdup(getenv("JB_PINFO_FLAGS"));
	JB_TweakLoaderPath = strdup(getenv("JB_TWEAKLOADER_PATH"));
	pflags = (uint64_t)strtoull(JB_PinfoFlags, NULL, 0);

    unsandbox();
    loadExecutablePath();

    char* xpc_service_name = getenv("XPC_SERVICE_NAME");
    if (xpc_service_name) in_jailbreakd = (strcmp(xpc_service_name, "in.palera.palera1nd") == 0);

    if (getenv("DYLD_INSERT_LIBRARIES") && !strcmp(getenv("DYLD_INSERT_LIBRARIES"), HOOK_DYLIB_PATH)) {
        // Unset DYLD_INSERT_LIBRARIES, but only if systemhook itself is the only thing contained in it
        unsetenv("DYLD_INSERT_LIBRARIES");
    }


    if (getenv("DYLD_LIBRARY_PATH") && !strcmp(getenv("DYLD_LIBRARY_PATH"), LIBROOT_DYLIB_DIRECTORY_PATH)) {
        // Unset DYLD_LIBRARY_PATH, but only if libroot.dylib directory path is the only thing contained in it
        unsetenv("DYLD_LIBRARY_PATH");
    }


#ifdef HAVE_SYSTEMWIDE_IOSEXEC
	for (int32_t i = 0; i < _dyld_image_count(); i++) {
		const char* name = _dyld_get_image_name(i);
		if (strstr(name, "/libiosexec.1.dylib")) has_libiosexec = true;
	}

	if (!has_libiosexec) get_libiosexec();
#endif

	struct stat sb;
	if(stat(gExecutablePath, &sb) == 0) {
		if (S_ISREG(sb.st_mode) && (sb.st_mode & (S_ISUID | S_ISGID))) {
			// jbdswFixSetuid();
		}
	}

	if (gExecutablePath) {
        static struct utsname name;
        static int release = 0;
        if (!release) {
            int ret = uname(&name);
            if (!ret) release = atoi(name.release);
        }
		
        if (release >= 20) {
            if (
                strcmp(gExecutablePath, "/usr/libexec/securityd") == 0 ||
                strcmp(gExecutablePath, "/usr/libexec/trustd") == 0 ||
                strcmp(gExecutablePath, "/usr/libexec/watchdogd") == 0 ||
                strcmp(gExecutablePath, "/usr/libexec/lsd") == 0 ||
                strcmp(gExecutablePath, "/System/Library/CoreServices/SpringBoard.app/SpringBoard") == 0 ||
                strcmp(gExecutablePath, "/usr/sbin/cfprefsd") == 0 ||
                strcmp(gExecutablePath, "/Applications/PineBoard.app/PineBoard") == 0) {
                dlopen_hook("/cores/binpack/usr/lib/universalhooks.dylib", RTLD_NOW);
            }
        }
	}

	//fprintf(stderr, "shouldEnableTweaks(): %d\n", shouldEnableTweaks());

	if (shouldEnableTweaks()) {
			const char *tweakLoaderPath;
			if (strcmp(JB_TweakLoaderPath, "@default") == 0) {
				if (pflags & palerain_option_rootful) {
					/* ellekit */
					tweakLoaderPath = "/usr/lib/TweakLoader.dylib";
			
					///* substitute */
					//if (access(tweakLoaderPath, F_OK) != 0)
					//	tweakLoaderPath = "/usr/lib/tweakloader.dylib";
			
					/* libhooker */
					if (access(tweakLoaderPath, F_OK) != 0)
						tweakLoaderPath = "/usr/lib/TweakInject.dylib";

				} else {
					/* ellekit */
						tweakLoaderPath = "/var/jb/usr/lib/TweakLoader.dylib";

					/* libhooker */
					if (access(tweakLoaderPath, F_OK) != 0)
						tweakLoaderPath = "/var/jb/usr/lib/TweakInject.dylib";
				}
			} else tweakLoaderPath = JB_TweakLoaderPath;
		if(access(tweakLoaderPath, F_OK) == 0)
		{
			void *tweakLoaderHandle = dlopen_hook(tweakLoaderPath, RTLD_NOW);
			if (tweakLoaderHandle != NULL) {
					dlclose(tweakLoaderHandle);
			}
		}
	}

	freeExecutablePath();
}

#define ITHINK_PURPOSE (0x0100000000000000llu)
#define RB2_USERREBOOT (0x2000000000000000llu)
#define RB2_OBLITERATE (0x4000000000000000llu)
#define RB2_FULLREBOOT (0x8000000000000000llu)
#define ITHINK_HALT    (0x8000000000000008llu)

int reboot3(uint64_t howto, ...);
int reboot3_hook(uint64_t howto, ...)
{
	if (in_jailbreakd || ((howto & RB2_USERREBOOT) == 0)) {
		if (howto & ITHINK_PURPOSE) {
			va_list va;
			va_start(va, howto);
			uint32_t purpose = va_arg(va, uint32_t);
			va_end(va);
			return reboot3(howto, purpose);
		} else return reboot3(howto);
	}

	xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_uint64(xdict, "howto", howto);
	if (howto & ITHINK_PURPOSE) {
		va_list va;
		va_start(va, howto);
		uint32_t purpose = va_arg(va, uint32_t);
		va_end(va);
		xpc_dictionary_set_uint64(xdict, "purpose", purpose);
	}
	xpc_dictionary_set_uint64(xdict, "cmd", JBD_CMD_PERFORM_REBOOT3);

	xpc_object_t xreply = jailbreak_send_jailbreakd_message_with_reply_sync(xdict);
	if (xpc_get_type(xreply) == XPC_TYPE_ERROR) {
		xpc_release(xreply);
        return EAGAIN;
    }
    xpc_release(xdict);
	int error;
	if ((error = xpc_dictionary_get_int64(xreply, "error"))) {
		xpc_release(xreply);
		return error;
	}
    xpc_release(xreply);
	return 0;
}

SHOOK_EXPORT int64_t jbdswInterceptUserspacePanic(const char *messageString) {
    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(xdict, "cmd", JBD_CMD_INTERCEPT_USERSPACE_PANIC);
    xpc_dictionary_set_string(xdict, "message", messageString);
    xpc_object_t xreply = jailbreak_send_jailbreakd_message_with_reply_sync(xdict);
    if (xpc_get_type(xreply) == XPC_TYPE_ERROR) {
        xpc_release(xreply);
        return EAGAIN;
    }
    xpc_release(xdict);
    int error;
    if ((error = xpc_dictionary_get_int64(xreply, "error"))) {
        xpc_release(xreply);
        return error;
    }
    xpc_release(xreply);
    return 0;
}

#define CONSTRUCT_V(major, minor, subminor) ((major & 0xffff) << 16) | ((minor & 0xff) << 8) | (subminor & 0xff)

uint32_t current_platform_min(void) {
    uint32_t current_plat = dyld_get_active_platform();
    switch (current_plat) {
        case PLATFORM_IOS:
        case PLATFORM_BRIDGEOS:
            return 2;
        case PLATFORM_TVOS:
            return 9;
        default:
            return 1;
    }
}

// this hook is used to make __builtin_available work normally in platform mismatched binaries
__API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), bridgeos(4.0))
__attribute__((visibility ("hidden")))
bool _availability_version_check_hook(uint32_t count, DyldBuildVersion versions[]) {
    uint32_t current_plat = dyld_get_active_platform();
    for (uint32_t i = 0; i < count; i++) {
        DyldBuildVersion* version = &versions[i];
        if (current_plat == version->platform) continue;
        uint32_t major = (version->version >> 16) & 0xffff;
        uint32_t minor = (version->version >> 8) & 0xff;
        uint32_t subminor = version->version & 0xff;
        switch (current_plat) {
            case PLATFORM_IOS:
            case PLATFORM_TVOS:
                switch (version->platform) {
                    case PLATFORM_MACOS:
                        if (major == 10) {
                            major = minor > (current_platform_min() + 2) ? minor - 2 : current_platform_min();
                            minor = subminor;
                            subminor = 0;
                        } else if (major > 10) major += 3;
                        else major = current_platform_min();
                        break;
                    case PLATFORM_BRIDGEOS:
                        major += 9;
                        break;
                    case PLATFORM_WATCHOS:
                    case PLATFORM_WATCHOSSIMULATOR:
                        major += 7;
                        break;
                    case PLATFORM_VISIONOS:
                    case PLATFORM_VISIONOSSIMULATOR:
                        major += 16;
                        break;
                    default:
                        break;
                }
                break;
            case PLATFORM_BRIDGEOS:
                switch (version->platform) {
                    case PLATFORM_MACOS:
                        if (major == 10) {
                            major = minor > (current_platform_min() + 11) ? minor - 11 : current_platform_min();
                            minor = subminor;
                            subminor = 0;
                        } else if (major > 10) {
                            major = major > (current_platform_min() + 6) ? major - 6 : current_platform_min();
                        }
                        else major = current_platform_min();
                        break;
                    case PLATFORM_IOS:
                    case PLATFORM_TVOS:
                    case PLATFORM_IOSSIMULATOR:
                    case PLATFORM_TVOSSIMULATOR:
                    case PLATFORM_MACCATALYST:
                        major = major > (current_platform_min() + 9) ? major - 9 : current_platform_min();
                        break;
                    case PLATFORM_WATCHOS:
                    case PLATFORM_WATCHOSSIMULATOR:
                        major = major > (current_platform_min() + 2) ? major - 2 : current_platform_min();
                        break;
                    case PLATFORM_VISIONOS:
                    case PLATFORM_VISIONOSSIMULATOR:
                        major += 7;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        version->version = CONSTRUCT_V(major, minor, subminor);
        version->platform = current_plat;
    }
    if (!_availability_version_check) {
        fprintf(stderr, "_availability_version_check unavailable\n");
        return false;
    }
    bool retval = _availability_version_check(count, versions);
    return retval;
}


DYLD_INTERPOSE(posix_spawn_hook, posix_spawn)
DYLD_INTERPOSE(posix_spawnp_hook, posix_spawnp)
DYLD_INTERPOSE(execve_hook, execve)
DYLD_INTERPOSE(execle_hook, execle)
DYLD_INTERPOSE(execlp_hook, execlp)
DYLD_INTERPOSE(execv_hook, execv)
DYLD_INTERPOSE(execl_hook, execl)
DYLD_INTERPOSE(execvp_hook, execvp)
DYLD_INTERPOSE(execvP_hook, execvP)
DYLD_INTERPOSE(dlopen_hook, dlopen)
DYLD_INTERPOSE(dlopen_from_hook, dlopen_from)
DYLD_INTERPOSE(dlopen_audited_hook, dlopen_audited)
DYLD_INTERPOSE(dlopen_preflight_hook, dlopen_preflight)
DYLD_INTERPOSE(sandbox_init_hook, sandbox_init)
DYLD_INTERPOSE(sandbox_init_with_parameters_hook, sandbox_init_with_parameters)
DYLD_INTERPOSE(sandbox_init_with_extensions_hook, sandbox_init_with_extensions)
DYLD_INTERPOSE(ptrace_hook, ptrace)
DYLD_INTERPOSE(fork_hook, fork)
DYLD_INTERPOSE(vfork_hook, vfork)
DYLD_INTERPOSE(forkpty_hook, forkpty)
DYLD_INTERPOSE(daemon_hook, daemon)
DYLD_INTERPOSE(reboot3_hook, reboot3)

__API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), bridgeos(4.0))
DYLD_INTERPOSE(_availability_version_check_hook, _availability_version_check)
