#include <CoreFoundation/CoreFoundation.h>
#include <spawn.h>
#include <spawn_private.h>
#include <sys/spawn_internal.h>
#include <paleinfo.h>

#define HOOK_DYLIB_PATH "/cores/binpack/usr/lib/systemhook.dylib"
#define JB_ENV_COUNT 4
extern char *JB_SandboxExtensions;
extern char *JB_RootPath;
extern char *JB_PinfoFlags;
extern bool swh_is_debugged;
extern uint64_t pflags;
extern bool has_libiosexec;

#define JB_ROOT_PATH(path) ({ \
	char *outPath = alloca(PATH_MAX); \
	strlcpy(outPath, JB_RootPath, PATH_MAX); \
	strlcat(outPath, path, PATH_MAX); \
	(outPath); \
})

bool stringStartsWith(const char *str, const char* prefix);
bool stringEndsWith(const char* str, const char* suffix);

#if 0
int64_t jbdswFixSetuid(void);
int64_t jbdswProcessBinary(const char *filePath);
int64_t jbdswProcessLibrary(const char *filePath);
int64_t jbdswDebugMe(void);
int64_t jbdswInterceptUserspacePanic(const char *messageString);
#endif

int resolvePath(const char *file, const char *searchPath, int (^attemptHandler)(char *path));
int spawn_hook_common(pid_t *restrict pid, const char *restrict path,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict],
					   void *pspawn_org);
