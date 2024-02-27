#include <CoreFoundation/CoreFoundation.h>
#include <spawn.h>
#include <spawn_private.h>
#include <sys/spawn_internal.h>
#include <paleinfo.h>

#define HOOK_DYLIB_PATH "/cores/binpack/usr/lib/systemhook.dylib"
#define LIBROOT_DYLIB_DIRECTORY_PATH "/cores/binpack/usr/lib/libroot"

#define JB_ENV_COUNT 7
extern char *JB_SandboxExtensions;
extern char *JB_RootPath;
extern char *JB_PinfoFlags;
extern char *JB_TweakLoaderPath;
extern bool swh_is_debugged;
extern uint64_t pflags;
extern bool has_libiosexec;

#define JB_ROOT_PATH(path) ({ \
	char *outPath = alloca(PATH_MAX); \
	strlcpy(outPath, JB_RootPath, PATH_MAX); \
	strlcat(outPath, path, PATH_MAX); \
	(outPath); \
})

#define SHOOK_EXPORT __attribute__((visibility ("default")))

bool stringStartsWith(const char *str, const char* prefix);
bool stringEndsWith(const char* str, const char* suffix);

SHOOK_EXPORT int64_t jbdswInterceptUserspacePanic(const char *messageString);


int resolvePath(const char *file, const char *searchPath, int (^attemptHandler)(char *path));
SHOOK_EXPORT int spawn_hook_common(pid_t *restrict pid, const char *restrict path,
					   const posix_spawn_file_actions_t *restrict file_actions,
					   const posix_spawnattr_t *restrict attrp,
					   char *const argv[restrict],
					   char *const envp[restrict],
					   void *pspawn_org);

typedef struct {
    uint32_t platform;
    uint32_t version;
} DyldBuildVersion;

__API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), bridgeos(3.0))
uint32_t dyld_get_active_platform(void);

__API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), bridgeos(4.0))
__attribute__((visibility ("hidden")))
bool _availability_version_check_hook(uint32_t count, DyldBuildVersion versions[]);

__API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), bridgeos(4.0))
bool _availability_version_check(uint32_t count, DyldBuildVersion versions[]);

#define CONSTRUCT_V(major, minor, subminor) ((major & 0xffff) << 16) | ((minor & 0xff) << 8) | (subminor & 0xff)
