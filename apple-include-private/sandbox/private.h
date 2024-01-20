#ifndef _SANDBOX_PRIVATE_H_
#define _SANDBOX_PRIVATE_H_

#include <stdint.h>
#include <sys/types.h>
#include <os/availability.h>
#include <bsm/libbsm.h>

__BEGIN_DECLS


/* The following flags are reserved for Mac OS X.  Developers should not
 * depend on their availability.
 */

/*
 * @define SANDBOX_NAMED_BUILTIN   The `profile' argument specifies the
 * name of a builtin profile that is statically compiled into the
 * system.
 */
#define SANDBOX_NAMED_BUILTIN	0x0002

/*
 * @define SANDBOX_NAMED_EXTERNAL   The `profile' argument specifies the
 * pathname of a Sandbox profile.  The pathname may be abbreviated: If
 * the name does not start with a `/' it is treated as relative to
 * /usr/share/sandbox and a `.sb' suffix is appended.
 */
#define SANDBOX_NAMED_EXTERNAL	0x0003

/*
 * @define SANDBOX_NAMED_MASK   Mask for name types: 4 bits, 15 possible
 * name types, 3 currently defined.
 */
#define SANDBOX_NAMED_MASK	0x000f

enum sandbox_filter_type {
	SANDBOX_FILTER_NONE,
	SANDBOX_FILTER_PATH,
	SANDBOX_FILTER_GLOBAL_NAME,
	SANDBOX_FILTER_LOCAL_NAME,
	SANDBOX_FILTER_APPLEEVENT_DESTINATION,
	SANDBOX_FILTER_RIGHT_NAME,
	SANDBOX_FILTER_PREFERENCE_DOMAIN,
	SANDBOX_FILTER_KEXT_BUNDLE_ID,
	SANDBOX_FILTER_INFO_TYPE,
	SANDBOX_FILTER_NOTIFICATION,
	// ?
	// ?
	SANDBOX_FILTER_XPC_SERVICE_NAME = 12,
	SANDBOX_FILTER_IOKIT_CONNECTION,
	// ?
	// ?
	// ?
	// ?
};

enum sandbox_extension_flags {
	FS_EXT_DEFAULTS =              0,
	FS_EXT_FOR_PATH =       (1 << 0),
	FS_EXT_FOR_FILE =       (1 << 1),
	FS_EXT_READ =           (1 << 2),
	FS_EXT_WRITE =          (1 << 3),
	FS_EXT_PREFER_FILEID =  (1 << 4),
};

typedef struct {
    char* builtin;
    unsigned char* data;
    size_t size;
} *sandbox_profile_t;

typedef struct {
    const char **params;
    size_t size;
    size_t available;
} *sandbox_params_t;

typedef struct {
    const char* bundle;
    const char* path;
} *sandbox_persona_t; // Not seeing this API public used, guessed struct name

extern const char * APP_SANDBOX_IOKIT_CLIENT;
extern const char * APP_SANDBOX_MACH;
extern const char * APP_SANDBOX_READ;
extern const char * APP_SANDBOX_READ_WRITE;

extern const char * IOS_SANDBOX_APPLICATION_GROUP;
extern const char * IOS_SANDBOX_CONTAINER;

extern const enum sandbox_filter_type SANDBOX_CHECK_ALLOW_APPROVAL;
extern const enum sandbox_filter_type SANDBOX_CHECK_CANONICAL;
extern const enum sandbox_filter_type SANDBOX_CHECK_NOFOLLOW;
extern const enum sandbox_filter_type SANDBOX_CHECK_NO_APPROVAL;
extern const enum sandbox_filter_type SANDBOX_CHECK_NO_REPORT;

extern const uint32_t SANDBOX_EXTENSION_CANONICAL;
extern const uint32_t SANDBOX_EXTENSION_DEFAULT;
extern const uint32_t SANDBOX_EXTENSION_MAGIC;
extern const uint32_t SANDBOX_EXTENSION_NOFOLLOW;
extern const uint32_t SANDBOX_EXTENSION_NO_REPORT;
extern const uint32_t SANDBOX_EXTENSION_NO_STORAGE_CLASS;
extern const uint32_t SANDBOX_EXTENSION_PREFIXMATCH;
extern const uint32_t SANDBOX_EXTENSION_UNRESOLVED;

extern const char* const SANDBOX_BUILD_ID;

int sandbox_apply(sandbox_profile_t);
int sandbox_apply_container(sandbox_profile_t, uint32_t);

int sandbox_check(pid_t, const char *operation, enum sandbox_filter_type, ...);
//int sandbox_check_bulk
int sandbox_check_by_audit_token(audit_token_t, const char *operation, enum sandbox_filter_type, ...);
//int sandbox_check_by_reference
int sandbox_check_by_uniqueid(uid_t, pid_t, const char *operation, enum sandbox_filter_type, ...);
//int sandbox_check_common

sandbox_profile_t *sandbox_compile_entitlements(char *ents, sandbox_params_t *params, char **err);
sandbox_profile_t *sandbox_compile_file(char *profile_file, sandbox_params_t *params, char **err);
sandbox_profile_t *sandbox_compile_named(char *profile_name, sandbox_params_t *params, char **err);
sandbox_profile_t *sandbox_compile_string(char *profile_string, sandbox_params_t *params, char **err);

int sandbox_consume_extension(const char *path, const char *ext_token);
int sandbox_consume_fs_extension(const char *ext_token, char **path);
int sandbox_consume_mach_extension(const char *ext_token, char **name);

int sandbox_container_paths_iterate_items(int, sandbox_profile_t);
int sandbox_container_path_for_audit_token(char *buffer, size_t bufsize);
int sandbox_container_path_for_pid(pid_t, char *buffer, size_t bufsize);

sandbox_params_t sandbox_create_params(void);

void sandbox_free_params(sandbox_params_t);
void sandbox_free_profile(sandbox_profile_t);

int64_t sandbox_extension_consume(const char *extension_token);
char *sandbox_extension_issue_file(const char *extension_class, const char *path, uint32_t flags);
char *sandbox_extension_issue_file_to_process(const char *extension_class, const char *path, uint32_t flags, audit_token_t);
char *sandbox_extension_issue_file_to_process_by_pid(const char *extension_class, const char *path, uint32_t flags, pid_t);
char *sandbox_extension_issue_file_to_self(const char *extension_class, const char *path, uint32_t flags);
char *sandbox_extension_issue_generic(const char *extension_class, uint32_t flags);
char *sandbox_extension_issue_generic_to_process(const char *extension_class, uint32_t flags, audit_token_t);
char *sandbox_extension_issue_generic_to_process_by_pid(const char *extension_class, uint32_t flags, pid_t);
char *sandbox_extension_issue_iokit_registry_entry_class(const char *extension_class, const char *registry_entry_class, uint32_t flags);
char *sandbox_extension_issue_iokit_registry_entry_class_to_process(const char *extension_class, const char *registry_entry_class, uint32_t flags, audit_token_t);
char *sandbox_extension_issue_iokit_registry_entry_class_to_process_by_pid(const char *extension_class, const char *registry_entry_class, uint32_t flags, pid_t);
char *sandbox_extension_issue_iokit_user_client_class(const char *extension_class, const char *registry_entry_class, uint32_t flags);
char *sandbox_extension_issue_mach(const char *extension_class, const char *name, uint32_t flags);
char *sandbox_extension_issue_mach_to_process(const char *extension_class, const char *name, uint32_t flags, audit_token_t);
char *sandbox_extension_issue_mach_to_process_by_pid(const char *extension_class, const char *name, uint32_t flags, pid_t);
char *sandbox_extension_issue_posix_ipc(const char *extension_class, const char *name, uint32_t flags);
void sandbox_extension_reap(void);
int sandbox_extension_release(int64_t extension_handle);
int sandbox_extension_release_file(int64_t extension_handle, const char *path);
int sandbox_extension_update_file(int64_t extension_handle, const char *path);
int sandbox_init(const char *profile, uint64_t flags, char **errorbuf);

API_DEPRECATED("No longer supported", macos(10.5, 10.8), ios(2.0, 6.0), tvos(4.0, 4.0), watchos(1.0, 1.0))
API_UNAVAILABLE(macCatalyst)
int sandbox_init_from_pid(const char *profile, uint64_t flags, pid_t pid, char **errorbuf);

API_DEPRECATED("No longer supported", macos(10.5, 10.8), ios(2.0, 6.0), tvos(4.0, 4.0), watchos(1.0, 1.0))
API_UNAVAILABLE(macCatalyst)
int sandbox_init_with_extensions(const char *profile, uint64_t flags, const char *const extensions[], char **errorbuf);

API_DEPRECATED("No longer supported", macos(10.5, 10.8), ios(2.0, 6.0), tvos(4.0, 4.0), watchos(1.0, 1.0))
API_UNAVAILABLE(macCatalyst)
int sandbox_init_with_parameters(const char *profile, uint64_t flags, const char *const parameters[], char **errorbuf);

int sandbox_inspect_pid(pid_t, char **buffer, size_t *bufsize);
int sandbox_inspect_smemory(void); // ?

int sandbox_issue_extension(const char *path, char **ext_token);
int sandbox_issue_fs_extension(const char *path, uint64_t flags, char **ext_token);
int sandbox_issue_fs_rw_extension(const char *path, char **ext_token);
int sandbox_issue_mach_extension(const char *name, char **ext_token);

int sandbox_note(const char *note);

int sandbox_query_approval_policy_for_path(const char *policy, const char *path, char **approval);

int sandbox_release_fs_extension(const char *ext_token);

int sandbox_requests_integrity_protection_for_preference_domain(const char *bundle);

int sandbox_set_param(sandbox_params_t, const char *key, const char *value);
int sandbox_set_user_state_item(uid_t uid, int type, sandbox_persona_t item, int *buf);
/***
int sandbox_set_user_state_item(uid_t uid, int type, sandbox_persona_t item, int *buf)
{
	return sandbox_set_user_state_item_with_persona(uid, 0xFFFFFFFF, type, item, buf);
}
***/
int sandbox_set_user_state_item_with_persona(uid_t uid, uint64_t offset, sandbox_persona_t item, int *buf);

/*
// These functions are returning a struct with all members int, which was not predefined by any public sources
// Hope someone can figure this out
sandbox_pack_t sandbox_user_state_item_buffer_create(void);
int sandbox_user_state_item_buffer_destroy(sandbox_pack_t);
int sandbox_user_state_item_buffer_send(sandbox_pack_t);
int sandbox_user_state_iterate_items(int, sandbox_pack_t);
*/

int sandbox_suspend(pid_t pid);

int sandbox_unsuspend(void);

const char *_amkrtemp(const char *);

int _sandbox_in_a_container(void);

__END_DECLS

#endif /* _SANDBOX_PRIVATE_H_ */
